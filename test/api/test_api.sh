#!/bin/bash

# ═══════════════════════════════════════════════════════════════
# ESP32 Weather Station API Tests
# Быстрые smoke-тесты REST API с помощью curl
#
#   ./test_api.sh --host 192.168.1.65        # плата
#   ./test_api.sh --host 127.0.0.1:8080      # test/mock_server.py
#
# ⚠️ Вызывает /reset — min/max на плате будут сброшены.
# ═══════════════════════════════════════════════════════════════

# Без set -e: отдельный упавший тест не должен обрывать весь прогон
set -uo pipefail

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Конфигурация
ESP32_IP="${ESP32_IP:-192.168.1.100}"   # можно с портом: 127.0.0.1:8080
TIMEOUT=5
VERBOSE=false

# Счетчики тестов
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_TOTAL=0

# Результат последнего http_get (глобальные — без subshell, чтобы
# счётчики и сообщения не терялись внутри $(...))
HTTP_CODE=""
HTTP_BODY=""
HTTP_HEADERS=""

# ═══════════════════════════════════════════════════════════════
# Вспомогательные функции
# ═══════════════════════════════════════════════════════════════

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[✓]${NC} $1"
    TESTS_PASSED=$((TESTS_PASSED + 1))
}

log_error() {
    echo -e "${RED}[✗]${NC} $1"
    TESTS_FAILED=$((TESTS_FAILED + 1))
}

log_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

run_test() {
    TESTS_TOTAL=$((TESTS_TOTAL + 1))
    log_info "Test #${TESTS_TOTAL}: $1"
}

# GET/POST запрос: заполняет HTTP_CODE, HTTP_BODY, HTTP_HEADERS
http_get() {
    local url=$1
    local method=${2:-GET}
    local headers_file
    headers_file=$(mktemp)

    HTTP_BODY=$(curl -s -X "$method" -D "$headers_file" -w "\n%{http_code}" \
                     --max-time "$TIMEOUT" "$url" 2>/dev/null)
    HTTP_CODE=$(tail -n 1 <<< "$HTTP_BODY")
    HTTP_BODY=$(sed '$d' <<< "$HTTP_BODY")
    HTTP_HEADERS=$(cat "$headers_file")
    rm -f "$headers_file"

    [ "$VERBOSE" = true ] && echo "  $method $url → $HTTP_CODE"
    return 0
}

# Проверка кода ответа последнего запроса
expect_code() {
    local expected=$1
    local what=$2
    if [ "$HTTP_CODE" = "$expected" ]; then
        log_success "$what → HTTP $HTTP_CODE"
        return 0
    fi
    log_error "$what: expected HTTP $expected, got ${HTTP_CODE:-no response}"
    [ "$VERBOSE" = true ] && echo "Response: $HTTP_BODY"
    return 1
}

# Значение из JSON последнего ответа по пути вида "battery.voltage"
json_get() {
    python3 -c '
import json, sys
try:
    value = json.loads(sys.argv[1])
    for key in sys.argv[2].split("."):
        value = value[key]
except (KeyError, TypeError, ValueError):
    print("__MISSING__")
else:
    print(json.dumps(value) if isinstance(value, (list, dict)) else value)
' "$HTTP_BODY" "$1"
}

