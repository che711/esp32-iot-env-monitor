#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include "config.h"

// ============================================
// Событие кнопки
// ============================================
enum class ButtonEvent {
    NONE,         // Ничего не произошло
    SHORT_PRESS,  // Короткое нажатие (отдаётся в момент ОТПУСКАНИЯ)
    LONG_PRESS    // Долгое нажатие (отдаётся сразу по достижении порога)
};

// ============================================
// Класс тактовой кнопки с антидребезгом
// ============================================
// Полностью неблокирующий: никаких delay(), вся логика — на millis().
// Подключение по умолчанию: кнопка между пином и GND, внутренний pull-up.
class ButtonManager {
public:
    explicit ButtonManager(int pin, bool activeLow = true);

    void begin();

    // Вызывать в loop() на каждой итерации. Возвращает событие или NONE.
    ButtonEvent poll();

    // Текущее (уже отфильтрованное от дребезга) состояние
    bool isPressed() const { return _stableState; }

private:
    int  _pin;
    bool _activeLow;

    bool _stableState;    // Состояние после антидребезга: true = нажата
    bool _lastRawState;   // Последнее сырое чтение
    bool _longFired;      // LONG_PRESS уже отдан для текущего нажатия

    unsigned long _lastChangeTime;  // Когда сырое состояние менялось в последний раз
    unsigned long _pressStartTime;  // Когда началось текущее нажатие

    bool readRaw() const;
};

#endif // BUTTON_H
