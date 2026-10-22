#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "my_wifi.h"
#include "my_mqtt.h"
#include "my_servo.h"
#include "my_softap.h"
#include "nvs_flash.h"
#include "my_server.h"
#include "mdns.h"
#include "my_ntp.h"
#include "my_mdns.h"
#include "my_timer.h"
#include "my_dispatcher.h"

TaskHandle_t xTask = NULL;
TaskHandle_t xTaskCheckTime = NULL;
TaskHandle_t xTaskMQTT = NULL;
TaskHandle_t xTaskServer = NULL;
QueueHandle_t xQueueMQTTIn = NULL;
QueueHandle_t xQueueMQTTOut = NULL;
QueueHandle_t xQueueFeed = NULL;

void task_receive_mqtt(void *arg)
{
  char *data = NULL;
  while (1)
  {
    xQueueReceive(xQueueMQTTIn, &data, portMAX_DELAY);
    dispatch(data, xQueueMQTTOut, xQueueFeed, xTaskCheckTime);
  }
}
void task_mqtt(void *arg)
{
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  mqtt_app_start(xQueueMQTTIn);
  ntp_init();

  while (1)
  {
    char *data = NULL;
    xQueueReceive(xQueueMQTTOut, &data, portMAX_DELAY);
    send_mqtt(data);
    free(data);
  }
}
void task_client(void *arg)
{
  wifi_init_sta(xTaskServer, xTaskMQTT);
  vTaskDelete(NULL);
}
void task_server(void *arg)
{
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  initialise_mdns();
  wifi_init_softap();
  server_init();
  vTaskDelay(10 * 60 * 1000 / portTICK_PERIOD_MS);
  esp_restart();
  vTaskDelete(NULL);
}
void task_check_time(void *arg)
{
  check_time(xQueueFeed);
}
void task_feed(void *arg)
{
  int intensity = 0;
  while (1)
  {
    xQueueReceive(xQueueFeed, &intensity, portMAX_DELAY);
    rotate_servo(intensity);
  }
}
void app_main(void)
{
  xQueueMQTTIn = xQueueCreate(3, 40 * sizeof(char));
  xQueueMQTTOut = xQueueCreate(3, 40 * sizeof(char));
  xQueueFeed = xQueueCreate(3, 40 * sizeof(int));
  xTaskCreate(task_receive_mqtt, "TaskReceiveMQTT", 2048, NULL, 0, &xTask);
  xTaskCreate(task_check_time, "CheckTime", 2048, NULL, 0, &xTaskCheckTime);
  xTaskCreate(task_client, "TaskClient", 3072, NULL, 0, NULL);
  xTaskCreate(task_mqtt, "TaskMQTT", 3072, NULL, 0, &xTaskMQTT);
  xTaskCreate(task_server, "TaskServer", 3072, NULL, 0, &xTaskServer);
  xTaskCreate(task_feed, "TaskFeed", 1024, NULL, 0, NULL);
}