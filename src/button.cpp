#include "button.h"

ButtonManager::ButtonManager(int pin, bool activeLow)
    : _pin(pin), _activeLow(activeLow),
      _stableState(false), _lastRawState(false), _longFired(false),
      _lastChangeTime(0), _pressStartTime(0) {
}

void ButtonManager::begin() {
    pinMode(_pin, _activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);

    // Синхронизируем стартовое состояние, чтобы не поймать фантомное
    // нажатие на первой же итерации loop()
    _lastRawState   = readRaw();
    _stableState    = _lastRawState;
    _lastChangeTime = millis();

    Serial.printf("✓ Button initialized on GPIO%d (%s, internal %s)\n",
                  _pin,
                  _activeLow ? "active LOW" : "active HIGH",
                  _activeLow ? "pull-up" : "pull-down");
}

bool ButtonManager::readRaw() const {
    int level = digitalRead(_pin);
    return _activeLow ? (level == LOW) : (level == HIGH);
}

ButtonEvent ButtonManager::poll() {
    unsigned long now = millis();
    bool raw = readRaw();

    // Сырое состояние дёрнулось — перезапускаем окно антидребезга
    if (raw != _lastRawState) {
        _lastRawState   = raw;
        _lastChangeTime = now;
        return ButtonEvent::NONE;
    }

    // Состояние держится дольше BUTTON_DEBOUNCE_MS — принимаем его как настоящее
    if (raw != _stableState && (now - _lastChangeTime) >= BUTTON_DEBOUNCE_MS) {
        _stableState = raw;

        if (_stableState) {
            // Нажатие началось
            _pressStartTime = now;
            _longFired      = false;
        } else {
            // Отпустили. Короткое нажатие засчитываем, только если
            // LONG_PRESS для этого удержания ещё не отдавался —
            // иначе одно удержание дало бы два события подряд.
            if (!_longFired) {
                return ButtonEvent::SHORT_PRESS;
            }
        }
        return ButtonEvent::NONE;
    }

    // Кнопку держат — проверяем порог долгого нажатия
    if (_stableState && !_longFired &&
        (now - _pressStartTime) >= BUTTON_LONG_PRESS_MS) {
        _longFired = true;
        return ButtonEvent::LONG_PRESS;
    }

    return ButtonEvent::NONE;
}
