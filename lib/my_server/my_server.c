#include "my_server.h"
/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "cJSON.h"

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */

static httpd_handle_t server = NULL;
static const char *TAG = "server";

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
  /* Send response with custom headers and body set as the
   * string passed in user context*/
  const char *resp_str = "<label for=\"ssid\">Nome da Rede:</label><br />"
                         "<input type=\"text\" id=\"ssid\" required><br />"
                         "<label for=\"password\">Password:</label><br />"
                         "<input type=\"password\" id=\"password\" required><br />"
                         "<button onclick=\"sendPOST()\">Configurar</button>"                         
                         "<script>"
                         "function sendPOST(){"
                         "    fetch('ssid_password', {method:'POST', body: '{\"ssid\":\"'+document.getElementById('ssid').value+'\",\"password\":\"'+document.getElementById('password').value+'\"}'})"
                         "}"
                         "</script>";
  httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

  return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = "Hello World!"};

/* An HTTP POST handler */
static esp_err_t ssid_password_post_handler(httpd_req_t *req)
{
  char buf[100];
  int ret, remaining = req->content_len;

  /* Read the data for the request */
  ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));

  /* Send back the same data */
  httpd_resp_send_chunk(req, buf, ret);

  /* Log data received */
  ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
  ESP_LOGI(TAG, "%.*s", ret, buf);
  ESP_LOGI(TAG, "====================================");

  printf("%s\n", buf);
  cJSON *data_JSON = cJSON_Parse(buf);
  nvs_handle nvs_handle;
  ESP_ERROR_CHECK(nvs_open("store", NVS_READWRITE, &nvs_handle));
  char ssid[30] = "";
  strcpy(ssid, cJSON_GetObjectItem(data_JSON, "ssid")->valuestring);
  char password[30] = "";
  strcpy(password, cJSON_GetObjectItem(data_JSON, "password")->valuestring);
  ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "ssid", ssid));
  ESP_ERROR_CHECK(nvs_set_str(nvs_handle, "password", password));
  ESP_ERROR_CHECK(nvs_commit(nvs_handle));
  nvs_close(nvs_handle);
  esp_restart();
  return ESP_OK;
}

static const httpd_uri_t ssid_password = {
    .uri = "/ssid_password",
    .method = HTTP_POST,
    .handler = ssid_password_post_handler,
    .user_ctx = NULL};

esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
  if (strcmp("/hello", req->uri) == 0)
  {
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/hello URI is not available");
    /* Return ESP_OK to keep underlying socket open */
    return ESP_OK;
  }
  else if (strcmp("/ssid_password", req->uri) == 0)
  {
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "/ssid_password URI is not available");
    /* Return ESP_FAIL to close underlying socket */
    return ESP_FAIL;
  }
  /* For any other URI send 404 and close socket */
  httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
  return ESP_FAIL;
}

static httpd_handle_t start_webserver(void)
{

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  // Start the httpd server
  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK)
  {
    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &hello);
    httpd_register_uri_handler(server, &ssid_password);

    return server;
  }

  ESP_LOGI(TAG, "Error starting server!");
  return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
  // Stop the httpd server
  httpd_stop(server);
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
  httpd_handle_t *server = (httpd_handle_t *)arg;
  if (*server)
  {
    ESP_LOGI(TAG, "Stopping webserver");
    stop_webserver(*server);
    *server = NULL;
  }
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
  httpd_handle_t *server = (httpd_handle_t *)arg;
  if (*server == NULL)
  {
    ESP_LOGI(TAG, "Starting webserver");
    *server = start_webserver();
  }
}

void server_init(void)
{
  server = start_webserver();
}