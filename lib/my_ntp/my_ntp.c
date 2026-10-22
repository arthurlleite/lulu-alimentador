#include "my_ntp.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define TAG_NTP "NTP_TIME"

void print_time(long time, const char *message)
{
  struct tm *timeinfo = localtime(&time);

  char buffer[50];
  strftime(buffer, sizeof(buffer), "%c", timeinfo);
  ESP_LOGI(TAG_NTP, "message: %s: %s", message, buffer);
}

void ntp_init(void)
{
  setenv("TZ", "UTC+3", 1);
  tzset();
  sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
  sntp_setservername(0, "br.pool.ntp.org");
  sntp_init();  
}
