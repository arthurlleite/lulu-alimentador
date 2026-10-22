#include "wifi_mgr.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_netif_sntp.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "mdns.h"
#include "cJSON.h"
#include "app.h"
#include "dns_server.h"
#include "mqtt_link.h"

static const char *TAG = "wifi";

static TimerHandle_t s_retry_timer;
static TimerHandle_t s_fallback_timer;
static int s_retry;
static int s_ap_clients;
static bool s_services_started;

static void set_net(net_status_t n)
{
  app_lock();
  g_app.net = n;
  app_unlock();
}

static void start_portal(void)
{
  app_lock();
  bool already = g_app.portal;
  bool has_wifi = g_app.has_wifi;
  app_unlock();
  if (already)
    return;

  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP);
  wifi_config_t ap = {
      .ap = {
          .channel = 1,
          .max_connection = 4,
          .authmode = WIFI_AUTH_OPEN,
      },
  };
  int n = snprintf((char *)ap.ap.ssid, sizeof(ap.ap.ssid), "%s-%02X%02X",
                   CONFIG_FEEDER_AP_PREFIX, mac[4], mac[5]);
  ap.ap.ssid_len = n;
  if (strlen(CONFIG_FEEDER_AP_PASSWORD) >= 8)
  {
    strlcpy((char *)ap.ap.password, CONFIG_FEEDER_AP_PASSWORD, sizeof(ap.ap.password));
    ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
  }

  // APSTA: o AP serve a página e o STA continua disponível para buscar redes.
  esp_wifi_set_mode(WIFI_MODE_APSTA);
  esp_wifi_set_config(WIFI_IF_AP, &ap);
  dns_server_start();

  app_lock();
  g_app.portal = true;
  if (!has_wifi)
    g_app.net = NET_SETUP;
  app_unlock();
  ESP_LOGW(TAG, "Rede de configuração ativa: \"%s\" -> http://192.168.4.1", ap.ap.ssid);
}

static void stop_portal(void)
{
  app_lock();
  bool active = g_app.portal;
  g_app.portal = false;
  app_unlock();
  if (!active)
    return;
  dns_server_stop();
  esp_wifi_set_mode(WIFI_MODE_STA);
  s_ap_clients = 0;
  ESP_LOGI(TAG, "Rede de configuração desligada");
}

static void on_time_sync(struct timeval *tv)
{
  ESP_LOGI(TAG, "Relógio sincronizado (NTP)");
  app_changed();
}

static void start_services(void)
{
  if (s_services_started)
    return;
  s_services_started = true;

  esp_sntp_config_t sntp = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(
      2, ESP_SNTP_SERVER_LIST("a.st1.ntp.br", "pool.ntp.org"));
  sntp.sync_cb = on_time_sync;
  esp_netif_sntp_init(&sntp);

  if (mdns_init() == ESP_OK)
  {
    mdns_hostname_set(CONFIG_FEEDER_HOSTNAME);
    app_lock();
    mdns_instance_name_set(g_app.cfg.name);
    app_unlock();
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
  }

  mqtt_link_start();
}

static void retry_cb(TimerHandle_t t)
{
  // Enquanto alguém usa a página de configuração, não troca de canal
  // tentando conectar: isso derrubaria o celular do AP.
  if (s_ap_clients > 0)
  {
    xTimerChangePeriod(s_retry_timer, pdMS_TO_TICKS(10000), 0);
    return;
  }
  esp_wifi_connect();
}

static void fallback_cb(TimerHandle_t t)
{
  app_lock();
  bool connected = g_app.net >= NET_WIFI;
  app_unlock();
  if (!connected)
    start_portal();
}

