#include "mqtt_link.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "esp_mac.h"
#include "mqtt_client.h"
#include "esp_crt_bundle.h"
#include "app.h"
#include "commands.h"

static const char *TAG = "mqtt";
static esp_mqtt_client_handle_t s_client;
static volatile bool s_connected;
static char s_base[64];

static void topic(char *out, size_t len, const char *leaf)
{
  snprintf(out, len, "%s/%s", s_base, leaf);
}

void mqtt_link_publish_state(void)
{
  if (!s_connected)
    return;
  char t[80];
  topic(t, sizeof(t), "state");
  char *json = app_state_json(false);
  if (json)
  {
    esp_mqtt_client_publish(s_client, t, json, 0, 0, 1);
    free(json);
  }
}

void mqtt_link_publish_event(const char *json)
{
  if (!s_connected)
    return;
  char t[80];
  topic(t, sizeof(t), "event");
  esp_mqtt_client_publish(s_client, t, json, 0, 0, 0);
}

static void on_command(const char *data, int len)
{
  cJSON *resp = commands_handle(data, len, false);
  cJSON_AddStringToObject(resp, "type", "ack");
  char *s = cJSON_PrintUnformatted(resp);
  cJSON_Delete(resp);
  if (s)
  {
    mqtt_link_publish_event(s);
    free(s);
  }
  mqtt_link_publish_state();
}

static void mqtt_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
  esp_mqtt_event_handle_t ev = data;
  char t[80];
  switch ((esp_mqtt_event_id_t)id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "Conectado ao broker");
    s_connected = true;
    app_lock();
    g_app.net = NET_ONLINE;
    app_unlock();
    topic(t, sizeof(t), "online");
    esp_mqtt_client_publish(s_client, t, "1", 1, 1, 1);
    topic(t, sizeof(t), "cmd");
    esp_mqtt_client_subscribe(s_client, t, 1);
    mqtt_link_publish_state();
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGW(TAG, "Desconectado do broker");
    s_connected = false;
    app_lock();
    if (g_app.net == NET_ONLINE)
      g_app.net = NET_WIFI;
    app_unlock();
    break;
  case MQTT_EVENT_DATA:
    // O original repassava o ponteiro event->data para outra tarefa; o buffer
    // é reaproveitado pelo cliente e não termina em '\0'. Aqui o comando é
    // tratado na hora, com o tamanho explícito.
    if (ev->current_data_offset == 0 && ev->data_len == ev->total_data_len)
    {
      ESP_LOGI(TAG, "Comando: %.*s", ev->data_len, ev->data);
      on_command(ev->data, ev->data_len);
    }
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGW(TAG, "Erro MQTT (tipo %d)", ev->error_handle->error_type);
    break;
  default:
    break;
  }
}

void mqtt_link_start(void)
{
  if (s_client)
    return;

  app_lock();
  snprintf(s_base, sizeof(s_base), "%s/%s", CONFIG_FEEDER_MQTT_PREFIX, g_app.cfg.code);
  app_unlock();

  static char client_id[32], lwt_topic[80];
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  snprintf(client_id, sizeof(client_id), "alimentador-%02x%02x%02x", mac[3], mac[4], mac[5]);
  topic(lwt_topic, sizeof(lwt_topic), "online");

  esp_mqtt_client_config_t cfg = {
      .broker.address.uri = CONFIG_FEEDER_MQTT_URI,
      .broker.verification.crt_bundle_attach = esp_crt_bundle_attach,
      .credentials.client_id = client_id,
      .session.keepalive = 30,
      .session.last_will = {
          .topic = lwt_topic,
          .msg = "0",
          .msg_len = 1,
          .qos = 1,
          .retain = 1,
      },
      .buffer.size = 2048,
      .task.stack_size = 6144,
  };
  s_client = esp_mqtt_client_init(&cfg);
  esp_mqtt_client_register_event(s_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
  esp_mqtt_client_start(s_client);
  ESP_LOGI(TAG, "Broker %s, tópicos %s/#", CONFIG_FEEDER_MQTT_URI, s_base);
}
