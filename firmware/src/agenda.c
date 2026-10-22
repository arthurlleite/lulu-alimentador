#include "agenda.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "estado.h"
#include "dosador.h"

// O original (my_timer.c) calculava quanto dormir até o próximo horário; com a
// agenda vazia o tempo ficava negativo. Aqui a checagem é por minuto: cada
// horário dispara uma vez por dia e um ajuste de relógio (NTP) não pula refeição.

static const char *TAG = "agenda";

static void tarefa_agenda(void *arg)
{
  int ultimo_disparo[MAX_REFEICOES];
  for (int i = 0; i < MAX_REFEICOES; i++)
    ultimo_disparo[i] = -1;

  for (;;)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    if (!estado_hora_valida())
      continue;

    time_t agora = time(NULL);
    struct tm t;
    localtime_r(&agora, &t);
    int chave = t.tm_yday * 1440 + t.tm_hour * 60 + t.tm_min;

    refeicao_t refeicoes[MAX_REFEICOES];
    estado_travar();
    int qtd = g_estado.cfg.qtd_refeicoes;
    memcpy(refeicoes, g_estado.cfg.refeicoes, sizeof(refeicoes));
    estado_liberar();

    for (int i = 0; i < qtd; i++)
    {
      const refeicao_t *r = &refeicoes[i];
      if (!r->ativa || r->hora != t.tm_hour || r->minuto != t.tm_min)
        continue;
      if (ultimo_disparo[i] == chave)
        continue;
      ultimo_disparo[i] = chave;
      ESP_LOGI(TAG, "Refeição das %02d:%02d", r->hora, r->minuto);
      dosador_pedir(r->porcoes, ORIGEM_AGENDA);
    }
  }
}

void agenda_iniciar(void)
{
  xTaskCreate(tarefa_agenda, "agenda", 3072, NULL, 5, NULL);
}
