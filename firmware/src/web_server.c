#include "web_server.h"

#include <stdlib.h>
#include <string.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"
#include "app.h"
#include "commands.h"
#include "wifi_mgr.h"

// O original (my_server.c) só tinha um formulário de SSID/senha sem escapar o
// JSON, sem limite de tamanho e com strcpy em buffer de 30 bytes.

#define MAX_BODY 1024

extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[] asm("_binary_index_html_end");
extern const char setup_html_start[] asm("_binary_setup_html_start");
extern const char setup_html_end[] asm("_binary_setup_html_end");

static const char *TAG = "http";

static esp_err_t send_html(httpd_req_t *req, const char *start, const char *end)
{
  httpd_resp_set_type(req, "text/html; charset=utf-8");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
  // EMBED_TXTFILES acrescenta um '\0' no fim, que não vai na resposta.
  return httpd_resp_send(req, start, end - start - 1);
}

static esp_err_t send_json(httpd_req_t *req, const char *json)
{
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_sendstr(req, json ? json : "{}");
}

static char *read_body(httpd_req_t *req)
{
  if (req->content_len == 0 || req->content_len > MAX_BODY)
    return NULL;
  char *buf = malloc(req->content_len + 1);
  if (!buf)
    return NULL;
  size_t got = 0;
  while (got < req->content_len)
  {
    int r = httpd_req_recv(req, buf + got, req->content_len - got);
    if (r == HTTPD_SOCK_ERR_TIMEOUT)
      continue;
    if (r <= 0)
    {
      free(buf);
      return NULL;
    }
    got += r;
  }
  buf[got] = '\0';
  return buf;
}

static esp_err_t root_get(httpd_req_t *req)
{
  app_lock();
  bool setup = !g_app.has_wifi;
  app_unlock();
  if (setup)
    return send_html(req, setup_html_start, setup_html_end);
  return send_html(req, index_html_start, index_html_end);
}

static esp_err_t setup_get(httpd_req_t *req)
{
  return send_html(req, setup_html_start, setup_html_end);
}

static esp_err_t state_get(httpd_req_t *req)
{
  char *json = app_state_json(true);
  esp_err_t r = send_json(req, json);
  free(json);
  return r;
}

static esp_err_t cmd_post(httpd_req_t *req)
{
  char *body = read_body(req);
  if (!body)
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Corpo inválido");
  cJSON *resp = commands_handle(body, strlen(body), true);
  free(body);

  char *state = app_state_json(true);
  cJSON *st = state ? cJSON_Parse(state) : NULL;
  free(state);
  if (st)
    cJSON_AddItemToObject(resp, "state", st);
  char *out = cJSON_PrintUnformatted(resp);
  cJSON_Delete(resp);
  esp_err_t r = send_json(req, out);
  free(out);
  app_changed();
  return r;
}

static esp_err_t scan_get(httpd_req_t *req)
{
  char *json = wifi_mgr_scan_json();
  esp_err_t r = send_json(req, json);
  free(json);
  return r;
}

static esp_err_t provision_post(httpd_req_t *req)
{
  char *body = read_body(req);
  cJSON *in = body ? cJSON_Parse(body) : NULL;
  free(body);
  const char *ssid = in ? cJSON_GetStringValue(cJSON_GetObjectItem(in, "ssid")) : NULL;
  const char *pass = in ? cJSON_GetStringValue(cJSON_GetObjectItem(in, "pass")) : NULL;
  const char *name = in ? cJSON_GetStringValue(cJSON_GetObjectItem(in, "name")) : NULL;
  if (!pass)
    pass = "";

  const char *err = NULL;
  if (!ssid || strlen(ssid) < 1 || strlen(ssid) > 32)
    err = "Escolha uma rede Wi-Fi";
  else if (strlen(pass) > 0 && (strlen(pass) < 8 || strlen(pass) > 63))
    err = "A senha do Wi-Fi tem de 8 a 63 caracteres";
  else if (name && (strlen(name) < 1 || strlen(name) > 31))
    err = "O nome do pet tem de 1 a 31 caracteres";

  cJSON *resp = cJSON_CreateObject();
  if (err)
  {
    cJSON_AddBoolToObject(resp, "ok", false);
    cJSON_AddStringToObject(resp, "msg", err);
  }
  else
  {
    app_lock();
    if (name)
      strlcpy(g_app.cfg.name, name, sizeof(g_app.cfg.name));
    app_save_settings();
    char link[512];
    snprintf(link, sizeof(link), "%s?d=%s", CONFIG_FEEDER_APP_URL, g_app.cfg.code);
    cJSON_AddStringToObject(resp, "code", g_app.cfg.code);
    app_unlock();
    cJSON_AddBoolToObject(resp, "ok", true);
    cJSON_AddStringToObject(resp, "msg", "Salvo! Reiniciando para conectar...");
    cJSON_AddStringToObject(resp, "app_url", link);
    cJSON_AddStringToObject(resp, "host", CONFIG_FEEDER_HOSTNAME ".local");
    wifi_mgr_provision(ssid, pass);
  }
  cJSON_Delete(in);
  char *out = cJSON_PrintUnformatted(resp);
  cJSON_Delete(resp);
  esp_err_t r = send_json(req, out);
  free(out);
  return r;
}

static esp_err_t not_found(httpd_req_t *req, httpd_err_code_t code)
{
  app_lock();
  bool portal = g_app.portal;
  app_unlock();
  if (portal)
  {
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/setup");
    return httpd_resp_send(req, NULL, 0);
  }
  return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Não encontrado");
}

void web_server_start(void)
{
  httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
  cfg.max_uri_handlers = 12;
  cfg.lru_purge_enable = true;
  cfg.stack_size = 6144;

  httpd_handle_t server = NULL;
  if (httpd_start(&server, &cfg) != ESP_OK)
  {
    ESP_LOGE(TAG, "Falha ao iniciar o servidor HTTP");
    return;
  }
  const httpd_uri_t routes[] = {
      {.uri = "/", .method = HTTP_GET, .handler = root_get},
      {.uri = "/setup", .method = HTTP_GET, .handler = setup_get},
      {.uri = "/api/state", .method = HTTP_GET, .handler = state_get},
      {.uri = "/api/cmd", .method = HTTP_POST, .handler = cmd_post},
      {.uri = "/api/scan", .method = HTTP_GET, .handler = scan_get},
      {.uri = "/api/provision", .method = HTTP_POST, .handler = provision_post},
  };
  for (size_t i = 0; i < sizeof(routes) / sizeof(routes[0]); i++)
    httpd_register_uri_handler(server, &routes[i]);
  httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, not_found);
  ESP_LOGI(TAG, "Servidor HTTP na porta %d", cfg.server_port);
}
