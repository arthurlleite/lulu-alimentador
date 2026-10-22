#include "rede_wifi.h"

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
#include "estado.h"
#include "dns_portal.h"
#include "nuvem_mqtt.h"

static const char *TAG = "wifi";

static TimerHandle_t s_timer_tentativa;
static TimerHandle_t s_timer_portal;
static int s_tentativas;
static int s_clientes_ap;
static bool s_servicos_iniciados;

static void mudar_rede(status_rede_t r)
{
  estado_travar();
  g_estado.rede = r;
  estado_liberar();
}

static void abrir_portal(void)
{
  estado_travar();
  bool ja_aberto = g_estado.portal;
  bool tem_wifi = g_estado.tem_wifi;
  estado_liberar();
  if (ja_aberto)
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
                   CONFIG_ALIM_PREFIXO_AP, mac[4], mac[5]);
  ap.ap.ssid_len = n;
  if (strlen(CONFIG_ALIM_SENHA_AP) >= 8)
  {
    strlcpy((char *)ap.ap.password, CONFIG_ALIM_SENHA_AP, sizeof(ap.ap.password));
    ap.ap.authmode = WIFI_AUTH_WPA2_PSK;
  }

  // APSTA: o AP serve a página e o STA continua disponível para buscar redes.
  esp_wifi_set_mode(WIFI_MODE_APSTA);
  esp_wifi_set_config(WIFI_IF_AP, &ap);
  dns_portal_iniciar();

  estado_travar();
  g_estado.portal = true;
  if (!tem_wifi)
    g_estado.rede = REDE_CONFIGURACAO;
  estado_liberar();
  ESP_LOGW(TAG, "Rede de configuração ativa: \"%s\" -> http://192.168.4.1", ap.ap.ssid);
}

static void fechar_portal(void)
{
  estado_travar();
  bool aberto = g_estado.portal;
  g_estado.portal = false;
  estado_liberar();
  if (!aberto)
    return;
  dns_portal_parar();
  esp_wifi_set_mode(WIFI_MODE_STA);
  s_clientes_ap = 0;
  ESP_LOGI(TAG, "Rede de configuração desligada");
}

static void ao_acertar_hora(struct timeval *tv)
{
  ESP_LOGI(TAG, "Relógio acertado (NTP)");
  estado_mudou();
}

static void iniciar_servicos(void)
{
  if (s_servicos_iniciados)
    return;
  s_servicos_iniciados = true;

  esp_sntp_config_t sntp = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(
      2, ESP_SNTP_SERVER_LIST("a.st1.ntp.br", "pool.ntp.org"));
  sntp.sync_cb = ao_acertar_hora;
  esp_netif_sntp_init(&sntp);

  if (mdns_init() == ESP_OK)
  {
    mdns_hostname_set(CONFIG_ALIM_NOME_MDNS);
    estado_travar();
    mdns_instance_name_set(g_estado.cfg.nome);
    estado_liberar();
    mdns_service_add(NULL, "_http", "_tcp", 80, NULL, 0);
  }

  nuvem_iniciar();
}

static void ao_timer_tentativa(TimerHandle_t t)
{
  // Enquanto alguém usa a página de configuração, não troca de canal
  // tentando conectar: isso derrubaria o celular do AP.
  if (s_clientes_ap > 0)
  {
    xTimerChangePeriod(s_timer_tentativa, pdMS_TO_TICKS(10000), 0);
    return;
  }
  esp_wifi_connect();
}

static void ao_timer_portal(TimerHandle_t t)
{
  estado_travar();
  bool conectado = g_estado.rede >= REDE_WIFI;
  estado_liberar();
  if (!conectado)
    abrir_portal();
}

static void ao_evento(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
  if (base == WIFI_EVENT)
  {
    switch (id)
    {
    case WIFI_EVENT_STA_START:
    {
      estado_travar();
      bool tem_wifi = g_estado.tem_wifi;
      estado_liberar();
      if (tem_wifi)
        esp_wifi_connect();
      break;
    }
    case WIFI_EVENT_STA_DISCONNECTED:
    {
      estado_travar();
      bool tentar = g_estado.tem_wifi;
      if (g_estado.rede != REDE_CONFIGURACAO)
        g_estado.rede = REDE_CONECTANDO;
      g_estado.ip[0] = '\0';
      estado_liberar();
      if (!tentar)
        break;
      int espera_s = s_tentativas < 5 ? (1 << s_tentativas) : 30; // 1,2,4,8,16,30,30...
      s_tentativas++;
      ESP_LOGW(TAG, "Sem conexão, nova tentativa em %d s", espera_s);
      xTimerChangePeriod(s_timer_tentativa, pdMS_TO_TICKS(espera_s * 1000), 0);
      break;
    }
    case WIFI_EVENT_AP_STACONNECTED:
      s_clientes_ap++;
      break;
    case WIFI_EVENT_AP_STADISCONNECTED:
      if (s_clientes_ap > 0)
        s_clientes_ap--;
      break;
    default:
      break;
    }
  }
  else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t *ev = dados;
    s_tentativas = 0;
    estado_travar();
    snprintf(g_estado.ip, sizeof(g_estado.ip), IPSTR, IP2STR(&ev->ip_info.ip));
    if (g_estado.rede != REDE_ONLINE)
      g_estado.rede = REDE_WIFI;
    estado_liberar();
    ESP_LOGI(TAG, "Conectado, IP " IPSTR, IP2STR(&ev->ip_info.ip));
    xTimerStop(s_timer_portal, 0);
    fechar_portal();
    iniciar_servicos();
    estado_mudou();
  }
}

