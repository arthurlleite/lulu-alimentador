#include "sensores.h"

#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"
#include "esp_log.h"
#include "estado.h"
#include "nuvem_mqtt.h"
#include "interface.h"

#define PERIODO_MS 10000
#define PUBLICAR_A_CADA 6 // ciclos (~1 min) mesmo sem mudança

static const char *TAG = "sensores";

#if CONFIG_ALIM_SENSOR_NIVEL
// Retorna a distância em mm ou -1 sem eco. O som anda 0,343 mm/us (ida e volta).
static int ultrassom_mm(void)
{
  gpio_set_level(CONFIG_ALIM_GPIO_TRIG, 1);
  esp_rom_delay_us(10);
  gpio_set_level(CONFIG_ALIM_GPIO_TRIG, 0);

  int64_t inicio = esp_timer_get_time();
  while (gpio_get_level(CONFIG_ALIM_GPIO_ECHO) == 0)
    if (esp_timer_get_time() - inicio > 30000)
      return -1;
  int64_t subida = esp_timer_get_time();
  while (gpio_get_level(CONFIG_ALIM_GPIO_ECHO) == 1)
    if (esp_timer_get_time() - subida > 25000)
      return -1;
  int64_t us = esp_timer_get_time() - subida;
  return (int)(us * 343 / 2000);
}

static int comparar_int(const void *a, const void *b)
{
  return *(const int *)a - *(const int *)b;
}

// Mediana de 5 leituras: o tubo de 100 mm gera ecos nas paredes.
static int ler_nivel_mm(void)
{
  int v[5], n = 0;
  for (int i = 0; i < 5; i++)
  {
    int d = ultrassom_mm();
    if (d > 0)
      v[n++] = d;
    vTaskDelay(pdMS_TO_TICKS(60));
  }
  if (n < 3)
    return -1;
  qsort(v, n, sizeof(int), comparar_int);
  return v[n / 2];
}
#endif

#if CONFIG_ALIM_DHT11
static int esperar_nivel(int nivel, int limite_us)
{
  int64_t inicio = esp_timer_get_time();
  while (gpio_get_level(CONFIG_ALIM_GPIO_DHT) != nivel)
    if (esp_timer_get_time() - inicio > limite_us)
      return -1;
  return (int)(esp_timer_get_time() - inicio);
}

// Protocolo de um fio do DHT11: 40 bits; nível alto > 40 us = bit 1.
static bool ler_dht11(float *temp, float *umid)
{
  static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  uint8_t dados[5] = {0};
  int pino = CONFIG_ALIM_GPIO_DHT;

  gpio_set_direction(pino, GPIO_MODE_OUTPUT);
  gpio_set_level(pino, 0);
  vTaskDelay(pdMS_TO_TICKS(20));

  bool ok = true;
  portENTER_CRITICAL(&mux);
  gpio_set_level(pino, 1);
  esp_rom_delay_us(30);
  gpio_set_direction(pino, GPIO_MODE_INPUT);
  if (esperar_nivel(0, 100) < 0 || esperar_nivel(1, 100) < 0 || esperar_nivel(0, 100) < 0)
    ok = false;
  for (int i = 0; ok && i < 40; i++)
  {
    if (esperar_nivel(1, 70) < 0)
    {
      ok = false;
      break;
    }
    int alto = esperar_nivel(0, 100);
    if (alto < 0)
    {
      ok = false;
      break;
    }
    dados[i / 8] <<= 1;
    if (alto > 40)
      dados[i / 8] |= 1;
  }
  portEXIT_CRITICAL(&mux);

  if (!ok || ((dados[0] + dados[1] + dados[2] + dados[3]) & 0xFF) != dados[4])
    return false;
  *umid = dados[0] + dados[1] / 10.0f;
  *temp = dados[2] + dados[3] / 10.0f;
  return true;
}
#endif

#if CONFIG_ALIM_SENSOR_CHAMA
#define CHAMA_PASSO_MS 100
#define CHAMA_CONFIRMA 10 // 1 s seguido vendo chama antes de alarmar
#define CHAMA_LIBERA 50   // 5 s sem chama para limpar o alarme

