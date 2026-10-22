#include "sensors.h"

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "app.h"
#include "mqtt_link.h"

#define PERIOD_MS 10000
#define PUBLISH_EVERY 6 // ciclos (~1 min) mesmo sem mudança

static const char *TAG = "sensores";

#if CONFIG_FEEDER_LEVEL_SENSOR
// Retorna a distância em mm ou -1 sem eco. O som anda 0,343 mm/us (ida e volta).
static int ultrasonic_read_mm(void)
{
  gpio_set_level(CONFIG_FEEDER_US_TRIG_GPIO, 1);
  esp_rom_delay_us(10);
  gpio_set_level(CONFIG_FEEDER_US_TRIG_GPIO, 0);

  int64_t start = esp_timer_get_time();
  while (gpio_get_level(CONFIG_FEEDER_US_ECHO_GPIO) == 0)
    if (esp_timer_get_time() - start > 30000)
      return -1;
  int64_t rise = esp_timer_get_time();
  while (gpio_get_level(CONFIG_FEEDER_US_ECHO_GPIO) == 1)
    if (esp_timer_get_time() - rise > 25000)
      return -1;
  int64_t us = esp_timer_get_time() - rise;
  return (int)(us * 343 / 2000);
}

static int cmp_int(const void *a, const void *b)
{
  return *(const int *)a - *(const int *)b;
}

// Mediana de 5 leituras: o tubo de 100 mm gera ecos nas paredes.
static int level_read_mm(void)
{
  int v[5], n = 0;
  for (int i = 0; i < 5; i++)
  {
    int d = ultrasonic_read_mm();
    if (d > 0)
      v[n++] = d;
    vTaskDelay(pdMS_TO_TICKS(60));
  }
  if (n < 3)
    return -1;
  qsort(v, n, sizeof(int), cmp_int);
  return v[n / 2];
}
#endif

#if CONFIG_FEEDER_DHT11
static int wait_level(int level, int timeout_us)
{
  int64_t start = esp_timer_get_time();
  while (gpio_get_level(CONFIG_FEEDER_DHT_GPIO) != level)
    if (esp_timer_get_time() - start > timeout_us)
      return -1;
  return (int)(esp_timer_get_time() - start);
}

// Protocolo de um fio do DHT11: 40 bits; nível alto > 40 us = bit 1.
static bool dht11_read(float *temp, float *hum)
{
  static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  uint8_t data[5] = {0};
  int pin = CONFIG_FEEDER_DHT_GPIO;

  gpio_set_direction(pin, GPIO_MODE_OUTPUT);
  gpio_set_level(pin, 0);
  vTaskDelay(pdMS_TO_TICKS(20));

  bool ok = true;
  portENTER_CRITICAL(&mux);
  gpio_set_level(pin, 1);
  esp_rom_delay_us(30);
  gpio_set_direction(pin, GPIO_MODE_INPUT);
  if (wait_level(0, 100) < 0 || wait_level(1, 100) < 0 || wait_level(0, 100) < 0)
    ok = false;
  for (int i = 0; ok && i < 40; i++)
  {
    if (wait_level(1, 70) < 0)
    {
      ok = false;
      break;
    }
    int high = wait_level(0, 100);
    if (high < 0)
    {
      ok = false;
      break;
    }
    data[i / 8] <<= 1;
    if (high > 40)
      data[i / 8] |= 1;
  }
  portEXIT_CRITICAL(&mux);

  if (!ok || ((data[0] + data[1] + data[2] + data[3]) & 0xFF) != data[4])
    return false;
  *hum = data[0] + data[1] / 10.0f;
  *temp = data[2] + data[3] / 10.0f;
  return true;
}
#endif

static void sensors_task(void *arg)
{
  int cycle = 0;
#if CONFIG_FEEDER_LEVEL_SENSOR
  int last_pct = -2;
  bool low_reported = false;
#endif
  for (;;)
  {
    app_lock();
    bool feeding = g_app.feeding;
    app_unlock();
    if (feeding)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    bool changed = false;
#if CONFIG_FEEDER_LEVEL_SENSOR
    int mm = level_read_mm();
    app_lock();
    int empty = g_app.cfg.level_empty_mm, full = g_app.cfg.level_full_mm;
    int pct = -1;
    if (mm > 0 && empty > full)
    {
      pct = (empty - mm) * 100 / (empty - full);
      pct = pct < 0 ? 0 : (pct > 100 ? 100 : pct);
    }
    g_app.level_pct = pct;
    g_app.level_cm = mm / 10.0f;
    bool low = pct >= 0 && pct < CONFIG_FEEDER_LOW_LEVEL_PCT;
    if (low && g_app.alarm == ALARM_NONE)
      g_app.alarm = ALARM_LOW_FOOD;
    else if (!low && g_app.alarm == ALARM_LOW_FOOD)
      g_app.alarm = ALARM_NONE;
    app_unlock();

    if (abs(pct - last_pct) >= 2)
    {
      changed = true;
      last_pct = pct;
    }
    if (low && !low_reported)
    {
      mqtt_link_publish_event("{\"type\":\"low_food\"}");
      ESP_LOGW(TAG, "Ração baixa: %d%%", pct);
    }
    low_reported = low;
#endif

#if CONFIG_FEEDER_DHT11
    if (cycle % 3 == 0) // DHT11 aceita no máximo 1 leitura/s; 30 s basta
    {
      float t, h;
      bool ok = dht11_read(&t, &h);
      app_lock();
      g_app.env_ok = ok;
      if (ok)
      {
        g_app.temp_c = t;
        g_app.hum_pct = h;
      }
      app_unlock();
    }
#endif

    if (changed || ++cycle % PUBLISH_EVERY == 0)
      app_changed();
    vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
  }
}

void sensors_init(void)
{
#if CONFIG_FEEDER_LEVEL_SENSOR
  gpio_reset_pin(CONFIG_FEEDER_US_TRIG_GPIO);
  gpio_set_direction(CONFIG_FEEDER_US_TRIG_GPIO, GPIO_MODE_OUTPUT);
  gpio_set_level(CONFIG_FEEDER_US_TRIG_GPIO, 0);
  gpio_reset_pin(CONFIG_FEEDER_US_ECHO_GPIO);
  gpio_set_direction(CONFIG_FEEDER_US_ECHO_GPIO, GPIO_MODE_INPUT);
#endif
#if CONFIG_FEEDER_DHT11
  gpio_reset_pin(CONFIG_FEEDER_DHT_GPIO);
  gpio_set_pull_mode(CONFIG_FEEDER_DHT_GPIO, GPIO_PULLUP_ONLY);
#endif
  xTaskCreate(sensors_task, "sensores", 3072, NULL, 3, NULL);
}
