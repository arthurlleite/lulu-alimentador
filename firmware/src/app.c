#include "app.h"

#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_random.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "cJSON.h"
#include "mqtt_link.h"

#define NVS_NS "feeder"
#define SETTINGS_VERSION 2

static const char *TAG = "app";
static SemaphoreHandle_t s_lock;

app_state_t g_app;

static void default_settings(settings_t *s)
{
  memset(s, 0, sizeof(*s));
  strcpy(s->name, "Pet");
  s->beep = true;
  s->level_empty_mm = 300;
  s->level_full_mm = 40;
  s->meal_count = 2;
  s->meals[0] = (meal_t){.hour = 7, .minute = 0, .portions = 1, .enabled = true};
  s->meals[1] = (meal_t){.hour = 19, .minute = 0, .portions = 1, .enabled = true};
}

static void new_code(char *out)
{
  // 16 caracteres hex = 64 bits aleatórios do gerador de hardware.
  uint32_t a = esp_random(), b = esp_random();
  snprintf(out, 17, "%08lx%08lx", (unsigned long)a, (unsigned long)b);
}

static void load_settings(void)
{
  settings_t *s = &g_app.cfg;
  nvs_handle_t h;
  bool loaded = false;
  if (nvs_open(NVS_NS, NVS_READONLY, &h) == ESP_OK)
  {
    uint8_t ver = 0;
    size_t len = sizeof(*s);
    if (nvs_get_u8(h, "cfg_v", &ver) == ESP_OK && ver == SETTINGS_VERSION &&
        nvs_get_blob(h, "cfg", s, &len) == ESP_OK && len == sizeof(*s))
    {
      loaded = true;
    }
    nvs_close(h);
  }
  if (!loaded)
  {
    ESP_LOGI(TAG, "Sem configuração salva, usando padrões");
    default_settings(s);
  }
  if (strlen(s->code) != 16)
  {
    new_code(s->code);
    loaded = false;
  }
  s->name[sizeof(s->name) - 1] = '\0';
  if (s->meal_count > MAX_MEALS)
    s->meal_count = 0;
  if (!loaded)
    app_save_settings();
}

void app_init(void)
{
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  s_lock = xSemaphoreCreateMutex();
  memset(&g_app, 0, sizeof(g_app));
  g_app.level_pct = -1;
  load_settings();

  char ssid[33], pass[65];
  g_app.has_wifi = app_get_wifi(ssid, sizeof(ssid), pass, sizeof(pass));
  if (g_app.has_wifi)
    strlcpy(g_app.ssid, ssid, sizeof(g_app.ssid));

  setenv("TZ", CONFIG_FEEDER_TZ, 1);
  tzset();
  ESP_LOGI(TAG, "Pet: %s | tópico: %s/%s", g_app.cfg.name, CONFIG_FEEDER_MQTT_PREFIX, g_app.cfg.code);
}

void app_lock(void) { xSemaphoreTake(s_lock, portMAX_DELAY); }
void app_unlock(void) { xSemaphoreGive(s_lock); }

bool app_time_valid(void)
{
  time_t now = time(NULL);
  struct tm t;
  localtime_r(&now, &t);
  return t.tm_year >= (2025 - 1900);
}

const char *app_source_name(feed_source_t src)
{
  switch (src)
  {
  case SRC_SCHEDULE:
    return "agenda";
  case SRC_BUTTON:
    return "botao";
  default:
    return "app";
  }
}

void app_save_settings(void)
{
  nvs_handle_t h;
  if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK)
    return;
  nvs_set_u8(h, "cfg_v", SETTINGS_VERSION);
  nvs_set_blob(h, "cfg", &g_app.cfg, sizeof(g_app.cfg));
  nvs_commit(h);
  nvs_close(h);
}

bool app_get_wifi(char *ssid, size_t ssid_len, char *pass, size_t pass_len)
{
  nvs_handle_t h;
  if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK)
    return false;
  bool ok = nvs_get_str(h, "ssid", ssid, &ssid_len) == ESP_OK &&
            nvs_get_str(h, "pass", pass, &pass_len) == ESP_OK && ssid[0] != '\0';
  nvs_close(h);
  return ok;
}

void app_set_wifi(const char *ssid, const char *pass)
{
  nvs_handle_t h;
  ESP_ERROR_CHECK(nvs_open(NVS_NS, NVS_READWRITE, &h));
  nvs_set_str(h, "ssid", ssid);
  nvs_set_str(h, "pass", pass);
  nvs_commit(h);
  nvs_close(h);
}

void app_erase_wifi(void)
{
  nvs_handle_t h;
  if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK)
    return;
  nvs_erase_key(h, "ssid");
  nvs_erase_key(h, "pass");
  nvs_commit(h);
  nvs_close(h);
}

void app_record_feed(uint8_t portions, feed_source_t src, bool ok)
{
  app_lock();
  memmove(&g_app.history[1], &g_app.history[0], sizeof(feed_log_t) * (HISTORY_LEN - 1));
  g_app.history[0] = (feed_log_t){
      .ts = app_time_valid() ? time(NULL) : 0,
      .portions = portions,
      .src = src,
      .ok = ok,
  };
  if (g_app.history_len < HISTORY_LEN)
    g_app.history_len++;
  g_app.feeding = false;
  if (!ok)
    g_app.alarm = ALARM_NO_OUTPUT;
  else if (g_app.alarm == ALARM_NO_OUTPUT)
    g_app.alarm = ALARM_NONE;
  app_unlock();
}

