#include "scheduler.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "app.h"
#include "feeder.h"

// O original (my_timer.c) calculava quanto dormir até o próximo horário; com a
// agenda vazia o tempo ficava negativo. Aqui a checagem é por minuto: cada
// horário dispara uma vez por dia e um ajuste de relógio (NTP) não pula refeição.

static const char *TAG = "agenda";

static void scheduler_task(void *arg)
{
  int last_fired[MAX_MEALS];
  for (int i = 0; i < MAX_MEALS; i++)
    last_fired[i] = -1;

  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (!app_time_valid())
      continue;

    time_t now = time(NULL);
    struct tm t;
    localtime_r(&now, &t);
    int key = t.tm_yday * 1440 + t.tm_hour * 60 + t.tm_min;

    meal_t meals[MAX_MEALS];
    app_lock();
    int count = g_app.cfg.meal_count;
    memcpy(meals, g_app.cfg.meals, sizeof(meals));
    app_unlock();

    for (int i = 0; i < count; i++)
    {
      if (!meals[i].enabled || meals[i].hour != t.tm_hour || meals[i].minute != t.tm_min)
        continue;
      if (last_fired[i] == key)
        continue;
      last_fired[i] = key;
      ESP_LOGI(TAG, "Refeição das %02d:%02d", meals[i].hour, meals[i].minute);
      feeder_request(meals[i].portions, SRC_SCHEDULE);
    }
  }
}

void scheduler_init(void)
{
  xTaskCreate(scheduler_task, "agenda", 3072, NULL, 5, NULL);
}
