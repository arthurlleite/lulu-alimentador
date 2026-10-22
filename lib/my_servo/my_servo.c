#include "my_servo.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define GPIO_NUM GPIO_NUM_5
#define PWM_180deg 126
#define PWM_0deg 28
#define MAX_ROTATE_MS 800
#define TAG "SERVO"

static void ledc_init(void)
{
  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .timer_num = LEDC_TIMER_0,
      .duty_resolution = LEDC_TIMER_10_BIT,
      .freq_hz = 50,
      .clk_cfg = LEDC_AUTO_CLK};
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  ledc_channel_config_t ledc_channel = {
      .gpio_num = GPIO_NUM,
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LEDC_CHANNEL_0,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER_0,
      .duty = 0,
      .hpoint = 0};

  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

static uint32_t calc_duty(int angle_deg)
{
  return (PWM_180deg - PWM_0deg) * angle_deg / 180 + PWM_0deg;
}

static void one_rotation(int duration)
{
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, calc_duty(180)));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
  vTaskDelay(duration / portTICK_PERIOD_MS);
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, calc_duty(0)));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));
  vTaskDelay(MAX_ROTATE_MS / portTICK_PERIOD_MS);
}

void rotate_servo(int intensity)
{
  ESP_LOGI(TAG, "Rotating Servo");
  ledc_init();
  int n_rotation = intensity / 100;
  int remainder = (intensity % 100) * MAX_ROTATE_MS / 100;

  for (int i = 0; i < n_rotation; i++)
  {
    one_rotation(MAX_ROTATE_MS);
  }
  if (remainder > 0)
  {
    one_rotation(remainder);
  }
  ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
}