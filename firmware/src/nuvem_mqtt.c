#include "nuvem_mqtt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_mac.h"
#include "mqtt_client.h"
#include "esp_crt_bundle.h"
#include "estado.h"
#include "comandos.h"

static const char *TAG = "nuvem";
static esp_mqtt_client_handle_t s_cliente;
static volatile bool s_conectado;
static char s_base[64];

static void topico(char *saida, size_t tam, const char *folha)
{
  snprintf(saida, tam, "%s/%s", s_base, folha);
}

void nuvem_publicar_estado(void)
{
  if (!s_conectado)
    return;
  char t[80];
  topico(t, sizeof(t), "estado");
  char *json = estado_json(false);
  if (json)
  {
    esp_mqtt_client_publish(s_cliente, t, json, 0, 0, 1);
    free(json);
  }
}

void nuvem_publicar_evento(const char *json)
{
  if (!s_conectado)
    return;
  char t[80];
  topico(t, sizeof(t), "evento");
  esp_mqtt_client_publish(s_cliente, t, json, 0, 0, 0);
}

static void ao_receber_comando(const char *dados, int tam)
{
  cJSON *resp = comandos_executar(dados, tam, false);
  cJSON_AddStringToObject(resp, "tipo", "resposta");
  char *s = cJSON_PrintUnformatted(resp);
  cJSON_Delete(resp);
  if (s)
  {
    nuvem_publicar_evento(s);
    free(s);
  }
  nuvem_publicar_estado();
}

static void ao_evento_mqtt(void *arg, esp_event_base_t base, int32_t id, void *dados)
{
  esp_mqtt_event_handle_t ev = dados;
  char t[80];
  switch ((esp_mqtt_event_id_t)id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "Conectado ao broker");
    s_conectado = true;
    estado_travar();
    g_estado.rede = REDE_ONLINE;
    estado_liberar();
    topico(t, sizeof(t), "online");
    esp_mqtt_client_publish(s_cliente, t, "1", 1, 1, 1);
    topico(t, sizeof(t), "comando");
    esp_mqtt_client_subscribe(s_cliente, t, 1);
    nuvem_publicar_estado();
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGW(TAG, "Desconectado do broker");
    s_conectado = false;
    estado_travar();
    if (g_estado.rede == REDE_ONLINE)
      g_estado.rede = REDE_WIFI;
    estado_liberar();
    break;
  case MQTT_EVENT_DATA:
    // O original repassava o ponteiro event->data para outra tarefa; o buffer
    // é reaproveitado pelo cliente e não termina em '\0'. Aqui o comando é
    // tratado na hora, com o tamanho explícito.
    if (ev->current_data_offset == 0 && ev->data_len == ev->total_data_len)
    {
      ESP_LOGI(TAG, "Comando: %.*s", ev->data_len, ev->data);
      ao_receber_comando(ev->data, ev->data_len);
    }
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGW(TAG, "Erro MQTT (tipo %d)", ev->error_handle->error_type);
    break;
  default:
    break;
  }
}

void nuvem_iniciar(void)
{
  if (s_cliente)
    return;

  estado_travar();
  snprintf(s_base, sizeof(s_base), "%s/%s", CONFIG_ALIM_MQTT_PREFIXO, g_estado.cfg.codigo);
  estado_liberar();

  static char id_cliente[32], topico_lwt[80];
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(id_cliente, sizeof(id_cliente), "alimentador-%02x%02x%02x", mac[3], mac[4], mac[5]);
  topico(topico_lwt, sizeof(topico_lwt), "online");

  esp_mqtt_client_config_t cfg = {
      .broker.address.uri = CONFIG_ALIM_MQTT_URI,
      .broker.verification.crt_bundle_attach = esp_crt_bundle_attach,
      .credentials.client_id = id_cliente,
      .session.keepalive = 30,
      .session.last_will = {
          .topic = topico_lwt,
          .msg = "0",
          .msg_len = 1,
          .qos = 1,
          .retain = 1,
      },
      .buffer.size = 2048,
      .task.stack_size = 6144,
  };
  s_cliente = esp_mqtt_client_init(&cfg);
  esp_mqtt_client_register_event(s_cliente, ESP_EVENT_ANY_ID, ao_evento_mqtt, NULL);
  esp_mqtt_client_start(s_cliente);
  ESP_LOGI(TAG, "Broker %s, tópicos %s/#", CONFIG_ALIM_MQTT_URI, s_base);
}