static const char *net_name(net_status_t n)
{
  switch (n)
  {
  case NET_SETUP:
    return "configuracao";
  case NET_CONNECTING:
    return "conectando";
  case NET_WIFI:
    return "wifi";
  default:
    return "online";
  }
}

static const char *alarm_name(alarm_t a)
{
  switch (a)
  {
  case ALARM_LOW_FOOD:
    return "racao_baixa";
  case ALARM_NO_OUTPUT:
    return "sem_saida";
  default:
    return NULL;
  }
}

static cJSON *meal_json(const meal_t *m)
{
  cJSON *o = cJSON_CreateObject();
  cJSON_AddNumberToObject(o, "h", m->hour);
  cJSON_AddNumberToObject(o, "m", m->minute);
  cJSON_AddNumberToObject(o, "p", m->portions);
  cJSON_AddBoolToObject(o, "on", m->enabled);
  return o;
}

char *app_state_json(bool include_secret)
{
  cJSON *root = cJSON_CreateObject();
  bool time_ok = app_time_valid();
  time_t now = time(NULL);

  app_lock();
  const app_state_t *a = &g_app;
  cJSON_AddStringToObject(root, "name", a->cfg.name);
  cJSON_AddStringToObject(root, "fw", FW_VERSION);
  cJSON_AddStringToObject(root, "net", net_name(a->net));
  cJSON_AddBoolToObject(root, "time_ok", time_ok);
  if (time_ok)
    cJSON_AddNumberToObject(root, "time", (double)now);
  cJSON_AddBoolToObject(root, "feeding", a->feeding);
  cJSON_AddBoolToObject(root, "beep", a->cfg.beep);
  const char *alarm = alarm_name(a->alarm);
  if (alarm)
    cJSON_AddStringToObject(root, "alarm", alarm);
  else
    cJSON_AddNullToObject(root, "alarm");

  cJSON *wifi = cJSON_AddObjectToObject(root, "wifi");
  cJSON_AddStringToObject(wifi, "ssid", a->ssid);
  cJSON_AddStringToObject(wifi, "ip", a->ip);
  cJSON_AddBoolToObject(wifi, "portal", a->portal);

  if (a->level_pct >= 0)
  {
    cJSON *lvl = cJSON_AddObjectToObject(root, "level");
    cJSON_AddNumberToObject(lvl, "pct", a->level_pct);
    cJSON_AddNumberToObject(lvl, "cm", (int)(a->level_cm * 10) / 10.0);
  }
  else
    cJSON_AddNullToObject(root, "level");
  cJSON *cal = cJSON_AddObjectToObject(root, "level_cal");
  cJSON_AddNumberToObject(cal, "empty_cm", a->cfg.level_empty_mm / 10.0);
  cJSON_AddNumberToObject(cal, "full_cm", a->cfg.level_full_mm / 10.0);

  if (a->env_ok)
  {
    cJSON *env = cJSON_AddObjectToObject(root, "env");
    cJSON_AddNumberToObject(env, "t", a->temp_c);
    cJSON_AddNumberToObject(env, "h", a->hum_pct);
  }
  else
    cJSON_AddNullToObject(root, "env");

  cJSON *meals = cJSON_AddArrayToObject(root, "meals");
  int best = -1, best_in = 24 * 60 + 1;
  struct tm t;
  localtime_r(&now, &t);
  int now_min = t.tm_hour * 60 + t.tm_min;
  for (int i = 0; i < a->cfg.meal_count; i++)
  {
    const meal_t *m = &a->cfg.meals[i];
    cJSON_AddItemToArray(meals, meal_json(m));
    if (!m->enabled)
      continue;
    int in = (m->hour * 60 + m->minute - now_min + 24 * 60) % (24 * 60);
    if (in == 0)
      in = 24 * 60;
    if (in < best_in)
    {
      best_in = in;
      best = i;
    }
  }
  if (best >= 0 && time_ok)
  {
    cJSON *next = meal_json(&a->cfg.meals[best]);
    cJSON_AddNumberToObject(next, "in_min", best_in);
    cJSON_AddItemToObject(root, "next", next);
  }
  else
    cJSON_AddNullToObject(root, "next");

  cJSON *hist = cJSON_AddArrayToObject(root, "history");
  for (int i = 0; i < a->history_len; i++)
  {
    const feed_log_t *f = &a->history[i];
    cJSON *o = cJSON_CreateObject();
    cJSON_AddNumberToObject(o, "ts", (double)f->ts);
    cJSON_AddNumberToObject(o, "p", f->portions);
    cJSON_AddStringToObject(o, "src", app_source_name(f->src));
    cJSON_AddBoolToObject(o, "ok", f->ok);
    cJSON_AddItemToArray(hist, o);
  }

  if (include_secret)
  {
    char link[512];
    snprintf(link, sizeof(link), "%s?d=%s", CONFIG_FEEDER_APP_URL, a->cfg.code);
    cJSON_AddStringToObject(root, "code", a->cfg.code);
    cJSON_AddStringToObject(root, "app_url", link);
    cJSON_AddStringToObject(root, "host", CONFIG_FEEDER_HOSTNAME ".local");
  }
  app_unlock();

  char *out = cJSON_PrintUnformatted(root);
  cJSON_Delete(root);
  return out;
}

void app_changed(void)
{
  mqtt_link_publish_state();
}
