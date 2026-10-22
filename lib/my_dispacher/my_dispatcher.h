#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

void dispatch(char *data, QueueHandle_t xQueueMQTTOut, QueueHandle_t xQueueFeed, TaskHandle_t xTaskCheckTime);