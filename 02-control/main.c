#include "stdio.h"
#include "stdlib.h"
#include "pico/stdlib.h"
#include "stdio-task/stdio-task.h"
#include "protocol-task/protocol-task.h"

#define DEVICE_NAME "my-pico-device"
#define DEVICE_VRSN "v0.0.1"

void version_callback(const char* args)
{
	printf("device name: '%s', firmware version: %s\n", DEVICE_NAME, DEVICE_VRSN);
}

int main ()
{
    stdio_init_all();
    stdio_task_init();

    api_t device_api[] =
    {
        {"version", version_callback, "get device name and firmware version"},
        {NULL, NULL, NULL},
    };

    protocol_task_init(device_api);
    
    while(1)
    {
        stdio_task_handle();
        protocol_task_handle(stdio_task_handle());
    }
}