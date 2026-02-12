#!/bin/bash

# ═══════════════════════════════════════════════════════════════
# ESP32 Weather Station API Tests
# Тестирование REST API endpoints с помощью curl
# ═══════════════════════════════════════════════════════════════

set -e

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Конфигурация
ESP32_IP="${ESP32_IP:-192.168.1.100}"
BASE_URL="http://${ESP32_IP}"
TIMEOUT=5
VERBOSE=false

# Счетчики тестов
TESTS_PASSED=0
TESTS_FAILED=0
TESTS_TOTAL=0

# ═══════════════════════════════════════════════════════════════
# Вспомогательные функции
# ═══════════════════════════════════════════════════════════════

log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[✓]${NC} $1"
    ((TESTS_PASSED++))
}

log_error() {
    echo -e "${RED}[✗]${NC} $1"
    ((TESTS_FAILED++))
}

log_warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

run_test() {
    local test_name=$1
    ((TESTS_TOTAL++))
    log_info "Test #${TESTS_TOTAL}: ${test_name}"
}

# HTTP запрос с проверкой
http_request() {
    local url=$1
    local expected_code=${2:-200}
    local method=${3:-GET}
    
    local response=$(curl -s -w "\n%{http_code}" -X ${method} "${url}" --max-time ${TIMEOUT} 2>&1)
    local http_code=$(echo "$response" | tail -n 1)
    local body=$(echo "$response" | sed '$d')
    
    if [ "$http_code" != "$expected_code" ]; then
        log_error "Expected HTTP $expected_code, got $http_code"
        [ "$VERBOSE" = true ] && echo "Response: $body"
        return 1
    fi
    
    echo "$body"
    return 0
}

# Проверка JSON поля
check_json_field() {
    local json=$1
    local field=$2
    local expected_type=${3:-any}
    
    # Используем Python для парсинга JSON (если доступен)
    if command -v python3 &> /dev/null; then
        local value=$(echo "$json" | python3 -c "import json,sys; data=json.load(sys.stdin); print(data.get('$field', 'MISSING'))")
        
        if [ "$value" = "MISSING" ]; then
            log_error "Field '$field' not found in JSON"
            return 1
        fi
        
        log_success "Field '$field' exists (value: $value)"
        return 0
    else
        # Простая проверка без парсера
        if echo "$json" | grep -q "\"$field\""; then
            log_success "Field '$field' found in response"
            return 0
        else
            log_error "Field '$field' not found in JSON"
            return 1
        fi
    fi
}

# ═══════════════════════════════════════════════════════════════
# Тесты
# ═══════════════════════════════════════════════════════════════

test_connectivity() {
    run_test "ESP32 connectivity check"
    
    if ping -c 1 -W 2 ${ESP32_IP} &> /dev/null; then
        log_success "ESP32 is reachable at ${ESP32_IP}"
        return 0
    else
        log_error "ESP32 is not reachable at ${ESP32_IP}"
        return 1
    fi
}

test_root_endpoint() {
    run_test "GET / - Root endpoint (HTML page)"
    
    local response=$(http_request "${BASE_URL}/")
    
    if [ $? -eq 0 ]; then
        # Проверяем что это HTML
        if echo "$response" | grep -q "<!DOCTYPE html>"; then
            log_success "Root endpoint returns HTML page"
            
            # Проверяем наличие ключевых элементов
            echo "$response" | grep -q "Environmental Statistics" && log_success "HTML contains title"
            echo "$response" | grep -q "temperature" && log_success "HTML contains temperature element"
            echo "$response" | grep -q "humidity" && log_success "HTML contains humidity element"
            
            return 0
        else
            log_error "Root endpoint doesn't return valid HTML"
            return 1
        fi
    fi
    
    return 1
}

test_data_endpoint() {
    run_test "GET /data - Sensor data endpoint"
    
    local response=$(http_request "${BASE_URL}/data")
    
    if [ $? -eq 0 ]; then
        log_success "Data endpoint is accessible"
        
        # Проверка обязательных полей
        check_json_field "$response" "temperature"
        check_json_field "$response" "humidity"
        check_json_field "$response" "minTemp"
        check_json_field "$response" "maxTemp"
        check_json_field "$response" "minHumid"
        check_json_field "$response" "maxHumid"
        check_json_field "$response" "avgTemp"
        check_json_field "$response" "avgHumid"
        check_json_field "$response" "dewPoint"
        check_json_field "$response" "heatIndex"
        check_json_field "$response" "timestamp"
        
        # Проверка диапазонов значений
        if command -v python3 &> /dev/null; then
            local temp=$(echo "$response" | python3 -c "import json,sys; print(json.load(sys.stdin)['temperature'])")
            
            if (( $(echo "$temp > -40 && $temp < 85" | bc -l) )); then
                log_success "Temperature is in valid range: ${temp}°C"
            else
                log_warning "Temperature out of range: ${temp}°C"
            fi
        fi
        
        return 0
    fi
    
    return 1
}

