#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

uint32_t global_variable = 0;

const uint32_t constant_variable = 42;

int main()
{
    stdio_init_all();
    while(1)
    {
        //вывод просто строковой константы
        printf("Hello World!\n");
        //вывод строки с аргументами
        printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
        //считать счетчик микросекунд и вывести его значение
        uint64_t timestamp = time_us_64();
        printf("system timestamp: %llu us\n", timestamp);
        //создать переменную на стеке и вывести ее адрес и значение в разных форматах
        uint32_t stack_variable = 8888;
        printf("stack variable | addr = 0x%X | value = %u\n", &stack_variable, *(&stack_variable));
        printf("stack variable | addr = 0x%X | value = %X\n", &stack_variable, *(&stack_variable));
        printf("stack variable | addr = 0x%X | value = 0x%X\n", &stack_variable, *(&stack_variable));
        //инкрементировать значение глобальной переменной и вывести ее значение и адрес
        global_variable++;
        printf("global variable | addr = 0x%X | value = %u\n", &global_variable, *(&global_variable));
        //создать переменную на куче, записать в нее значение и вывести ее значение и адрес
        uint32_t* heap_variable = (uint32_t*)malloc(sizeof(uint32_t));
        *heap_variable = 5555;
        printf("heap variable | addr = 0x%X | value = %u\n", heap_variable, *heap_variable);
        //добавить вывод адреса и значения постоянной
        printf("constant variable | addr = 0x%X | value = %u\n", &constant_variable, *(&constant_variable));
        //добавить вывод адреса и значения строковой постоянной
        printf("constant string | addr = 0x%X | value = 0x%X, [%s]\n", DEVICE_NAME, *((uint32_t*)DEVICE_NAME), DEVICE_NAME);
        //добавить вывод адреса и значения одного из множества регистров RP2040 с обращением прямо по адресу
        printf("reg chip id | addr = 0x%X | value = 0x%X\n", 0x40000000, *((uint32_t*)0x40000000));
        //добавить вывод адреса и значения `global_variable` с обращением прямо по адресу
        printf("var by addr | addr = 0x%X | value = %u\n", 0x20002278, *((uint32_t*)0x20002278));
        //добавить вывод адреса функции `main` и первых бинарных команд этой функции
        printf("main function | addr = 0x%X | value = 0x%X\n", main, *((uint32_t*)main));
        //Добавить задержку
        sleep_ms(1000);
    }
}