#include "hardware/adc.h"
#include "adc-task.h"
#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"

#define GPIO_NUMBER 26
#define ADC_CHANEL_NUMBER 0
#define ADC_CHANEL_INNERTEMPRETURE 4

adc_task_state_t adc_state = ADC_TASK_STATE_IDLE;
uint64_t time_stamp;
uint64_t ADC_TASK_MEAS_PERIOD_US = 100000;

void adc_task_init()
{
    adc_init();
    adc_gpio_init(GPIO_NUMBER);
    adc_set_temp_sensor_enabled(true);
}

float adc_task_get_voltage()
{
    adc_select_input(ADC_CHANEL_NUMBER);
    uint16_t voltage_counts = adc_read();
    float voltage_V = voltage_counts * 3.3f/4096;
    return voltage_V;
}

float adc_task_get_temp()
{
    adc_select_input(ADC_CHANEL_INNERTEMPRETURE);
    uint16_t temp_counts = adc_read();
    float temp_V = temp_counts * 3.3f/4096;
    float temp_C = 27.0f - (temp_V - 0.706f) / 0.001721f;
    return temp_C;
}

void adc_task_set_state(adc_task_state_t state)
{
    adc_state = state;
}

void adc_task_handle(void) {
    if (adc_state != ADC_TASK_STATE_RUN) {
        return;
    }

    uint64_t now_stamp = time_us_64();
    if (now_stamp - time_stamp < ADC_TASK_MEAS_PERIOD_US) {
        return;
    }
    time_stamp = now_stamp;

    uint64_t uptime_ms = now_stamp / 1000;

    float voltage = adc_task_get_voltage();
    float temperature = adc_task_get_temp();

    printf("%f %f\n", voltage, temperature);
}