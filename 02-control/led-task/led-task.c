#include "led-task.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

const uint LED_PIN = 25;
uint LED_BLINK_PERIOD_US = 500000;

uint64_t led_ts;
led_state_t led_state;

void led_task_init(led_ts)
{

}

void led_task_handle(led_state)
{
    switch (led_state)
    {
        case LED_STATE_OFF:
	        // ваш код
	        break;
        case LED_STATE_ON:
	        // ваш код
	        break;
        case LED_STATE_BLINK:
	        // ваш код
	        break;
        default:
	        break;
    }
    if (time_us_64() > led_ts)
        {
	        led_ts = time_us_64() + (LED_BLINK_PERIOD_US / 2);
	        // ваш код
        }
}