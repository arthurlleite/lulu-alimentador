#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void mqtt_app_start(QueueHandle_t xQueueMQTT);
void send_mqtt(char *data);