void rede_wifi_iniciar(void)
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();
  esp_netif_create_default_wifi_ap();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM)); // a senha fica na nossa NVS
  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, ao_evento, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, ao_evento, NULL));

  s_timer_tentativa = xTimerCreate("wifi_tentar", pdMS_TO_TICKS(1000), pdFALSE, NULL, ao_timer_tentativa);
  s_timer_portal = xTimerCreate("wifi_portal", pdMS_TO_TICKS(CONFIG_ALIM_PORTAL_APOS_S * 1000),
                                pdFALSE, NULL, ao_timer_portal);

  char ssid[33] = "", senha[65] = "";
  bool tem_wifi = estado_ler_wifi(ssid, sizeof(ssid), senha, sizeof(senha));

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  if (tem_wifi)
  {
    wifi_config_t sta = {0};
    strlcpy((char *)sta.sta.ssid, ssid, sizeof(sta.sta.ssid));
    strlcpy((char *)sta.sta.password, senha, sizeof(sta.sta.password));
    // Aceita redes WPA2 e WPA3; sem senha, aceita rede aberta.
    sta.sta.threshold.authmode = senha[0] ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
    sta.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta));
    mudar_rede(REDE_CONECTANDO);
    // O original imprimia a senha na serial; aqui só o nome da rede.
    ESP_LOGI(TAG, "Conectando em \"%s\"", ssid);
    xTimerStart(s_timer_portal, 0);
  }
  ESP_ERROR_CHECK(esp_wifi_start());
  if (!tem_wifi)
    abrir_portal();
}

char *rede_wifi_buscar_json(void)
{
  cJSON *lista = cJSON_CreateArray();
  wifi_scan_config_t busca = {.show_hidden = false};
  if (esp_wifi_scan_start(&busca, true) == ESP_OK)
  {
    uint16_t n = 20;
    wifi_ap_record_t *redes = calloc(n, sizeof(wifi_ap_record_t));
    if (redes && esp_wifi_scan_get_ap_records(&n, redes) == ESP_OK)
    {
      for (int i = 0; i < n; i++)
      {
        const char *nome = (const char *)redes[i].ssid;
        bool repetida = nome[0] == '\0';
        for (int j = 0; j < i && !repetida; j++)
          repetida = strcmp(nome, (const char *)redes[j].ssid) == 0;
        if (repetida)
          continue; // a lista vem ordenada por sinal: fica a mais forte
        cJSON *o = cJSON_CreateObject();
        cJSON_AddStringToObject(o, "ssid", nome);
        cJSON_AddNumberToObject(o, "sinal", redes[i].rssi);
        cJSON_AddBoolToObject(o, "senha", redes[i].authmode != WIFI_AUTH_OPEN);
        cJSON_AddItemToArray(lista, o);
      }
    }
    free(redes);
  }
  else
  {
    ESP_LOGW(TAG, "Busca de redes falhou (rádio ocupado)");
  }
  char *saida = cJSON_PrintUnformatted(lista);
  cJSON_Delete(lista);
  return saida;
}

static void ao_reiniciar(void *arg)
{
  esp_restart();
}

static void reiniciar_em_breve(void)
{
  static esp_timer_handle_t t;
  if (!t)
  {
    esp_timer_create_args_t args = {.callback = ao_reiniciar, .name = "reiniciar"};
    esp_timer_create(&args, &t);
  }
  esp_timer_start_once(t, 2500 * 1000); // dá tempo da resposta HTTP chegar
}

void rede_wifi_configurar(const char *ssid, const char *senha)
{
  ESP_LOGI(TAG, "Nova rede salva: \"%s\". Reiniciando...", ssid);
  estado_gravar_wifi(ssid, senha);
  reiniciar_em_breve();
}

void rede_wifi_esquecer(void)
{
  estado_apagar_wifi();
  reiniciar_em_breve();
}
