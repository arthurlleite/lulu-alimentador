#include "my_dispatcher.h"
#include "cJSON.h"
#include "nvs_flash.h"
#include "string.h"
#include "stdio.h"
#include "my_timer.h"

void dispatch(char *data, QueueHandle_t xQueueMQTTOut, QueueHandle_t xQueueFeed, TaskHandle_t xTaskCheckTime)
{
  char *msg;
  msg = malloc(200);
  cJSON *data_JSON = cJSON_Parse(data);
  if (strcmp(cJSON_GetObjectItem(data_JSON, "cmd")->valuestring, "CONFIG") == 0)
  {
    // Extract intensity value
    int intensity = atoi(cJSON_GetObjectItem(data_JSON, "intensity")->valuestring);

    // Get the timers array from JSON
    cJSON *timers_JSON = cJSON_GetObjectItem(data_JSON, "timers");

    int numberOfTimers = cJSON_GetArraySize(timers_JSON);

    if (numberOfTimers < 1)
    {
      return;
    }

    // Initialize the array to store timers
    int *programedTimes = NULL;
    programedTimes = malloc(numberOfTimers * sizeof(int));

    // Load timers into the array
    for (int i = 0; i < numberOfTimers; i++)
    {
      cJSON *timer_JSON = cJSON_GetArrayItem(timers_JSON, i);
      programedTimes[i] = atoi(timer_JSON->valuestring);
    }

    // Open NVS
    nvs_handle nvs_handle;
    ESP_ERROR_CHECK(nvs_open("store", NVS_READWRITE, &nvs_handle));

    // // Store intensity and programedTimes in NVS
    ESP_ERROR_CHECK(nvs_set_i32(nvs_handle, "intensity", intensity));
    ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, "programedTimes", programedTimes, numberOfTimers * sizeof(int)));

    // Commit changes
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));

    // Close NVS
    nvs_close(nvs_handle);

    free(programedTimes);

    // Send success message to MQTT output queue
    sprintf(msg, "PROGRAMADO COM SUCESSO");
    xQueueSendToBack(xQueueMQTTOut, &msg, portMAX_DELAY);
    xTaskNotifyGive(xTaskCheckTime);
  }
  if (strcmp(cJSON_GetObjectItem(data_JSON, "cmd")->valuestring, "NOW") == 0)
  {
    int intensity = 0;
    intensity = atoi(cJSON_GetObjectItem(data_JSON, "intensity")->valuestring);
    xQueueSendToBack(xQueueFeed, &intensity, portMAX_DELAY);
    sprintf(msg, "ALIMENTADO COM SUCESSO");
    xQueueSendToBack(xQueueMQTTOut, (void *)&(msg), portMAX_DELAY);
  }
  if (strcmp(cJSON_GetObjectItem(data_JSON, "cmd")->valuestring, "PING") == 0)
  {
    int *timers = NULL, intensity = 0, numTimers = 0;
    load_timers(&intensity, &timers, &numTimers);
    sprintf(msg, "Programação. Intensidade: %d. Horários: ", intensity);
    char temp_buffer[50];
    for (int i = 0; i < numTimers; i++)
    {
      sprintf(temp_buffer, " %dh%dmin%ds", timers[i] / 3600, (timers[i] % 3600) / 60, timers[i] % 60);
      strcat(msg, temp_buffer); // Copy the formatted string back to msg
    }
    if (timers != NULL)
    {
      free(timers);
      timers = NULL;
    }
    xQueueSendToBack(xQueueMQTTOut, (void *)&(msg), portMAX_DELAY);
  }
  // Free cJSON object after use
  cJSON_Delete(data_JSON);
}