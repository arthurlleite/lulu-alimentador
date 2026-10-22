#include "interface.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "estado.h"
#include "dosador.h"
#include "rede_wifi.h"

// LED:  azul piscando = rede de configuração | amarelo piscando = conectando
//       ciano = Wi-Fi sem nuvem | verde = online | magenta = servindo
//       vermelho piscando por cima = ração baixa ou nada caiu
// Botão: toque curto = 1 porção | segurar 5 s = apagar Wi-Fi e reconfigurar.
// O botão BOOT da placa (GPIO0) faz o mesmo que o botão externo.

#define GPIO_BOOT 0
#define TOQUE_LONGO_MS 5000
#define PASSO_MS 50

static const char *TAG = "interface";

enum
{
  APAGADO = 0,
  VERMELHO = 1,
  VERDE = 2,
  AZUL = 4,
  AMARELO = VERMELHO | VERDE,
  CIANO = VERDE | AZUL,
  MAGENTA = VERMELHO | AZUL,
};

static void led_cor(int cor)
{
#if CONFIG_ALIM_LED_ANODO_COMUM
  const int aceso = 0;
#else
  const int aceso = 1;
#endif
  gpio_set_level(CONFIG_ALIM_GPIO_LED_R, (cor & VERMELHO) ? aceso : !aceso);
  gpio_set_level(CONFIG_ALIM_GPIO_LED_G, (cor & VERDE) ? aceso : !aceso);
  gpio_set_level(CONFIG_ALIM_GPIO_LED_B, (cor & AZUL) ? aceso : !aceso);
}

void interface_bipar(int vezes, int ms)
{
  for (int i = 0; i < vezes; i++)
  {
    gpio_set_level(CONFIG_ALIM_GPIO_BUZZER, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    gpio_set_level(CONFIG_ALIM_GPIO_BUZZER, 0);
    vTaskDelay(pdMS_TO_TICKS(ms));
  }
}

static int cor_do_status(int passo)
{
  bool pisca = (passo / 10) % 2; // 1 Hz
  estado_travar();
  status_rede_t rede = g_estado.rede;
  bool servindo = g_estado.servindo;
  alarme_t alarme = g_estado.alarme;
  estado_liberar();

  if (servindo)
    return MAGENTA;
  if (alarme != ALARME_NENHUM && (passo % 20) < 4)
    return VERMELHO;
  switch (rede)
  {
  case REDE_CONFIGURACAO:
    return pisca ? AZUL : APAGADO;
  case REDE_CONECTANDO:
    return pisca ? AMARELO : APAGADO;
  case REDE_WIFI:
    return CIANO;
  default:
    return VERDE;
  }
}

static bool botao_apertado(void)
{
  return gpio_get_level(GPIO_BOOT) == 0 || gpio_get_level(CONFIG_ALIM_GPIO_BOTAO) == 0;
}

static void tarefa_interface(void *arg)
{
  int passo = 0, segurando_ms = 0;
  bool longo_feito = false;
  for (;;)
  {
    if (botao_apertado())
    {
      segurando_ms += PASSO_MS;
      if (segurando_ms >= TOQUE_LONGO_MS && !longo_feito)
      {
        longo_feito = true;
        ESP_LOGW(TAG, "Botão segurado: apagando o Wi-Fi");
        interface_bipar(3, 200);
        rede_wifi_esquecer();
      }
    }
    else
    {
      if (segurando_ms >= 60 && segurando_ms < 1500)
      {
        ESP_LOGI(TAG, "Botão: 1 porção");
        dosador_pedir(1, ORIGEM_BOTAO);
      }
      segurando_ms = 0;
      longo_feito = false;
    }
    // Segurando o botão: pisca vermelho avisando que vai apagar o Wi-Fi.
    led_cor(segurando_ms >= 1500 ? (passo % 4 < 2 ? VERMELHO : APAGADO) : cor_do_status(passo));
    passo++;
    vTaskDelay(pdMS_TO_TICKS(PASSO_MS));
  }
}

void interface_iniciar(void)
{
  gpio_config_t saidas = {
      .pin_bit_mask = (1ULL << CONFIG_ALIM_GPIO_LED_R) | (1ULL << CONFIG_ALIM_GPIO_LED_G) |
                      (1ULL << CONFIG_ALIM_GPIO_LED_B) | (1ULL << CONFIG_ALIM_GPIO_BUZZER),
      .mode = GPIO_MODE_OUTPUT,
  };
  gpio_config(&saidas);
  gpio_set_level(CONFIG_ALIM_GPIO_BUZZER, 0);
  led_cor(APAGADO);

  gpio_config_t entradas = {
      .pin_bit_mask = (1ULL << GPIO_BOOT) | (1ULL << CONFIG_ALIM_GPIO_BOTAO),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_ENABLE,
  };
  gpio_config(&entradas);

  xTaskCreate(tarefa_interface, "interface", 3072, NULL, 4, NULL);
}
