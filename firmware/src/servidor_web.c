#include "servidor_web.h"

#include <stdlib.h>
#include <string.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "cJSON.h"
#include "estado.h"
#include "comandos.h"
#include "rede_wifi.h"

// O original (my_server.c) só tinha um formulário de SSID/senha sem escapar o
// JSON, sem limite de tamanho e com strcpy em buffer de 30 bytes.

#define TAM_MAX_CORPO 1024

extern const char index_html_inicio[] asm("_binary_index_html_start");
extern const char index_html_fim[] asm("_binary_index_html_end");
extern const char configurar_html_inicio[] asm("_binary_configurar_html_start");
extern const char configurar_html_fim[] asm("_binary_configurar_html_end");

static const char *TAG = "http";

static esp_err_t enviar_html(httpd_req_t *req, const char *inicio, const char *fim)
{
  httpd_resp_set_type(req, "text/html; charset=utf-8");
  httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
  // EMBED_TXTFILES acrescenta um '\0' no fim, que não vai na resposta.
  return httpd_resp_send(req, inicio, fim - inicio - 1);
}

static esp_err_t enviar_json(httpd_req_t *req, const char *json)
{
  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Cache-Control", "no-store");
  return httpd_resp_sendstr(req, json ? json : "{}");
}

static char *ler_corpo(httpd_req_t *req)
{
  if (req->content_len == 0 || req->content_len > TAM_MAX_CORPO)
    return NULL;
  char *buf = malloc(req->content_len + 1);
  if (!buf)
    return NULL;
  size_t lido = 0;
  while (lido < req->content_len)
  {
    int r = httpd_req_recv(req, buf + lido, req->content_len - lido);
    if (r == HTTPD_SOCK_ERR_TIMEOUT)
      continue;
    if (r <= 0)
    {
      free(buf);
      return NULL;
    }
    lido += r;
  }
  buf[lido] = '\0';
  return buf;
}

static esp_err_t get_raiz(httpd_req_t *req)
{
  estado_travar();
  bool configurar = !g_estado.tem_wifi;
  estado_liberar();
  if (configurar)
    return enviar_html(req, configurar_html_inicio, configurar_html_fim);
  return enviar_html(req, index_html_inicio, index_html_fim);
}

static esp_err_t get_configurar(httpd_req_t *req)
{
  return enviar_html(req, configurar_html_inicio, configurar_html_fim);
}

static esp_err_t get_estado(httpd_req_t *req)
{
  char *json = estado_json(true);
  esp_err_t r = enviar_json(req, json);
  free(json);
  return r;
}

static esp_err_t post_comando(httpd_req_t *req)
{
  char *corpo = ler_corpo(req);
  if (!corpo)
    return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Corpo inválido");
  cJSON *resp = comandos_executar(corpo, strlen(corpo), true);
  free(corpo);

  char *estado = estado_json(true);
  cJSON *e = estado ? cJSON_Parse(estado) : NULL;
  free(estado);
  if (e)
    cJSON_AddItemToObject(resp, "estado", e);
  char *saida = cJSON_PrintUnformatted(resp);
  cJSON_Delete(resp);
  esp_err_t r = enviar_json(req, saida);
  free(saida);
  estado_mudou();
  return r;
}

static esp_err_t get_redes(httpd_req_t *req)
{
  char *json = rede_wifi_buscar_json();
  esp_err_t r = enviar_json(req, json);
  free(json);
  return r;
}

static esp_err_t post_configurar(httpd_req_t *req)
{
  char *corpo = ler_corpo(req);
  cJSON *in = corpo ? cJSON_Parse(corpo) : NULL;
  free(corpo);
  const char *ssid = in ? cJSON_GetStringValue(cJSON_GetObjectItem(in, "ssid")) : NULL;
  const char *senha = in ? cJSON_GetStringValue(cJSON_GetObjectItem(in, "senha")) : NULL;
  const char *nome = in ? cJSON_GetStringValue(cJSON_GetObjectItem(in, "nome")) : NULL;
  if (!senha)
    senha = "";

  const char *erro = NULL;
  if (!ssid || strlen(ssid) < 1 || strlen(ssid) > 32)
    erro = "Escolha uma rede Wi-Fi";
  else if (strlen(senha) > 0 && (strlen(senha) < 8 || strlen(senha) > 63))
    erro = "A senha do Wi-Fi tem de 8 a 63 caracteres";
  else if (nome && (strlen(nome) < 1 || strlen(nome) > 31))
    erro = "O nome do pet tem de 1 a 31 caracteres";

  cJSON *resp = cJSON_CreateObject();
  if (erro)
  {
    cJSON_AddBoolToObject(resp, "ok", false);
    cJSON_AddStringToObject(resp, "mensagem", erro);
  }
  else
  {
    estado_travar();
    if (nome)
      strlcpy(g_estado.cfg.nome, nome, sizeof(g_estado.cfg.nome));
    estado_salvar_config();
    char link[512];
    snprintf(link, sizeof(link), "%s?d=%s", CONFIG_ALIM_URL_APP, g_estado.cfg.codigo);
    cJSON_AddStringToObject(resp, "codigo", g_estado.cfg.codigo);
    estado_liberar();
    cJSON_AddBoolToObject(resp, "ok", true);
    cJSON_AddStringToObject(resp, "mensagem", "Salvo! Reiniciando para conectar...");
    cJSON_AddStringToObject(resp, "link_app", link);
    cJSON_AddStringToObject(resp, "endereco_local", CONFIG_ALIM_NOME_MDNS ".local");
    rede_wifi_configurar(ssid, senha);
  }
  cJSON_Delete(in);
  char *saida = cJSON_PrintUnformatted(resp);
  cJSON_Delete(resp);
  esp_err_t r = enviar_json(req, saida);
  free(saida);
  return r;
}

static esp_err_t nao_encontrado(httpd_req_t *req, httpd_err_code_t codigo)
{
  estado_travar();
  bool portal = g_estado.portal;
  estado_liberar();
  if (portal)
  {
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "http://192.168.4.1/configurar");
    return httpd_resp_send(req, NULL, 0);
  }
  return httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Não encontrado");
}

void servidor_web_iniciar(void)
{
  httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
  cfg.max_uri_handlers = 12;
  cfg.lru_purge_enable = true;
  cfg.stack_size = 6144;

  httpd_handle_t servidor = NULL;
  if (httpd_start(&servidor, &cfg) != ESP_OK)
  {
    ESP_LOGE(TAG, "Falha ao iniciar o servidor HTTP");
    return;
  }
  const httpd_uri_t rotas[] = {
      {.uri = "/", .method = HTTP_GET, .handler = get_raiz},
      {.uri = "/configurar", .method = HTTP_GET, .handler = get_configurar},
      {.uri = "/api/estado", .method = HTTP_GET, .handler = get_estado},
      {.uri = "/api/comando", .method = HTTP_POST, .handler = post_comando},
      {.uri = "/api/redes", .method = HTTP_GET, .handler = get_redes},
      {.uri = "/api/configurar", .method = HTTP_POST, .handler = post_configurar},
  };
  for (size_t i = 0; i < sizeof(rotas) / sizeof(rotas[0]); i++)
    httpd_register_uri_handler(servidor, &rotas[i]);
  httpd_register_err_handler(servidor, HTTPD_404_NOT_FOUND, nao_encontrado);
  ESP_LOGI(TAG, "Servidor HTTP na porta %d", cfg.server_port);
}
