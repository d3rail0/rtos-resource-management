#pragma once

#include "main.h"

struct Led
{
private:
    GPIO_TypeDef* _gpio;
    uint16_t _pin;

public:

    explicit Led(GPIO_TypeDef* t_gpio, uint16_t t_pin)
        : _gpio(t_gpio)
        , _pin(t_pin)

    {}

    void on()
    {
        HAL_GPIO_WritePin(_gpio, _pin, GPIO_PIN_SET);
    }

    void off()
    {
        HAL_GPIO_WritePin(_gpio, _pin, GPIO_PIN_RESET);
    }

    void toggle()
    {
        HAL_GPIO_TogglePin(_gpio, _pin);
    }

    auto state() -> bool
    {
        return HAL_GPIO_ReadPin(_gpio, _pin);
    }

};