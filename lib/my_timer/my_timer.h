#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

int load_timers(int *intensity, int **timers, int *numTimers);
void check_time(QueueHandle_t xQueueFeed);