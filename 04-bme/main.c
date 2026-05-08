#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task.h"
#include "led-task/led-task.h"
#include "hardware/i2c.h"
#include "bme280-driver.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

void rp2040_i2c_read(uint8_t* buffer, uint16_t length)
{
	i2c_read_timeout_us(i2c1, 0x76, buffer, length, false, 100000);
}

void rp2040_i2c_write(uint8_t* data, uint16_t size)
{
	i2c_write_timeout_us(i2c1, 0x76, data, size, false, 100000);
}

void help_callback(const char* args);

void version_callback(const char* args)
{
	printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

void led_on_callback(const char* args) 
{
    led_task_state_set(LED_STATE_ON);
}

void led_off_callback(const char* args) 
{
    led_task_state_set(LED_STATE_OFF);
}

void led_blink_callback(const char* args) 
{
    led_task_state_set(LED_STATE_BLINK);
}

void led_blink_set_period_ms_callback(const char* args)
{
    uint period_ms = 0;
    sscanf(args, "%u", &period_ms);
    if (period_ms == 0)
    {
        printf("Error: period is zero\n");
        return;
    }
    led_task_set_blink_period_ms(period_ms);
}

void mem_callback(const char* args)
{
    uint32_t addr;
    if (sscanf(args, "%x", &addr) != 1) {
        printf("Error: invalid address format.\n");
        return;
    }
    mem(addr);
}

void wmem_callback(const char* args)
{
    uint32_t addr, value;
    if (sscanf(args, "%x %x", &addr, &value) != 2) {
        printf("Error: invalid arguments.\n");
        return;
    }
    wmem(addr, value);
}

void read_reg_callback(const char* args)
{
    unsigned int addr, count;

    if (sscanf(args, "%x %x", &addr, &count) != 2) {
        printf("Usage: read_reg <addr_hex> <count_hex>\n");
        printf("Example: read_reg D0 3  (читает 3 регистра с адреса 0xD0)\n");
        return;
    }

    if (addr > 0xFF || count == 0 || count > 0xFF || addr + count > 0x100) {
        printf("Error: addr (0x%X) and count (0x%X) must satisfy:\n", addr, count);
        printf("  addr ≤ 0xFF, count between 1 and 0xFF, addr+count ≤ 0x100\n");
        return;
    }

    uint8_t buffer[256] = {0};
    bme280_read_regs((uint8_t)addr, buffer, (uint8_t)count);

    for (int i = 0; i < (int)count; i++) {
        printf("bme280 register [0x%X] = 0x%X\n", addr + i, buffer[i]);
    }
}

void write_reg_callback(const char* args)
{
    unsigned int addr, value;

    if (sscanf(args, "%x %x", &addr, &value) != 2) {
        printf("HUH\n");
        return;
    }

    if (addr > 0xFF || value > 0xFF) {
        printf("Error: addr (0x%X) and value (0x%X) must be <= 0xFF\n", addr, value);
        return;
    }

    bme280_write_reg((uint8_t)addr, (uint8_t)value);
    
    printf("Written 0x%02X to register [0x%02X]\n", (uint8_t)value, (uint8_t)addr);
}

void temp_raw_callback(const char* args) {
    printf("Raw Temperature: %u\n", bme280_read_temp_raw());
}

void pres_raw_callback(const char* args) {
    printf("Raw Pressure: %u\n", bme280_read_pres_raw());
}

void hum_raw_callback(const char* args) {
    printf("Raw Humidity: %u\n", bme280_read_hum_raw());
}

void temp_callback(const char* args) {
    printf("temp:%.2f\n", bme280_get_temperature());
}

void pres_callback(const char* args) {
    bme280_get_temperature(); 
    printf("pres:%.2f\n", bme280_get_pressure());
}

void hum_callback(const char* args) {
    bme280_get_temperature(); 
    printf("hum:%.2f\n", bme280_get_humidity());
}


api_t device_api[] =
{
    {"version", version_callback, "get device name and firmware version"},
    {"on", led_on_callback, "turns led on"},
    {"off", led_off_callback, "turns led off"},
    {"blink", led_blink_callback, "turns led in blink state"}, 
    {"set_period", led_blink_set_period_ms_callback, "set period for blink"},
    {"help", help_callback, "available commands list"},
    {"mem", mem_callback, "read from address"},
    {"wmem", wmem_callback, "write to address"},
    {"read_reg", read_reg_callback, "read registers on BME280"},
    {"write_reg", write_reg_callback, "write registers on BME280"},
    {"temp_raw", temp_raw_callback, "get temp in counts"},
    {"pres_raw", pres_raw_callback, "get pres in counts"},
    {"hum_raw", hum_raw_callback, "get hum in counts"},
    {"temp", temp_callback, "get temp in counts"},
    {"pres", pres_callback, "get pres in counts"},
    {"hum", hum_callback, "get hum in counts"},
    {NULL, NULL, NULL},
};

void help_callback(const char* args) 
{
    printf("Available commands:\n");
    for (int i = 0; device_api[i].command_name != NULL; i++) {
        printf("  %s: %s\n", device_api[i].command_name, device_api[i].command_help);
    }
}

int main ()
{
    i2c_init(i2c1, 100000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    stdio_init_all();
    stdio_task_init();
    protocol_task_init(device_api);
    led_task_init();
    bme280_init(rp2040_i2c_read, rp2040_i2c_write);
    while(1)
    {
        protocol_task_handle(stdio_task_handle());
        led_task_handle();
    }
}