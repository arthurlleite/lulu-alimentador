#include "my_mdns.h"

/* MDNS-SD Query and advertise Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif_ip_addr.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "mdns.h"
#include "driver/gpio.h"
#include "netdb.h"

static const char *TAG = "mdns";

void initialise_mdns(void)
{
  char hostname[20] = "alimentador";
  // initialize mDNS
  ESP_ERROR_CHECK(mdns_init());
  // set mDNS hostname (required if you want to advertise services)
  ESP_ERROR_CHECK(mdns_hostname_set(hostname));
  ESP_LOGI(TAG, "mdns hostname set to: [%s]", hostname);
  // set default mDNS instance name
  ESP_ERROR_CHECK(mdns_instance_name_set("alimentador"));
}
