#include "ui.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "app.h"
#include "feeder.h"
#include "wifi_mgr.h"

// LED:  azul piscando = rede de configuração | amarelo piscando = conectando
//       ciano = Wi-Fi sem nuvem | verde = online | magenta = servindo
//       vermelho piscando por cima = ração baixa ou nada caiu
// Botão: toque curto = 1 porção | segurar 5 s = apagar Wi-Fi e reconfigurar.
// O botão BOOT da placa (GPIO0) faz o mesmo que o botão externo.

#define BOOT_GPIO 0
#define LONG_PRESS_MS 5000
#define TICK_MS 50

static const char *TAG = "ui";

enum
{
  OFF = 0,
  RED = 1,
  GREEN = 2,
  BLUE = 4,
  YELLOW = RED | GREEN,
  CYAN = GREEN | BLUE,
  MAGENTA = RED | BLUE,
};

static void led_set(int color)
{
#if CONFIG_FEEDER_LED_COMMON_ANODE
  const int on = 0;
#else
  const int on = 1;
#endif
  gpio_set_level(CONFIG_FEEDER_LED_R_GPIO, (color & RED) ? on : !on);
  gpio_set_level(CONFIG_FEEDER_LED_G_GPIO, (color & GREEN) ? on : !on);
  gpio_set_level(CONFIG_FEEDER_LED_B_GPIO, (color & BLUE) ? on : !on);
}

void ui_beep(int count, int ms)
{
  for (int i = 0; i < count; i++)
  {
    gpio_set_level(CONFIG_FEEDER_BUZZER_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    gpio_set_level(CONFIG_FEEDER_BUZZER_GPIO, 0);
    vTaskDelay(pdMS_TO_TICKS(ms));
  }
}

static int status_color(int tick)
{
  bool blink = (tick / 10) % 2; // 1 Hz
  app_lock();
  net_status_t net = g_app.net;
  bool feeding = g_app.feeding;
  alarm_t alarm = g_app.alarm;
  app_unlock();

  if (feeding)
    return MAGENTA;
  if (alarm != ALARM_NONE && (tick % 20) < 4)
    return RED;
  switch (net)
  {
  case NET_SETUP:
    return blink ? BLUE : OFF;
  case NET_CONNECTING:
    return blink ? YELLOW : OFF;
  case NET_WIFI:
    return CYAN;
  default:
    return GREEN;
  }
}

static bool button_pressed(void)
{
  return gpio_get_level(BOOT_GPIO) == 0 || gpio_get_level(CONFIG_FEEDER_BUTTON_GPIO) == 0;
}

static void ui_task(void *arg)
{
  int tick = 0, held_ms = 0;
  bool long_done = false;
  for (;;)
  {
    if (button_pressed())
    {
      held_ms += TICK_MS;
      if (held_ms >= LONG_PRESS_MS && !long_done)
      {
        long_done = true;
        ESP_LOGW(TAG, "Botão segurado: apagando Wi-Fi");
        ui_beep(3, 200);
        wifi_mgr_factory_reset();
      }
    }
    else
    {
      if (held_ms >= 60 && held_ms < 1500)
      {
        ESP_LOGI(TAG, "Botão: 1 porção");
        feeder_request(1, SRC_BUTTON);
      }
      held_ms = 0;
      long_done = false;
    }
    // Segurando o botão: pisca vermelho avisando que vai apagar o Wi-Fi.
    led_set(held_ms >= 1500 ? (tick % 4 < 2 ? RED : OFF) : status_color(tick));
    tick++;
    vTaskDelay(pdMS_TO_TICKS(TICK_MS));
  }
}

void ui_init(void)
{
  gpio_config_t out = {
      .pin_bit_mask = (1ULL << CONFIG_FEEDER_LED_R_GPIO) | (1ULL << CONFIG_FEEDER_LED_G_GPIO) |
                      (1ULL << CONFIG_FEEDER_LED_B_GPIO) | (1ULL << CONFIG_FEEDER_BUZZER_GPIO),
      .mode = GPIO_MODE_OUTPUT,
  };
  gpio_config(&out);
  gpio_set_level(CONFIG_FEEDER_BUZZER_GPIO, 0);
  led_set(OFF);

  gpio_config_t in = {
      .pin_bit_mask = (1ULL << BOOT_GPIO) | (1ULL << CONFIG_FEEDER_BUTTON_GPIO),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
  };
  gpio_config(&in);

  xTaskCreate(ui_task, "ui", 3072, NULL, 4, NULL);
}
