#include "comandos.h"

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "esp_log.h"
#include "estado.h"
#include "dosador.h"
#include "rede_wifi.h"

// Substitui o my_dispatcher.c original, que não checava JSON inválido (travava
// com cJSON_GetObjectItem nulo) e vazava o buffer da mensagem.

static const char *TAG = "comandos";

static int ler_int(const cJSON *obj, const char *chave, int padrao)
{
  const cJSON *it = cJSON_GetObjectItemCaseSensitive(obj, chave);
  if (cJSON_IsNumber(it))
    return (int)it->valuedouble;
  if (cJSON_IsString(it))
    return atoi(it->valuestring);
  return padrao;
}

static int comparar_refeicao(const void *a, const void *b)
{
  const refeicao_t *x = a, *y = b;
  return (x->hora * 60 + x->minuto) - (y->hora * 60 + y->minuto);
}

static const char *cmd_agenda(const cJSON *in, bool *ok)
{
  const cJSON *lista = cJSON_GetObjectItemCaseSensitive(in, "refeicoes");
  if (!cJSON_IsArray(lista))
    return "Lista de horários ausente";
  int n = cJSON_GetArraySize(lista);
  if (n > MAX_REFEICOES)
    return "Máximo de 8 horários";

  refeicao_t refeicoes[MAX_REFEICOES] = {0};
  for (int i = 0; i < n; i++)
  {
    const cJSON *r = cJSON_GetArrayItem(lista, i);
    int h = ler_int(r, "h", -1), m = ler_int(r, "m", -1), p = ler_int(r, "p", 1);
    if (h < 0 || h > 23 || m < 0 || m > 59 || p < 1 || p > MAX_PORCOES)
      return "Horário ou porção inválidos";
    const cJSON *ativa = cJSON_GetObjectItemCaseSensitive(r, "ativa");
    refeicoes[i] = (refeicao_t){.hora = h, .minuto = m, .porcoes = p, .ativa = !cJSON_IsFalse(ativa)};
  }
  qsort(refeicoes, n, sizeof(refeicao_t), comparar_refeicao);

  estado_travar();
  memcpy(g_estado.cfg.refeicoes, refeicoes, sizeof(refeicoes));
  g_estado.cfg.qtd_refeicoes = n;
  estado_salvar_config();
  estado_liberar();
  *ok = true;
  return "Agenda salva";
}

static const char *cmd_ajustes(const cJSON *in, bool *ok)
{
  const cJSON *nome = cJSON_GetObjectItemCaseSensitive(in, "nome");
  const cJSON *bip = cJSON_GetObjectItemCaseSensitive(in, "bip");
  int vazio = ler_int(in, "nivel_vazio_cm", -1), cheio = ler_int(in, "nivel_cheio_cm", -1);

  if (cJSON_IsString(nome) && (strlen(nome->valuestring) < 1 || strlen(nome->valuestring) > 31))
    return "O nome deve ter de 1 a 31 caracteres";

  estado_travar();
  int novo_vazio = vazio > 0 ? vazio * 10 : g_estado.cfg.nivel_vazio_mm;
  int novo_cheio = cheio >= 0 ? cheio * 10 : g_estado.cfg.nivel_cheio_mm;
  if (novo_vazio - novo_cheio < 30)
  {
    estado_liberar();
    return "Calibração: vazio precisa ser ao menos 3 cm maior que cheio";
  }
  if (cJSON_IsString(nome))
    strlcpy(g_estado.cfg.nome, nome->valuestring, sizeof(g_estado.cfg.nome));
  if (cJSON_IsBool(bip))
    g_estado.cfg.bip = cJSON_IsTrue(bip);
  g_estado.cfg.nivel_vazio_mm = novo_vazio;
  g_estado.cfg.nivel_cheio_mm = novo_cheio;
  estado_salvar_config();
  estado_liberar();
  *ok = true;
  return "Ajustes salvos";
}

cJSON *comandos_executar(const char *dados, size_t tam, bool local)
{
  cJSON *resp = cJSON_CreateObject();
  cJSON *in = cJSON_ParseWithLength(dados, tam);
  const char *cmd = in ? cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(in, "comando")) : NULL;
  const char *msg = "Comando inválido";
  bool ok = false;

  if (!cmd)
  {
    // mensagem padrão
  }
  else if (strcmp(cmd, "alimentar") == 0)
  {
    int p = ler_int(in, "porcoes", 1);
    ok = dosador_pedir(p < 1 ? 1 : p, ORIGEM_APP);
    msg = ok ? "Servindo ração" : "Alimentador ocupado, tente de novo";
  }
  else if (strcmp(cmd, "agenda") == 0)
    msg = cmd_agenda(in, &ok);
  else if (strcmp(cmd, "ajustes") == 0)
    msg = cmd_ajustes(in, &ok);
  else if (strcmp(cmd, "status") == 0)
  {
    ok = true;
    msg = "ok";
  }
  else if (strcmp(cmd, "limpar_alarme") == 0)
  {
    estado_travar();
    g_estado.alarme = ALARME_NENHUM;
    estado_liberar();
    ok = true;
    msg = "Alarme limpo";
  }
  else if (strcmp(cmd, "acertar_hora") == 0 && local)
  {
    double instante = cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(in, "unix"));
    if (!estado_hora_valida() && instante > 1700000000)
    {
      struct timeval tv = {.tv_sec = (time_t)instante};
      settimeofday(&tv, NULL);
      ESP_LOGI(TAG, "Relógio acertado pelo navegador");
    }
    ok = true;
    msg = "Relógio ok";
  }
  else if (strcmp(cmd, "esquecer_wifi") == 0)
  {
    if (local)
    {
      rede_wifi_esquecer();
      ok = true;
      msg = "Wi-Fi apagado, reiniciando em modo de configuração";
    }
    else
      msg = "Por segurança, só pela rede local";
  }

  cJSON_AddBoolToObject(resp, "ok", ok);
  cJSON_AddStringToObject(resp, "mensagem", msg);
  if (cmd)
    cJSON_AddStringToObject(resp, "comando", cmd);
  const cJSON *id = in ? cJSON_GetObjectItemCaseSensitive(in, "id") : NULL;
  if (id)
    cJSON_AddItemToObject(resp, "id", cJSON_Duplicate(id, true));
  cJSON_Delete(in);
  return resp;
}