# Проверка наличия полей в JSON последнего ответа
check_json_fields() {
    local missing=()
    local field
    for field in "$@"; do
        [ "$(json_get "$field")" = "__MISSING__" ] && missing+=("$field")
    done
    if [ ${#missing[@]} -eq 0 ]; then
        log_success "All fields present: $*"
    else
        log_error "Missing fields: ${missing[*]}"
    fi
}

# Проверка значения регулярным выражением
check_json_match() {
    local field=$1
    local pattern=$2
    local value
    value=$(json_get "$field")
    if [[ "$value" =~ $pattern ]]; then
        log_success "$field = $value"
    else
        log_error "$field has unexpected value: '$value' (expected /$pattern/)"
    fi
}

# Проверка числового диапазона
check_json_range() {
    local field=$1 min=$2 max=$3
    local value
    value=$(json_get "$field")
    if python3 -c 'import sys; v, lo, hi = map(float, sys.argv[1:]); sys.exit(0 if lo <= v <= hi else 1)' \
            "$value" "$min" "$max" 2>/dev/null; then
        log_success "$field = $value (in $min..$max)"
    else
        log_error "$field out of range: '$value' (expected $min..$max)"
    fi
}

# ═══════════════════════════════════════════════════════════════
# Тесты
# ═══════════════════════════════════════════════════════════════

test_connectivity() {
    run_test "ESP32 connectivity check"
    http_get "${BASE_URL}/"
    if [ "$HTTP_CODE" = "200" ]; then
        log_success "ESP32 is reachable at ${BASE_URL}"
        return 0
    fi
    log_error "ESP32 is not reachable at ${BASE_URL}"
    return 1
}

test_root_endpoint() {
    run_test "GET / - Root endpoint (HTML page)"
    http_get "${BASE_URL}/"
    expect_code 200 "GET /" || return

    grep -q "<!DOCTYPE html>" <<< "$HTTP_BODY" \
        && log_success "Returns HTML page" || log_error "Response is not an HTML page"
    grep -q "<title>ENV Station</title>" <<< "$HTTP_BODY" \
        && log_success "HTML contains title" || log_error "Title 'ENV Station' not found"
    grep -q 'id="temperature"' <<< "$HTTP_BODY" \
        && log_success "HTML contains temperature element" || log_error "No #temperature element"
    grep -q 'id="humidity"' <<< "$HTTP_BODY" \
        && log_success "HTML contains humidity element" || log_error "No #humidity element"
}

test_data_endpoint() {
    run_test "GET /data - Sensor data endpoint"
    http_get "${BASE_URL}/data"
    expect_code 200 "GET /data" || return

    check_json_fields temperature humidity minTemp maxTemp minHumid maxHumid \
                      avgTemp avgHumid dewPoint heatIndex timestamp
    check_json_range temperature -40 85
    check_json_range humidity 0 100
}

test_stats_endpoint() {
    run_test "GET /stats - System statistics endpoint"
    http_get "${BASE_URL}/stats"
    expect_code 200 "GET /stats" || return

    check_json_fields uptime firmware buildTime millisOverflows freeHeap heapUsage \
                      cpuUsage ssid rssi ip requests errors \
                      battery.voltage battery.percent battery.status battery.source
    check_json_match firmware '^[0-9]+\.[0-9]+(\.[0-9]+)?$'
    check_json_match buildTime '^[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}$'
    check_json_match millisOverflows '^[0-9]+$'
    check_json_range battery.percent 0 100
    check_json_range rssi -100 0
}

test_history_endpoint() {
    run_test "GET /history - Historical data endpoint"
    http_get "${BASE_URL}/history"
    expect_code 200 "GET /history" || return

    check_json_fields labels temp humid dew heat

    local lengths
    lengths=$(python3 -c '
import json, sys
d = json.loads(sys.argv[1])
print(" ".join(str(len(d[k])) for k in ("labels", "temp", "humid", "dew", "heat")))
' "$HTTP_BODY")
    local first=${lengths%% *}
    if [ "$(tr ' ' '\n' <<< "$lengths" | sort -u | wc -l)" = "1" ]; then
        log_success "All history series have $first points"
    else
        log_error "History series have different lengths: $lengths"
    fi
    [ "$first" -eq 0 ] && log_warning "History is empty (device just booted?)"
}

test_reset_endpoint() {
    run_test "GET /reset - Reset min/max values"
    http_get "${BASE_URL}/reset"
    expect_code 200 "GET /reset" || return

    if [ "$(json_get success)" = "True" ]; then
        log_success "Reset returned success"
    else
        log_error "Reset returned unexpected response: $HTTP_BODY"
    fi
}

test_404_handling() {
    run_test "GET /nonexistent - 404 handling"
    http_get "${BASE_URL}/nonexistent"
    expect_code 404 "GET /nonexistent"

    run_test "POST /data - GET-only route"
    http_get "${BASE_URL}/data" POST
    expect_code 404 "POST /data"
}

test_cors_headers() {
    run_test "CORS headers check"
    http_get "${BASE_URL}/data"
    if grep -qi "^Access-Control-Allow-Origin: \*" <<< "$HTTP_HEADERS"; then
        log_success "CORS headers present"
    else
        log_error "Access-Control-Allow-Origin: * not found"
    fi
}

test_response_time() {
    run_test "Response time check"

    local seconds elapsed_ms
    seconds=$(curl -s -o /dev/null -w "%{time_total}" --max-time "$TIMEOUT" "${BASE_URL}/data")
    elapsed_ms=$(python3 -c 'import sys; print(int(float(sys.argv[1]) * 1000))' "$seconds")

    if [ "$elapsed_ms" -lt 1000 ]; then
        log_success "Response time: ${elapsed_ms}ms (good)"
    elif [ "$elapsed_ms" -lt 3000 ]; then
        log_warning "Response time: ${elapsed_ms}ms (acceptable)"
    else
        log_error "Response time: ${elapsed_ms}ms (slow)"
    fi
}

test_concurrent_requests() {
    run_test "Concurrent requests handling"
    log_info "Sending 5 concurrent requests..."

    local pids=() failed=0 pid
    for _ in {1..5}; do
        curl -sf -o /dev/null --max-time "$TIMEOUT" "${BASE_URL}/data" &
        pids+=($!)
    done
    for pid in "${pids[@]}"; do
        wait "$pid" || failed=$((failed + 1))
    done

    if [ "$failed" -eq 0 ]; then
        log_success "All 5 concurrent requests succeeded"
    else
        log_error "$failed of 5 concurrent requests failed"
    fi
}

test_websocket_availability() {
    run_test "WebSocket port availability"

    local host=${ESP32_IP%%:*}
    if ! command -v nc &> /dev/null; then
        log_warning "netcat not available, skipping WebSocket test"
    elif nc -z -w2 "$host" 81; then
        log_success "WebSocket port 81 is open"
    else
        log_warning "WebSocket port 81 is not accessible (mock server has none)"
    fi
}

# ═══════════════════════════════════════════════════════════════
# Основной запуск тестов
# ═══════════════════════════════════════════════════════════════

main() {
    BASE_URL="http://${ESP32_IP}"

    echo ""
    echo "═══════════════════════════════════════════════════════════"
    echo "  ESP32 Weather Station API Tests"
    echo "═══════════════════════════════════════════════════════════"
    echo ""
    echo "Target: ${BASE_URL}"
    echo "Timeout: ${TIMEOUT}s"
    echo ""

    # Проверка зависимостей
    local tool
    for tool in curl python3; do
        if ! command -v "$tool" &> /dev/null; then
            echo -e "${RED}[✗]${NC} $tool is not installed"
            exit 1
        fi
    done

    # Запуск тестов
    test_connectivity || exit 1

    echo ""
    echo "─────────────────────────────────────────────────────────"
    echo " HTTP Endpoints"
    echo "─────────────────────────────────────────────────────────"
    echo ""

    test_root_endpoint
    test_data_endpoint
    test_stats_endpoint
    test_history_endpoint
    test_reset_endpoint
    test_404_handling

    echo ""
    echo "─────────────────────────────────────────────────────────"
    echo " Performance & Reliability"
    echo "─────────────────────────────────────────────────────────"
    echo ""

    test_cors_headers
    test_response_time
    test_concurrent_requests
    test_websocket_availability

    # Итоговая статистика
    echo ""
    echo "═══════════════════════════════════════════════════════════"
    echo "  Test Results"
    echo "═══════════════════════════════════════════════════════════"
    echo ""
    echo "Total tests:  ${TESTS_TOTAL}"
    echo -e "${GREEN}Passed checks:${NC} ${TESTS_PASSED}"
    echo -e "${RED}Failed checks:${NC} ${TESTS_FAILED}"
    echo ""

    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}✓ All tests passed!${NC}"
        exit 0
    else
        echo -e "${RED}✗ Some tests failed${NC}"
        exit 1
    fi
}

# Обработка аргументов
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--host)
            ESP32_IP="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -t|--timeout)
            TIMEOUT="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  -h, --host HOST[:PORT]  ESP32 address (default: \$ESP32_IP or 192.168.1.100)"
            echo "  -v, --verbose           Verbose output"
            echo "  -t, --timeout SEC       Request timeout in seconds (default: 5)"
            echo "  --help                  Show this help"
            echo ""
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

main
