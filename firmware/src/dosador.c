#include "dosador.h"

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "nuvem_mqtt.h"
#include "interface.h"

// Servo de posição (MG996R/MG90S): 50 Hz, pulso de 0,5 a 2,5 ms.
// Cada porção abre a passagem (ângulo aberto), espera e fecha, como no projeto
// original (my_servo.c). Com o sensor IR na saída dá para saber se a ração caiu.

#define SERVO_MODO LEDC_LOW_SPEED_MODE
#define SERVO_TIMER LEDC_TIMER_0
#define SERVO_CANAL LEDC_CHANNEL_0
#define SERVO_BITS 14
#define SERVO_PERIODO_US 20000

typedef struct
{
  uint8_t porcoes;
  origem_t origem;
} pedido_t;

static const char *TAG = "dosador";
static QueueHandle_t s_fila;
static volatile uint32_t s_pulsos_saida;

#if CONFIG_ALIM_SENSOR_SAIDA
static void IRAM_ATTR isr_sensor_saida(void *arg)
{
  s_pulsos_saida++;
}
#endif

static void servo_iniciar(void)
{
  ledc_timer_config_t timer = {
      .speed_mode = SERVO_MODO,
      .timer_num = SERVO_TIMER,
      .duty_resolution = SERVO_BITS,
      .freq_hz = 50,
      .clk_cfg = LEDC_AUTO_CLK,
  };
  ESP_ERROR_CHECK(ledc_timer_config(&timer));
  ledc_channel_config_t canal = {
      .gpio_num = CONFIG_ALIM_GPIO_SERVO,
      .speed_mode = SERVO_MODO,
      .channel = SERVO_CANAL,
      .timer_sel = SERVO_TIMER,
      .intr_type = LEDC_INTR_DISABLE,
      .duty = 0,
      .hpoint = 0,
  };
  ESP_ERROR_CHECK(ledc_channel_config(&canal));
}

static void servo_angulo(int graus)
{
  uint32_t us = CONFIG_ALIM_SERVO_MIN_US +
                (CONFIG_ALIM_SERVO_MAX_US - CONFIG_ALIM_SERVO_MIN_US) * graus / 180;
  uint32_t duty = us * ((1 << SERVO_BITS) - 1) / SERVO_PERIODO_US;
  ledc_set_duty(SERVO_MODO, SERVO_CANAL, duty);
  ledc_update_duty(SERVO_MODO, SERVO_CANAL);
}

// Corta o PWM: o servo para de forçar (sem zumbido nem consumo parado).
static void servo_soltar(void)
{
  ledc_set_duty(SERVO_MODO, SERVO_CANAL, 0);
  ledc_update_duty(SERVO_MODO, SERVO_CANAL);
}

static void servir_porcao(void)
{
  servo_angulo(CONFIG_ALIM_ANGULO_ABERTO);
  vTaskDelay(pdMS_TO_TICKS(CONFIG_ALIM_TEMPO_ABERTO_MS));
  servo_angulo(CONFIG_ALIM_ANGULO_FECHADO);
  vTaskDelay(pdMS_TO_TICKS(CONFIG_ALIM_TEMPO_FECHAR_MS));
}

#if CONFIG_ALIM_SENSOR_SAIDA
// Pequenas sacudidas para soltar grãos presos antes de tentar de novo.
static void sacudir(void)
{
  int base = CONFIG_ALIM_ANGULO_FECHADO;
  int delta = CONFIG_ALIM_ANGULO_ABERTO > base ? 30 : -30;
  for (int i = 0; i < 4; i++)
  {
    servo_angulo(base + delta);
    vTaskDelay(pdMS_TO_TICKS(150));
    servo_angulo(base);
    vTaskDelay(pdMS_TO_TICKS(150));
  }
}
#endif

static void avisar_refeicao(const pedido_t *p, bool ok)
{
  char ev[128];
  snprintf(ev, sizeof(ev), "{\"tipo\":\"alimentado\",\"p\":%u,\"origem\":\"%s\",\"ok\":%s}",
           p->porcoes, estado_nome_origem(p->origem), ok ? "true" : "false");
  nuvem_publicar_evento(ev);
}

static void tarefa_dosador(void *arg)
{
  pedido_t p;
  for (;;)
  {
    xQueueReceive(s_fila, &p, portMAX_DELAY);
    ESP_LOGI(TAG, "Servindo %u porção(ões) [%s]", p.porcoes, estado_nome_origem(p.origem));

    estado_travar();
    g_estado.servindo = true;
    bool bip = g_estado.cfg.bip;
    estado_liberar();
    estado_mudou();

    if (bip)
      interface_bipar(2, 120); // chama o pet

    s_pulsos_saida = 0;
    for (int i = 0; i < p.porcoes; i++)
      servir_porcao();

    bool ok = true;
#if CONFIG_ALIM_SENSOR_SAIDA
    if (s_pulsos_saida == 0)
    {
      ESP_LOGW(TAG, "O sensor IR não viu ração cair; sacudindo e tentando de novo");
      sacudir();
      servir_porcao();
      ok = s_pulsos_saida > 0;
    }
    ESP_LOGI(TAG, "Pulsos no sensor IR: %lu", (unsigned long)s_pulsos_saida);
#endif
    servo_soltar();

    if (!ok)
      interface_bipar(4, 60);
    estado_registrar_refeicao(p.porcoes, p.origem, ok);
    avisar_refeicao(&p, ok);
    estado_mudou();
  }
}

void dosador_iniciar(void)
{
  servo_iniciar();
  servo_angulo(CONFIG_ALIM_ANGULO_FECHADO);
  vTaskDelay(pdMS_TO_TICKS(400));
  servo_soltar();

#if CONFIG_ALIM_SENSOR_SAIDA
  gpio_config_t io = {
      .pin_bit_mask = 1ULL << CONFIG_ALIM_GPIO_SAIDA,
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
      .intr_type = GPIO_INTR_NEGEDGE, // módulos IR puxam a saída para 0 ao detectar
  };
  gpio_config(&io);
  gpio_install_isr_service(0);
  gpio_isr_handler_add(CONFIG_ALIM_GPIO_SAIDA, isr_sensor_saida, NULL);
#endif

  s_fila = xQueueCreate(4, sizeof(pedido_t));
  xTaskCreate(tarefa_dosador, "dosador", 4096, NULL, 6, NULL);
}

bool dosador_pedir(uint8_t porcoes, origem_t origem)
{
  if (porcoes < 1)
    porcoes = 1;
  if (porcoes > MAX_PORCOES)
    porcoes = MAX_PORCOES;
  pedido_t p = {.porcoes = porcoes, .origem = origem};
  return xQueueSend(s_fila, &p, 0) == pdTRUE;
}
