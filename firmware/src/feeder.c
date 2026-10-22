#include "feeder.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "mqtt_link.h"
#include "ui.h"

// Servo de posição (MG996R/MG90S): 50 Hz, pulso de 0,5 a 2,5 ms.
// Cada porção abre a passagem (ângulo aberto), espera e fecha, como no projeto
// original (my_servo.c). Com o sensor IR na saída dá para saber se a ração caiu.

#define SERVO_MODE LEDC_LOW_SPEED_MODE
#define SERVO_TIMER LEDC_TIMER_0
#define SERVO_CHANNEL LEDC_CHANNEL_0
#define SERVO_RES_BITS 14
#define SERVO_PERIOD_US 20000

typedef struct
{
  uint8_t portions;
  feed_source_t src;
} feed_req_t;

static const char *TAG = "feeder";
static QueueHandle_t s_queue;
static volatile uint32_t s_food_pulses;

#if CONFIG_FEEDER_FOOD_SENSOR
static void IRAM_ATTR food_isr(void *arg)
{
  s_food_pulses++;
}
#endif

static void servo_init(void)
{
  ledc_timer_config_t timer = {
      .speed_mode = SERVO_MODE,
      .timer_num = SERVO_TIMER,
      .duty_resolution = SERVO_RES_BITS,
      .freq_hz = 50,
      .clk_cfg = LEDC_AUTO_CLK,
  };
  ESP_ERROR_CHECK(ledc_timer_config(&timer));
  ledc_channel_config_t ch = {
      .gpio_num = CONFIG_FEEDER_SERVO_GPIO,
      .speed_mode = SERVO_MODE,
      .channel = SERVO_CHANNEL,
      .timer_sel = SERVO_TIMER,
      .intr_type = LEDC_INTR_DISABLE,
      .duty = 0,
      .hpoint = 0,
  };
  ESP_ERROR_CHECK(ledc_channel_config(&ch));
}

static void servo_write(int angle)
{
  uint32_t us = CONFIG_FEEDER_SERVO_MIN_US +
                (CONFIG_FEEDER_SERVO_MAX_US - CONFIG_FEEDER_SERVO_MIN_US) * angle / 180;
  uint32_t duty = us * ((1 << SERVO_RES_BITS) - 1) / SERVO_PERIOD_US;
  ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, duty);
  ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);
}

// Corta o PWM: o servo para de forçar (sem zumbido nem consumo parado).
static void servo_release(void)
{
  ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, 0);
  ledc_update_duty(SERVO_MODE, SERVO_CHANNEL);
}

static void dispense_portion(void)
{
  servo_write(CONFIG_FEEDER_SERVO_OPEN_ANGLE);
  vTaskDelay(pdMS_TO_TICKS(CONFIG_FEEDER_OPEN_MS));
  servo_write(CONFIG_FEEDER_SERVO_CLOSED_ANGLE);
  vTaskDelay(pdMS_TO_TICKS(CONFIG_FEEDER_CLOSE_MS));
}

#if CONFIG_FEEDER_FOOD_SENSOR
// Pequenas sacudidas para soltar grãos presos antes de tentar de novo.
static void shake(void)
{
  int base = CONFIG_FEEDER_SERVO_CLOSED_ANGLE;
  int delta = CONFIG_FEEDER_SERVO_OPEN_ANGLE > base ? 30 : -30;
  for (int i = 0; i < 4; i++)
  {
    servo_write(base + delta);
    vTaskDelay(pdMS_TO_TICKS(150));
    servo_write(base);
    vTaskDelay(pdMS_TO_TICKS(150));
  }
}
#endif

static void publish_fed_event(const feed_req_t *req, bool ok)
{
  char ev[120];
  snprintf(ev, sizeof(ev), "{\"type\":\"fed\",\"p\":%u,\"src\":\"%s\",\"ok\":%s}",
           req->portions, app_source_name(req->src), ok ? "true" : "false");
  mqtt_link_publish_event(ev);
}

static void feeder_task(void *arg)
{
  feed_req_t req;
  for (;;)
  {
    xQueueReceive(s_queue, &req, portMAX_DELAY);
    ESP_LOGI(TAG, "Servindo %u porção(ões) [%s]", req.portions, app_source_name(req.src));

    app_lock();
    g_app.feeding = true;
    bool beep = g_app.cfg.beep;
    app_unlock();
    app_changed();

    if (beep)
      ui_beep(2, 120); // chama o pet

    s_food_pulses = 0;
    for (int i = 0; i < req.portions; i++)
      dispense_portion();

    bool ok = true;
#if CONFIG_FEEDER_FOOD_SENSOR
    if (s_food_pulses == 0)
    {
      ESP_LOGW(TAG, "Sensor IR não viu ração cair; sacudindo e tentando de novo");
      shake();
      dispense_portion();
      ok = s_food_pulses > 0;
    }
    ESP_LOGI(TAG, "Pulsos no sensor IR: %lu", (unsigned long)s_food_pulses);
#endif
    servo_release();

    if (!ok)
      ui_beep(4, 60);
    app_record_feed(req.portions, req.src, ok);
    publish_fed_event(&req, ok);
    app_changed();
  }
}

void feeder_init(void)
{
  servo_init();
  servo_write(CONFIG_FEEDER_SERVO_CLOSED_ANGLE);
  vTaskDelay(pdMS_TO_TICKS(400));
  servo_release();

#if CONFIG_FEEDER_FOOD_SENSOR
  gpio_config_t io = {
      .pin_bit_mask = 1ULL << CONFIG_FEEDER_FOOD_GPIO,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .intr_type = GPIO_INTR_NEGEDGE, // módulos IR puxam a saída para 0 ao detectar
  };
  gpio_config(&io);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(CONFIG_FEEDER_FOOD_GPIO, food_isr, NULL);
#endif

  s_queue = xQueueCreate(4, sizeof(feed_req_t));
  xTaskCreate(feeder_task, "feeder", 4096, NULL, 6, NULL);
}

bool feeder_request(uint8_t portions, feed_source_t src)
{
  if (portions < 1)
    portions = 1;
  if (portions > MAX_PORTIONS)
    portions = MAX_PORTIONS;
  feed_req_t req = {.portions = portions, .src = src};
  return xQueueSend(s_queue, &req, 0) == pdTRUE;
}