// O D0 do módulo vai a 0 quando o fotodiodo IR vê chama. Fica numa tarefa própria
// porque a tarefa de sensores dorme 10 s entre as leituras.
static void tarefa_chama(void *arg)
{
  int vendo = 0, sem_ver = 0;
  bool alarmado = false;
  for (;;)
  {
    if (gpio_get_level(CONFIG_ALIM_GPIO_CHAMA) == 0)
    {
      sem_ver = 0;
      if (!alarmado && ++vendo >= CHAMA_CONFIRMA)
      {
        alarmado = true;
        ESP_LOGE(TAG, "Chama detectada!");
        estado_travar();
        g_estado.alarme = ALARME_CHAMA;
        estado_liberar();
        estado_mudou();
        nuvem_publicar_evento("{\"tipo\":\"chama\"}");
      }
    }
    else
    {
      vendo = 0;
      if (alarmado && ++sem_ver >= CHAMA_LIBERA)
      {
        alarmado = false;
        ESP_LOGW(TAG, "Chama não é mais detectada");
        estado_travar();
        if (g_estado.alarme == ALARME_CHAMA)
          g_estado.alarme = ALARME_NENHUM;
        estado_liberar();
        estado_mudou();
      }
    }
    if (alarmado)
    {
      interface_bipar(1, 150); // ~3 bipes por segundo enquanto houver chama
      continue;
    }
    vTaskDelay(pdMS_TO_TICKS(CHAMA_PASSO_MS));
  }
}
#endif

static void tarefa_sensores(void *arg)
{
  int ciclo = 0;
#if CONFIG_ALIM_SENSOR_NIVEL
  int ultimo_pct = -2;
  bool avisou_baixo = false;
#endif
  for (;;)
  {
    estado_travar();
    bool servindo = g_estado.servindo;
    estado_liberar();
    if (servindo) // o servo gera ruído; espera terminar
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
      continue;
    }

    bool mudou = false;
#if CONFIG_ALIM_SENSOR_NIVEL
    int mm = ler_nivel_mm();
    estado_travar();
    int vazio = g_estado.cfg.nivel_vazio_mm, cheio = g_estado.cfg.nivel_cheio_mm;
    int pct = -1;
    if (mm > 0 && vazio > cheio)
    {
      pct = (vazio - mm) * 100 / (vazio - cheio);
      pct = pct < 0 ? 0 : (pct > 100 ? 100 : pct);
    }
    g_estado.nivel_pct = pct;
    g_estado.nivel_cm = mm / 10.0f;
    bool baixo = pct >= 0 && pct < CONFIG_ALIM_NIVEL_BAIXO_PCT;
    if (baixo && g_estado.alarme == ALARME_NENHUM)
      g_estado.alarme = ALARME_RACAO_BAIXA;
    else if (!baixo && g_estado.alarme == ALARME_RACAO_BAIXA)
      g_estado.alarme = ALARME_NENHUM;
    estado_liberar();

    if (abs(pct - ultimo_pct) >= 2)
    {
      mudou = true;
      ultimo_pct = pct;
    }
    if (baixo && !avisou_baixo)
    {
      nuvem_publicar_evento("{\"tipo\":\"racao_baixa\"}");
      ESP_LOGW(TAG, "Ração baixa: %d%%", pct);
    }
    avisou_baixo = baixo;
#endif

#if CONFIG_ALIM_DHT11
    if (ciclo % 3 == 0) // o DHT11 aceita no máximo 1 leitura/s; 30 s basta
    {
      float t, u;
      bool ok = ler_dht11(&t, &u);
      estado_travar();
      g_estado.ambiente_ok = ok;
      if (ok)
      {
        g_estado.temp_c = t;
        g_estado.umid_pct = u;
      }
      estado_liberar();
    }
#endif

    if (mudou || ++ciclo % PUBLICAR_A_CADA == 0)
      estado_mudou();
    vTaskDelay(pdMS_TO_TICKS(PERIODO_MS));
  }
}

void sensores_iniciar(void)
{
#if CONFIG_ALIM_SENSOR_NIVEL
  gpio_reset_pin(CONFIG_ALIM_GPIO_TRIG);
  gpio_set_direction(CONFIG_ALIM_GPIO_TRIG, GPIO_MODE_OUTPUT);
  gpio_set_level(CONFIG_ALIM_GPIO_TRIG, 0);
  gpio_reset_pin(CONFIG_ALIM_GPIO_ECHO);
  gpio_set_direction(CONFIG_ALIM_GPIO_ECHO, GPIO_MODE_INPUT);
#endif
#if CONFIG_ALIM_DHT11
  gpio_reset_pin(CONFIG_ALIM_GPIO_DHT);
  gpio_set_pull_mode(CONFIG_ALIM_GPIO_DHT, GPIO_PULLUP_ONLY);
#endif
#if CONFIG_ALIM_SENSOR_CHAMA
  gpio_reset_pin(CONFIG_ALIM_GPIO_CHAMA);
  gpio_set_direction(CONFIG_ALIM_GPIO_CHAMA, GPIO_MODE_INPUT);
  gpio_set_pull_mode(CONFIG_ALIM_GPIO_CHAMA, GPIO_PULLUP_ONLY); // sem módulo = sem alarme
  xTaskCreate(tarefa_chama, "chama", 3072, NULL, 4, NULL);
#endif
  xTaskCreate(tarefa_sensores, "sensores", 3072, NULL, 3, NULL);
}
