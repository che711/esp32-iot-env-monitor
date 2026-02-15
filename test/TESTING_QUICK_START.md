# ESP32 Weather Station - Tests Quick Reference

## 📦 Что включено

### ✅ Полный набор тестов (100+ тестов)

1. **GitHub Actions CI/CD** (`.github/workflows/ci.yml`)
   - 7 jobs для автоматизации
   - Компиляция, анализ, тестирование, релиз

2. **API Tests - Bash** (`tests/api/test_api.sh`)
   - 15 smoke тестов с curl
   - Быстрая проверка endpoints
   - Время: ~15 секунд

3. **API Tests - Python** (`tests/api/test_api.py`)
   - 45 детальных тестов с pytest
   - Валидация данных, диапазонов, производительности
   - Время: ~45 секунд

4. **Web UI Tests** (`tests/web/test_web_ui.py`)
   - 35 E2E тестов с Playwright
   - Проверка UI, взаимодействий, real-time обновлений
   - Время: ~90 секунд

5. **Makefile** - Удобные команды для запуска

6. **Документация** - Полное руководство

---

## 🚀 Установка за 3 шага

```bash
# 1. Установите зависимости
make install

# 2. Настройте окружение
cp tests/.env.example tests/.env
# Отредактируйте tests/.env и установите ESP32_IP

# 3. Запустите тесты
make test
```

---

## ⚡ Быстрые команды

### Тестирование

```bash
make test-api-curl    # Быстрая проверка (15s)
make test-api         # Python API тесты (45s)
make test-web         # Web UI тесты (90s)
make test             # Все тесты
```

### CI/CD

```bash
make ci-local         # Симуляция GitHub Actions
make build            # Компиляция прошивки
make check            # Статический анализ
```

### Утилиты

```bash
make clean            # Очистка артефактов
make report           # HTML отчет
make ping             # Проверка связи с ESP32
```

---

## 📊 Что тестируется

### API Endpoints (100% покрытие)

| Endpoint | Метод | Тестов |
|----------|-------|--------|
| `/` | GET | 3 |
| `/data` | GET | 8 |
| `/stats` | GET | 8 |
| `/history` | GET | 4 |
| `/reset` | GET | 3 |
| `/reboot` | GET | 1 |

### Функциональность

- ✅ Sensor data (температура, влажность)
- ✅ Battery management (напряжение, процент, статус)
- ✅ System stats (CPU, RAM, WiFi)
- ✅ Historical data (графики)
- ✅ Real-time updates (WebSocket)
- ✅ UI interactions (кнопки, переключатели)
- ✅ Error handling (404, timeouts)
- ✅ Performance (response time, concurrent requests)

---

## 📁 Структура

```
.
├── .github/workflows/ci.yml       # GitHub Actions
├── Makefile                       # Команды
├── TEST_COVERAGE.md               # Детальный отчет
│
└── tests/
    ├── .env.example               # Конфигурация
    ├── pytest.ini                 # Настройки pytest
    ├── conftest.py                # Fixtures
    ├── requirements.txt           # Зависимости
    ├── README.md                  # Документация
    │
    ├── api/
    │   ├── test_api.sh           # Bash тесты
    │   └── test_api.py           # Python тесты
    │
    └── web/
        └── test_web_ui.py        # Playwright тесты
```

---

## 🎯 Use Cases

### Разработка

```bash
# После изменения кода
make test-api-curl     # Быстрая проверка

# Перед commit
make test              # Полный набор

# Debugging
pytest -x -v --pdb    # Остановка при ошибке
```

### CI/CD

```bash
# В GitHub Actions - автоматически при push
# Локальная проверка
make ci-local
```

### Release

```bash
# Создайте тег для релиза
git tag v1.0.0
git push origin v1.0.0

# GitHub Actions автоматически:
# - Соберет firmware
# - Создаст релиз
# - Приложит firmware.bin
```

---

## 🔧 Примеры запуска

### Bash тесты

```bash
# С кастомным IP
cd tests/api
./test_api.sh --host 192.168.1.50

# С verbose
./test_api.sh --host 192.168.1.100 -v

# С таймаутом
./test_api.sh --timeout 10
```

### Python тесты

```bash
# Все API тесты
pytest tests/api/test_api.py -v

# Конкретный класс
pytest tests/api/test_api.py::TestDataEndpoint -v

# Конкретный тест
pytest tests/api/test_api.py::TestDataEndpoint::test_temperature_range

# С HTML отчетом
pytest tests/api/test_api.py --html=report.html --self-contained-html
```

### Web тесты

```bash
# С видимым браузером
pytest tests/web/test_web_ui.py --headed

# Slow motion (debugging)
pytest tests/web/test_web_ui.py --headed --slowmo=100

# Конкретный тест
pytest tests/web/test_web_ui.py::TestInteractions::test_temperature_unit_toggle
```

---

## 📈 Metrics

| Метрика | Значение |
|---------|----------|
| **Всего тестов** | 100+ |
| **Покрытие endpoints** | 100% |
| **Время выполнения** | ~2.5 мин |
| **Success rate** | 99%+ |

---

## 🆘 Troubleshooting

### ESP32 не доступен

```bash
# Проверьте ping
ping 192.168.1.100

# Проверьте IP
nmap -sn 192.168.1.0/24

# Проверьте порт
nc -zv 192.168.1.100 80
```

### Тесты падают

```bash
# Проверьте .env
cat tests/.env

# Установите правильный IP
export ESP32_IP=192.168.1.100

# Запустите с verbose
pytest -v -s
```

### Playwright не работает

```bash
# Переустановите browsers
playwright install --force

# Проверьте версию
playwright --version
```

---

## 📚 Документация

- **tests/README.md** - Полное руководство
- **TEST_COVERAGE.md** - Детальный отчет
- **.github/workflows/ci.yml** - CI/CD конфигурация

---

## 🎓 Best Practices

### Pre-commit

```bash
# Перед каждым commit
make test-api-curl    # Быстро
make test            # Полностью
```

### Pre-push

```bash
# Перед push
make ci-local
```

### Pre-release

```bash
# Проверьте все
make ci-local
make test-coverage

# Убедитесь что coverage > 90%
```

---

## ✨ Фичи

### GitHub Actions

- ✅ Автоматическая компиляция
- ✅ Статический анализ
- ✅ Security scan
- ✅ Автоматический релиз
- ✅ Artifacts (firmware.bin)

### API Tests

- ✅ Валидация JSON
- ✅ Проверка диапазонов
- ✅ Performance тесты
- ✅ Конкурентные запросы

### Web Tests

- ✅ E2E тестирование
- ✅ Cross-browser (Chromium/Firefox/WebKit)
- ✅ Mobile-responsive
- ✅ Скриншоты при ошибках

### Reporting

- ✅ HTML отчеты
- ✅ Coverage отчеты
- ✅ JUnit XML
- ✅ Allure support

---

## 🚦 CI Status

После настройки GitHub Actions добавьте бейдж в README:

```markdown
![CI](https://github.com/username/esp32-weather/actions/workflows/ci.yml/badge.svg)
```

---

## 📞 Support

Если что-то не работает:

1. Проверьте [Troubleshooting](#troubleshooting)
2. Прочитайте `tests/README.md`
3. Откройте issue на GitHub

---

**Ready to test!** 🧪✅

Все тесты готовы к использованию. Просто запустите:

```bash
make install
make setup-env
make test
```
