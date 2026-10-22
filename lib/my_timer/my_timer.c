#include "my_timer.h"
#include "nvs_flash.h"
#include "time.h"
#include "my_servo.h"
#include "my_ntp.h"
#include "esp_log.h"

#define TAG "FEED_TIMER"

int load_timers(int *intensity, int **timers, int *numTimers)
{
  esp_err_t err_programedTimes, err_intensity;
  nvs_handle nvs_handle;

  // Open NVS namespace
  ESP_ERROR_CHECK(nvs_open("store", NVS_READWRITE, &nvs_handle));

  // Get the intensity value
  int intensity_value = 0;
  err_intensity = nvs_get_i32(nvs_handle, "intensity", &intensity_value);

  size_t timers_size;
  // Get the size of the programedTimes blob
  err_programedTimes = nvs_get_blob(nvs_handle, "programedTimes", NULL, &timers_size);

  if (err_programedTimes != ESP_OK || err_intensity != ESP_OK)
  {
    return 0;
  }
  
  // Allocate memory for programedTimes
  *timers = malloc(timers_size);

  // Calculate the number of timers
  *numTimers = timers_size / sizeof((*timers)[0]);

  // Get the programedTimes blob data
  nvs_get_blob(nvs_handle, "programedTimes", *timers, &timers_size);

  // set intensity pointer value
  *intensity = intensity_value;

  // Close NVS
  nvs_close(nvs_handle);
  return 1;
}

void check_time(QueueHandle_t xQueueFeed)
{
  ESP_ERROR_CHECK(nvs_flash_init());

  int intensity = 0;
  int *programedTimes = NULL;
  int numTimers = 0;
  if (load_timers(&intensity, &programedTimes, &numTimers) == 0)
  {
    if (programedTimes != NULL)
    {
      free(programedTimes);
      programedTimes = NULL;
    }
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }

  time_t now = 0;
  struct tm *time_struct;

  time(&now);
  time_struct = localtime(&now);
  while (time_struct->tm_year < (2023 - 1900))
  {
    time(&now);
    time_struct = localtime(&now);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

  while (1)
  {
    time(&now);
    time_struct = localtime(&now);

    int nowSeconds = time_struct->tm_hour * 60 * 60 + time_struct->tm_min * 60 + time_struct->tm_sec;
    int nextTime = 0;
    int waitTime = 0;

    for (int i = 0; i < numTimers; i++)
    {
      if (programedTimes[i] > nowSeconds)
      {
        nextTime = programedTimes[i];
        break;
      }
      if (i == numTimers - 1)
      {
        nextTime = programedTimes[0] + 24 * 60 * 60;
        break;
      }
    }
    waitTime = nextTime - nowSeconds;
    ESP_LOGI(TAG, "Proximo timer é %d, agora é %d, aguardando %d s\n", nextTime, nowSeconds, waitTime);
    if (ulTaskNotifyTake(pdTRUE, waitTime * 1000 / portTICK_PERIOD_MS))
    {
      if (programedTimes != NULL)
      {
        free(programedTimes);
        programedTimes = NULL;
      }
      load_timers(&intensity, &programedTimes, &numTimers);
    }
    else
    {
      xQueueSendToBack(xQueueFeed, &intensity, portMAX_DELAY);
    }
  }
  free(time_struct);
}