static void event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
  if (base == WIFI_EVENT)
  {
    switch (id)
    {
    case WIFI_EVENT_STA_START:
    {
      app_lock();
      bool has_wifi = g_app.has_wifi;
      app_unlock();
      if (has_wifi)
        esp_wifi_connect();
      break;
    }
    case WIFI_EVENT_STA_DISCONNECTED:
    {
      app_lock();
      bool retry = g_app.has_wifi;
      if (g_app.net != NET_SETUP)
        g_app.net = NET_CONNECTING;
      g_app.ip[0] = '\0';
      app_unlock();
      if (!retry)
        break;
      int delay_s = s_retry < 5 ? (1 << s_retry) : 30; // 1,2,4,8,16,30,30...
      s_retry++;
      ESP_LOGW(TAG, "Sem conexão, nova tentativa em %d s", delay_s);
      xTimerChangePeriod(s_retry_timer, pdMS_TO_TICKS(delay_s * 1000), 0);
      break;
    }
    case WIFI_EVENT_AP_STACONNECTED:
      s_ap_clients++;
      break;
    case WIFI_EVENT_AP_STADISCONNECTED:
      if (s_ap_clients > 0)
        s_ap_clients--;
      break;
    default:
      break;
    }
  }
  else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *ev = data;
    s_retry = 0;
    app_lock();
    snprintf(g_app.ip, sizeof(g_app.ip), IPSTR, IP2STR(&ev->ip_info.ip));
    if (g_app.net != NET_ONLINE)
      g_app.net = NET_WIFI;
    app_unlock();
    ESP_LOGI(TAG, "Conectado, IP %s", g_app.ip);
    xTimerStop(s_fallback_timer, 0);
    stop_portal();
    start_services();
    app_changed();
  }
}

void wifi_mgr_start(void)
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();
  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); // credenciais ficam na nossa NVS
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL));

  s_retry_timer = xTimerCreate("wifi_retry", pdMS_TO_TICKS(1000), pdFALSE, NULL, retry_cb);
  s_fallback_timer = xTimerCreate("wifi_fallback", pdMS_TO_TICKS(CONFIG_FEEDER_PORTAL_FALLBACK_S * 1000),
                                  pdFALSE, NULL, fallback_cb);

  char ssid[33] = "", pass[65] = "";
  bool has_wifi = app_get_wifi(ssid, sizeof(ssid), pass, sizeof(pass));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  if (has_wifi)
  {
    wifi_config_t sta = {0};
    strlcpy((char *)sta.sta.ssid, ssid, sizeof(sta.sta.ssid));
    strlcpy((char *)sta.sta.password, pass, sizeof(sta.sta.password));
    // Aceita redes WPA2 e WPA3; sem senha, aceita rede aberta.
    sta.sta.threshold.authmode = pass[0] ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    sta.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta));
    set_net(NET_CONNECTING);
    // O original logava a senha na serial; aqui só o nome da rede.
    ESP_LOGI(TAG, "Conectando em \"%s\"", ssid);
    xTimerStart(s_fallback_timer, 0);
  }
  ESP_ERROR_CHECK(esp_wifi_start());
  if (!has_wifi)
    start_portal();
}

char *wifi_mgr_scan_json(void)
{
  cJSON *arr = cJSON_CreateArray();
  wifi_scan_config_t sc = {.show_hidden = false};
  if (esp_wifi_scan_start(&sc, true) == ESP_OK)
  {
    uint16_t n = 20;
    wifi_ap_record_t *recs = calloc(n, sizeof(wifi_ap_record_t));
    if (recs && esp_wifi_scan_get_ap_records(&n, recs) == ESP_OK)
    {
      for (int i = 0; i < n; i++)
      {
        const char *name = (const char *)recs[i].ssid;
        bool dup = name[0] == '\0';
        for (int j = 0; j < i && !dup; j++)
          dup = strcmp(name, (const char *)recs[j].ssid) == 0;
        if (dup)
          continue; // lista vem ordenada por sinal: fica o mais forte
        cJSON *o = cJSON_CreateObject();
        cJSON_AddStringToObject(o, "ssid", name);
        cJSON_AddNumberToObject(o, "rssi", recs[i].rssi);
        cJSON_AddBoolToObject(o, "lock", recs[i].authmode != WIFI_AUTH_OPEN);
        cJSON_AddItemToArray(arr, o);
      }
    }
    free(recs);
  }
  else
  {
    ESP_LOGW(TAG, "Busca de redes falhou (rádio ocupado)");
  }
  char *out = cJSON_PrintUnformatted(arr);
  cJSON_Delete(arr);
  return out;
}

static void restart_cb(void *arg)
{
  esp_restart();
}

static void restart_soon(void)
{
  static esp_timer_handle_t t;
  if (!t)
  {
    esp_timer_create_args_t args = {.callback = restart_cb, .name = "restart"};
    esp_timer_create(&args, &t);
  }
  esp_timer_start_once(t, 2500 * 1000); // dá tempo da resposta HTTP chegar
}

void wifi_mgr_provision(const char *ssid, const char *pass)
{
  ESP_LOGI(TAG, "Nova rede salva: \"%s\". Reiniciando...", ssid);
  app_set_wifi(ssid, pass);
  restart_soon();
}

void wifi_mgr_factory_reset(void)
{
  app_erase_wifi();
  restart_soon();
}
