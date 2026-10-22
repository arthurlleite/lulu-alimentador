#include "commands.h"

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "esp_log.h"
#include "app.h"
#include "feeder.h"
#include "wifi_mgr.h"

// Substitui o my_dispatcher.c original, que não checava JSON inválido (travava
// com cJSON_GetObjectItem nulo) e vazava o buffer da mensagem.

static const char *TAG = "cmd";

static int get_int(const cJSON *obj, const char *key, int def)
{
  const cJSON *it = cJSON_GetObjectItemCaseSensitive(obj, key);
  if (cJSON_IsNumber(it))
    return (int)it->valuedouble;
  if (cJSON_IsString(it))
    return atoi(it->valuestring);
  return def;
}

static int cmp_meal(const void *a, const void *b)
{
  const meal_t *x = a, *y = b;
  return (x->hour * 60 + x->minute) - (y->hour * 60 + y->minute);
}

static const char *cmd_schedule(const cJSON *in, bool *ok)
{
  const cJSON *arr = cJSON_GetObjectItemCaseSensitive(in, "meals");
  if (!cJSON_IsArray(arr))
    return "Lista de horários ausente";
  int n = cJSON_GetArraySize(arr);
  if (n > MAX_MEALS)
    return "Máximo de 8 horários";

  meal_t meals[MAX_MEALS] = {0};
  for (int i = 0; i < n; i++)
  {
    const cJSON *m = cJSON_GetArrayItem(arr, i);
    int h = get_int(m, "h", -1), min = get_int(m, "m", -1), p = get_int(m, "p", 1);
    if (h < 0 || h > 23 || min < 0 || min > 59 || p < 1 || p > MAX_PORTIONS)
      return "Horário ou porção inválidos";
    const cJSON *on = cJSON_GetObjectItemCaseSensitive(m, "on");
    meals[i] = (meal_t){.hour = h, .minute = min, .portions = p, .enabled = !cJSON_IsFalse(on)};
  }
  qsort(meals, n, sizeof(meal_t), cmp_meal);

  app_lock();
  memcpy(g_app.cfg.meals, meals, sizeof(meals));
  g_app.cfg.meal_count = n;
  app_save_settings();
  app_unlock();
  *ok = true;
  return "Agenda salva";
}

static const char *cmd_settings(const cJSON *in, bool *ok)
{
  const cJSON *name = cJSON_GetObjectItemCaseSensitive(in, "name");
  const cJSON *beep = cJSON_GetObjectItemCaseSensitive(in, "beep");
  int empty = get_int(in, "level_empty_cm", -1), full = get_int(in, "level_full_cm", -1);

  if (cJSON_IsString(name) && (strlen(name->valuestring) < 1 || strlen(name->valuestring) > 31))
    return "Nome deve ter de 1 a 31 caracteres";

  app_lock();
  int new_empty = empty > 0 ? empty * 10 : g_app.cfg.level_empty_mm;
  int new_full = full >= 0 ? full * 10 : g_app.cfg.level_full_mm;
  if (new_empty - new_full < 30)
  {
    app_unlock();
    return "Calibração: vazio precisa ser ao menos 3 cm maior que cheio";
  }
  if (cJSON_IsString(name))
    strlcpy(g_app.cfg.name, name->valuestring, sizeof(g_app.cfg.name));
  if (cJSON_IsBool(beep))
    g_app.cfg.beep = cJSON_IsTrue(beep);
  g_app.cfg.level_empty_mm = new_empty;
  g_app.cfg.level_full_mm = new_full;
  app_save_settings();
  app_unlock();
  *ok = true;
  return "Ajustes salvos";
}

cJSON *commands_handle(const char *data, size_t len, bool local)
{
  cJSON *resp = cJSON_CreateObject();
  cJSON *in = cJSON_ParseWithLength(data, len);
  const char *cmd = in ? cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(in, "cmd")) : NULL;
  const char *msg = "Comando inválido";
  bool ok = false;

  if (!cmd)
  {
    // msg padrão
  }
  else if (strcmp(cmd, "feed") == 0)
  {
    int p = get_int(in, "portions", 1);
    ok = feeder_request(p < 1 ? 1 : p, SRC_APP);
    msg = ok ? "Servindo ração" : "Alimentador ocupado, tente de novo";
  }
  else if (strcmp(cmd, "schedule") == 0)
    msg = cmd_schedule(in, &ok);
  else if (strcmp(cmd, "settings") == 0)
    msg = cmd_settings(in, &ok);
  else if (strcmp(cmd, "status") == 0)
  {
    ok = true;
    msg = "ok";
  }
  else if (strcmp(cmd, "clear_alarm") == 0)
  {
    app_lock();
    g_app.alarm = ALARM_NONE;
    app_unlock();
    ok = true;
    msg = "Alarme limpo";
  }
  else if (strcmp(cmd, "time") == 0 && local)
  {
    double epoch = cJSON_GetNumberValue(cJSON_GetObjectItemCaseSensitive(in, "epoch"));
    if (!app_time_valid() && epoch > 1700000000)
    {
      struct timeval tv = {.tv_sec = (time_t)epoch};
      settimeofday(&tv, NULL);
      ESP_LOGI(TAG, "Relógio acertado pelo navegador");
    }
    ok = true;
    msg = "Relógio ok";
  }
  else if (strcmp(cmd, "wifi_reset") == 0)
  {
    if (local)
    {
      wifi_mgr_factory_reset();
      ok = true;
      msg = "Wi-Fi apagado, reiniciando em modo de configuração";
    }
    else
      msg = "Por segurança, só pela rede local";
  }

  cJSON_AddBoolToObject(resp, "ok", ok);
  cJSON_AddStringToObject(resp, "msg", msg);
  if (cmd)
    cJSON_AddStringToObject(resp, "cmd", cmd);
  const cJSON *id = in ? cJSON_GetObjectItemCaseSensitive(in, "id") : NULL;
  if (id)
    cJSON_AddItemToObject(resp, "id", cJSON_Duplicate(id, true));
  cJSON_Delete(in);
  return resp;
}