test_stats_endpoint() {
    run_test "GET /stats - System statistics endpoint"
    
    local response=$(http_request "${BASE_URL}/stats")
    
    if [ $? -eq 0 ]; then
        log_success "Stats endpoint is accessible"
        
        # Проверка полей
        check_json_field "$response" "uptime"
        check_json_field "$response" "freeHeap"
        check_json_field "$response" "heapUsage"
        check_json_field "$response" "cpuUsage"
        check_json_field "$response" "ssid"
        check_json_field "$response" "rssi"
        check_json_field "$response" "ip"
        check_json_field "$response" "requests"
        check_json_field "$response" "errors"
        check_json_field "$response" "battery"
        
        # Проверка вложенных полей батареи
        if command -v python3 &> /dev/null; then
            local battery=$(echo "$response" | python3 -c "import json,sys; print(json.dumps(json.load(sys.stdin).get('battery', {})))")
            
            echo "$battery" | python3 -c "import json,sys; d=json.load(sys.stdin); print(d.get('voltage', 'N/A'))" | grep -q "N/A" && log_error "Battery voltage missing" || log_success "Battery voltage present"
            echo "$battery" | python3 -c "import json,sys; d=json.load(sys.stdin); print(d.get('percent', 'N/A'))" | grep -q "N/A" && log_error "Battery percent missing" || log_success "Battery percent present"
        fi
        
        return 0
    fi
    
    return 1
}

test_history_endpoint() {
    run_test "GET /history - Historical data endpoint"
    
    local response=$(http_request "${BASE_URL}/history")
    
    if [ $? -eq 0 ]; then
        log_success "History endpoint is accessible"
        
        # Проверка структуры
        check_json_field "$response" "labels"
        check_json_field "$response" "temp"
        check_json_field "$response" "humid"
        
        # Проверка что массивы не пустые
        if command -v python3 &> /dev/null; then
            local label_count=$(echo "$response" | python3 -c "import json,sys; print(len(json.load(sys.stdin)['labels']))")
            
            if [ "$label_count" -gt 0 ]; then
                log_success "History contains $label_count data points"
            else
                log_warning "History is empty"
            fi
        fi
        
        return 0
    fi
    
    return 1
}

test_reset_endpoint() {
    run_test "GET /reset - Reset min/max values"
    
    local response=$(http_request "${BASE_URL}/reset")
    
    if [ $? -eq 0 ]; then
        # Проверяем что вернулся успешный ответ
        if echo "$response" | grep -q "success.*true"; then
            log_success "Reset endpoint executed successfully"
            return 0
        else
            log_error "Reset endpoint returned unexpected response"
            return 1
        fi
    fi
    
    return 1
}

test_404_handling() {
    run_test "GET /nonexistent - 404 handling"
    
    local response=$(curl -s -w "\n%{http_code}" "${BASE_URL}/nonexistent" --max-time ${TIMEOUT} 2>&1)
    local http_code=$(echo "$response" | tail -n 1)
    
    if [ "$http_code" = "404" ]; then
        log_success "404 error handled correctly"
        return 0
    else
        log_error "Expected 404, got $http_code"
        return 1
    fi
}

test_cors_headers() {
    run_test "CORS headers check"
    
    local headers=$(curl -s -I "${BASE_URL}/data" --max-time ${TIMEOUT})
    
    if echo "$headers" | grep -qi "Access-Control-Allow-Origin"; then
        log_success "CORS headers present"
        return 0
    else
        log_warning "CORS headers not found"
        return 0  # Не критично
    fi
}

test_response_time() {
    run_test "Response time check"
    
    local start_time=$(date +%s%N)
    curl -s "${BASE_URL}/data" --max-time ${TIMEOUT} &> /dev/null
    local end_time=$(date +%s%N)
    
    local elapsed_ms=$(( (end_time - start_time) / 1000000 ))
    
    if [ $elapsed_ms -lt 1000 ]; then
        log_success "Response time: ${elapsed_ms}ms (good)"
    elif [ $elapsed_ms -lt 3000 ]; then
        log_warning "Response time: ${elapsed_ms}ms (acceptable)"
    else
        log_error "Response time: ${elapsed_ms}ms (slow)"
    fi
}

test_concurrent_requests() {
    run_test "Concurrent requests handling"
    
    log_info "Sending 5 concurrent requests..."
    
    for i in {1..5}; do
        curl -s "${BASE_URL}/data" --max-time ${TIMEOUT} &> /dev/null &
    done
    
    wait
    
    # Проверяем что сервер все еще отвечает
    if http_request "${BASE_URL}/data" &> /dev/null; then
        log_success "Server handles concurrent requests"
        return 0
    else
        log_error "Server failed after concurrent requests"
        return 1
    fi
}

test_websocket_availability() {
    run_test "WebSocket port availability"
    
    if command -v nc &> /dev/null; then
        if nc -z -w2 ${ESP32_IP} 81; then
            log_success "WebSocket port 81 is open"
            return 0
        else
            log_warning "WebSocket port 81 is not accessible"
            return 0  # Не критично для HTTP API
        fi
    else
        log_info "netcat not available, skipping WebSocket test"
    fi
}

# ═══════════════════════════════════════════════════════════════
# Основной запуск тестов
# ═══════════════════════════════════════════════════════════════

main() {
    echo ""
    echo "═══════════════════════════════════════════════════════════"
    echo "  ESP32 Weather Station API Tests"
    echo "═══════════════════════════════════════════════════════════"
    echo ""
    echo "Target: ${BASE_URL}"
    echo "Timeout: ${TIMEOUT}s"
    echo ""
    
    # Проверка зависимостей
    if ! command -v curl &> /dev/null; then
        log_error "curl is not installed"
        exit 1
    fi
    
    if ! command -v python3 &> /dev/null; then
        log_warning "python3 not found - JSON validation will be limited"
    fi
    
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
    echo -e "${GREEN}Passed:${NC}       ${TESTS_PASSED}"
    echo -e "${RED}Failed:${NC}       ${TESTS_FAILED}"
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
            BASE_URL="http://${ESP32_IP}"
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
            echo "  -h, --host IP       ESP32 IP address (default: 192.168.1.100)"
            echo "  -v, --verbose       Verbose output"
            echo "  -t, --timeout SEC   Request timeout in seconds (default: 5)"
            echo "  --help              Show this help"
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
