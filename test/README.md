# ESP32 Weather Station - Testing Documentation

Полное руководство по тестированию проекта ESP32 Weather Station.

## 📋 Содержание

- [Типы тестов](#типы-тестов)
- [Установка](#установка)
- [Быстрый старт](#быстрый-старт)
- [API тесты](#api-тесты)
- [Веб-тесты](#веб-тесты)
- [GitHub Actions CI/CD](#github-actions-cicd)
- [Расширенное использование](#расширенное-использование)

---

## Типы тестов

### 1. 🔨 Build Tests (GitHub Actions)
- Компиляция прошивки для ESP32-C3
- Проверка размера прошивки
- Статический анализ кода

### 2. 🌐 API Tests
- **Bash/curl** - быстрые smoke тесты
- **Python/pytest** - детальные функциональные тесты

### 3. 💻 Web UI Tests
- **Playwright** - end-to-end тестирование веб-интерфейса
- Проверка отображения данных
- Тестирование интерактивности

### 4. 🔗 Integration Tests
- Полный цикл работы системы
- Согласованность данных

---

## Установка

### Требования

- Python 3.10+
- curl (для bash тестов)
- ESP32 устройство в локальной сети — или `--mock` (см. ниже), тогда плата не нужна

### Шаг 1: Установка Python зависимостей

```bash
cd test
pip install -r requirements.txt
```

### Шаг 2: Установка Playwright browsers

```bash
playwright install
```

### Шаг 3: Настройка окружения

```bash
# Скопируйте пример конфигурации
cp .env.example .env

# Отредактируйте .env
# Установите IP вашего ESP32
nano .env
```

Пример `.env`:
```bash
ESP32_IP=192.168.1.100
TEST_TIMEOUT=5
HEADLESS=true
```

---

## Быстрый старт

### Все тесты сразу

```bash
# Установите IP вашего ESP32
export ESP32_IP=192.168.1.100

# Запустите все тесты (ui/, api/, web/)
cd test
pytest -v
```

> ⚠️ `TestResetEndpoint` и `test_api.sh` вызывают `/reset` — min/max на плате
> сбрасываются. Чтобы их пропустить: `pytest -k "not TestResetEndpoint"`.

### Без платы (`--mock`)

Те же тесты `api/` и `web/` можно прогнать против `mock_server.py` — он отдаёт
страницу из `src/html_pages.h` и JSON в формате прошивки (соответствие полей
проверяет `ui/test_mock_contract.py`). Так они и запускаются в CI.

```bash
cd test
pytest --mock          # WebSocket-тесты (device_only) будут пропущены
```

### Только API тесты

```bash
# Bash тесты (быстро)
cd test/api
chmod +x test_api.sh
./test_api.sh --host 192.168.1.100

# Python тесты (детально)
cd test
pytest api/test_api.py -v
```

### Только Web тесты

```bash
cd test
pytest web/test_web_ui.py -v
```

---

## API тесты

### Bash тесты с curl

Быстрые smoke-тесты для проверки доступности API.

**Запуск:**

```bash
cd test/api
./test_api.sh --host 192.168.1.100
```

**Опции:**

```bash
./test_api.sh --help

Options:
  -h, --host IP       ESP32 IP address
  -v, --verbose       Verbose output
  -t, --timeout SEC   Request timeout
```

**Что тестируется:**

- ✅ Доступность устройства (ping)
- ✅ GET / - HTML страница
- ✅ GET /data - данные датчиков
- ✅ GET /stats - системная статистика
- ✅ GET /history - исторические данные
- ✅ GET /reset - сброс min/max
- ✅ 404 обработка
- ✅ CORS headers
- ✅ Время отклика
- ✅ Конкурентные запросы
- ✅ WebSocket порт

**Пример вывода:**

```
═══════════════════════════════════════════════════════════
  ESP32 Weather Station API Tests
═══════════════════════════════════════════════════════════

Target: http://192.168.1.100
Timeout: 5s

[INFO] Test #1: ESP32 connectivity check
[✓] ESP32 is reachable at 192.168.1.100

─────────────────────────────────────────────────────────
 HTTP Endpoints
─────────────────────────────────────────────────────────

[INFO] Test #2: GET / - Root endpoint (HTML page)
[✓] Root endpoint returns HTML page
[✓] HTML contains title
[✓] HTML contains temperature element
[✓] HTML contains humidity element

...

═══════════════════════════════════════════════════════════
  Test Results
═══════════════════════════════════════════════════════════

Total tests:  15
Passed:       15
Failed:       0

✓ All tests passed!
```

---

### Python тесты с pytest

Детальные функциональные тесты с проверкой данных.

**Запуск всех API тестов:**

```bash
cd test
pytest api/test_api.py -v
```

**Запуск конкретного класса тестов:**

```bash
# Только тесты /data endpoint
pytest api/test_api.py::TestDataEndpoint -v

# Только тесты /stats endpoint
pytest api/test_api.py::TestStatsEndpoint -v
```

**Запуск конкретного теста:**

```bash
pytest api/test_api.py::TestDataEndpoint::test_temperature_range -v
```

**Что тестируется:**

#### Root Endpoint (/)
- Возвращает HTML
- Содержит title
- Содержит элементы сенсоров
- Содержит JavaScript

#### Data Endpoint (/data)
- Доступность
- Валидный JSON
- Обязательные поля
- Диапазоны температуры (-40 до 85°C)
- Диапазоны влажности (0-100%)
- Согласованность min/max
- Timestamp формат

#### Stats Endpoint (/stats)
- Системная информация
- Данные батареи
- Напряжение батареи (0-5V)
- Процент батареи (0-100%)
- CPU usage
- RSSI диапазон (-100 до 0 dBm)

#### History Endpoint (/history)
- Структура данных
- Одинаковая длина массивов
- Максимальный размер (60 точек)

#### Performance
- Время отклика < 1s
- Конкурентные запросы
- Согласованность данных

**Пример вывода:**

```
test/api/test_api.py::TestRootEndpoint::test_root_returns_html PASSED
test/api/test_api.py::TestDataEndpoint::test_data_endpoint_accessible PASSED
test/api/test_api.py::TestDataEndpoint::test_temperature_range PASSED
test/api/test_api.py::TestStatsEndpoint::test_battery_fields PASSED
...

==================== 45 passed in 12.34s ====================
```

**С HTML отчетом:**

```bash
pytest api/test_api.py --html=report.html --self-contained-html
```

Откройте `report.html` в браузере для красивого отчета.

---

## Веб-тесты

End-to-end тестирование веб-интерфейса с Playwright.

### Запуск

**Headless режим (по умолчанию):**

```bash
cd test
pytest web/test_web_ui.py -v
```

**С видимым браузером:**

```bash
export HEADLESS=false
pytest web/test_web_ui.py -v --headed
```

**В slow-motion (для отладки):**

```bash
export SLOW_MO=100
pytest web/test_web_ui.py -v --headed --slowmo=100
```

### Что тестируется

#### Page Load
- ✅ Страница загружается
- ✅ Правильный title (`ENV Station`)
- ✅ Нет JavaScript ошибок
- ✅ Адаптивность (Desktop/Tablet/Mobile) — без горизонтального скролла

#### UI Elements
- ✅ Header виден, бейдж версии прошивки и времени сборки
- ✅ Карточки Temperature / Humidity / Dew point / Heat index
- ✅ System & control card
- ✅ Графики (History + 4 sparkline)
- ✅ Кнопки управления

#### Data Display
- ✅ Температура отображается
- ✅ Влажность отображается
- ✅ Информация о батарее
- ✅ Min/Max значения
- ✅ Системная статистика
- ✅ WiFi информация

#### Real-time Updates (ждут реальные интервалы: данные — 10 с, история — 15 с)
- ✅ Автоматическое обновление данных
- ✅ Status badge обновляется
- ✅ Графики обновляются

#### Interactions
- ✅ Переключение °C/°F
- ✅ Reset: отмена в диалоге не отправляет запрос на плату
- ✅ Export CSV
- ✅ Export JSON
- ✅ Serial monitor clear
- ✅ Сворачивание карточек

#### WebSocket (только на плате, `device_only`)
- ✅ WebSocket подключается
- ✅ Приветствие платы появляется в Serial monitor

#### Accessibility
- ✅ У каждой кнопки есть доступное имя (текст, `aria-label` или `title`)
- ✅ Изображения имеют alt

#### Performance
- ✅ Время загрузки < 3s
- ✅ Нет console errors

### Пример вывода

```
test/web/test_web_ui.py::TestPageLoad::test_page_loads_successfully PASSED
test/web/test_web_ui.py::TestPageLoad::test_page_title PASSED
test/web/test_web_ui.py::TestUIElements::test_temperature_card_visible PASSED
test/web/test_web_ui.py::TestDataDisplay::test_temperature_displays PASSED
test/web/test_web_ui.py::TestInteractions::test_temperature_unit_toggle PASSED
...

==================== 35 passed in 45.67s ====================
```

### Скриншоты и видео

**Сделать скриншот при ошибке:**

```bash
pytest web/test_web_ui.py --screenshot=on --video=retain-on-failure
```

Скриншоты сохраняются в `test-results/`

---

## GitHub Actions CI/CD

Автоматическое тестирование при каждом push и pull request.

### Workflow файл

`.github/workflows/ci.yml` запускается при push в любую ветку, на теги `v*`,
на pull request из форков и вручную (PR из этого же репозитория уже проверен
push-запуском своей ветки, поэтому второй раз не гоняется). Jobs:

1. **Build firmware + static analysis** — `pio run` (размер RAM/Flash — в summary
   запуска) и `pio check` (cppcheck, падает на дефектах medium/high)
2. **Unit tests (native)** — `pio test -e native`: юнит-тесты
   `WeatherCalculations` на хосте, без платы (`test/native/`)
3. **UI + API tests (mock device)** — без платы, против `mock_server.py`:
   `pytest ui api web --mock` и `test_api.sh`
4. **GitHub release** — только для тегов `v*`: проверяет, что тег совпадает с
   `FIRMWARE_VERSION` в `src/config.h`, и прикладывает прошивку к релизу

### Тесты без платы (локально)

```bash
# Юнит-тесты расчётов
pio test -e native

# UI- и API-тесты против mock-сервера
cd test
pip install -r requirements-ci.txt
python -m playwright install chromium
pytest --mock

# Mock-сервер отдельно — править UI без перепрошивки
python test/mock_server.py --port 8080   # → http://127.0.0.1:8080
```

### Использование

**Автоматический запуск:**

```bash
# При push в любую ветку
git push origin my-branch

# Релиз: тег должен совпадать с FIRMWARE_VERSION в src/config.h
git tag v3.1 && git push origin v3.1

# При создании PR
gh pr create --base main --head feature-branch
```

**Ручной запуск:**

```bash
# Через GitHub UI: Actions -> CI -> Run workflow

# Или через gh CLI
gh workflow run ci.yml
```

### Просмотр результатов

```bash
# Список запусков
gh run list

# Просмотр конкретного запуска
gh run view <run-id>

# Логи
gh run view <run-id> --log
```

### Artifacts

После каждого запуска (хранятся 7 дней):
- `firmware-esp32c3` — `firmware.bin`, `bootloader.bin`, `partitions.bin`
- `test-report` — HTML-отчёт pytest (в том числе при падении)

**Скачать:**

```bash
gh run download <run-id>
```

---

## Расширенное использование

### Параллельное выполнение

```bash
# Запустить тесты параллельно
pytest -n auto
```

### Маркеры

```bash
# Только API тесты
pytest -m api

# Только Web тесты
pytest -m web

# Только быстрые тесты (исключить slow)
pytest -m "not slow"
```

### Coverage отчет

```bash
# С HTML отчетом
pytest --cov=. --cov-report=html

# Открыть отчет
open htmlcov/index.html
```

### Отладка

```bash
# Остановиться при первой ошибке
pytest -x

# Показать все print()
pytest -s

# Полный traceback
pytest --tb=long

# Войти в debugger при ошибке
pytest --pdb
```

### Фильтрация тестов

```bash
# Запустить тесты по имени
pytest -k "temperature"

# Исключить тесты
pytest -k "not slow"

# Комбинация
pytest -k "api and not performance"
```

### Повторный запуск упавших тестов

```bash
# Запустить только упавшие
pytest --lf

# Запустить упавшие + следующий тест
pytest --ff
```

### Создание отчетов

**HTML отчет:**

```bash
pytest --html=reports/test-report.html --self-contained-html
```

**JUnit XML (для CI):**

```bash
pytest --junitxml=reports/junit.xml
```

**Allure отчет:**

```bash
# Запустить тесты
pytest --alluredir=allure-results

# Сгенерировать отчет
allure generate allure-results -o allure-report

# Открыть
allure open allure-report
```

---

## Continuous Integration Best Practices

### Pre-commit hooks

Создайте `.pre-commit-config.yaml`:

```yaml
repos:
  - repo: local
    hooks:
      - id: pytest-check
        name: pytest-check
        entry: pytest
        language: system
        pass_filenames: false
        always_run: true
```

Установите:

```bash
pip install pre-commit
pre-commit install
```

### Локальная симуляция CI

Те же проверки, что и в CI (из корня репозитория):

```bash
pio run                                        # сборка прошивки
pio check --skip-packages --fail-on-defect medium --fail-on-defect high
pio test -e native                             # юнит-тесты
(cd test && pytest --mock)                     # UI + API против mock-сервера
```

---

## Troubleshooting

### ESP32 не доступен

```bash
# Проверьте ping
ping 192.168.1.100

# Проверьте порт
nc -zv 192.168.1.100 80

# Проверьте IP
nmap -sn 192.168.1.0/24
```

### Playwright ошибки

```bash
# Переустановите browsers
playwright install --force

# Проверьте версию
playwright --version
```

### Timeout ошибки

```bash
# Увеличьте timeout
export TEST_TIMEOUT=10
pytest

# Или в коде
pytest --timeout=30
```

### Порты заняты

```bash
# Проверьте что WebSocket порт 81 открыт
lsof -i :81

# Убейте процесс если нужно
kill -9 <PID>
```

---

## Структура тестов

```
test/
├── .env.example              # Пример конфигурации
├── pytest.ini                # Конфигурация pytest
├── conftest.py               # Shared fixtures, опция --mock
├── mock_server.py            # Mock ESP32 (страница + JSON как у прошивки)
├── requirements.txt          # Python зависимости (полный набор)
├── requirements-ci.txt       # Минимальный набор для CI
│
├── api/
│   ├── test_api.sh           # Bash/curl smoke-тесты
│   └── test_api.py           # Python/pytest тесты
│
├── web/
│   └── test_web_ui.py        # Playwright E2E тесты
│
├── ui/                       # Только против mock-сервера
│   ├── test_dashboard.py     # Вёрстка, сворачивание, бейдж версии
│   └── test_mock_contract.py # Поля mock == поля в src/web_server.cpp
│
└── native/
    └── test_calculations/    # Unity-тесты на хосте: pio test -e native
```

---

## Метрики качества

### Цели покрытия

- **API Endpoints:** 100%
- **Error Handling:** 90%+
- **UI Components:** 80%+

### KPIs

- ⏱️ Время отклика API: < 1s
- 📄 Загрузка страницы: < 3s
- ✅ Success Rate: > 99%
- 🔄 Uptime: > 99.9%

---

## Дополнительные ресурсы

- [Pytest Documentation](https://docs.pytest.org/)
- [Playwright Documentation](https://playwright.dev/)
- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [ESP32 Documentation](https://docs.espressif.com/)

---

## Поддержка

Если у вас возникли проблемы:

1. Проверьте [Troubleshooting](#troubleshooting)
2. Откройте issue на GitHub
3. Проверьте Serial Monitor для логов ESP32

**Happy Testing!** 🧪✅
