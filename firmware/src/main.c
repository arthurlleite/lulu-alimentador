// Alimentador Automático de Pets — ESP32 + ESP-IDF (FreeRTOS)
//
// Tarefas:
//   feeder    (prio 6) fila de refeições -> servo + sensor IR da saída
//   agenda    (prio 5) confere os horários a cada segundo
//   ui        (prio 4) LED RGB de status e botão
//   sensores  (prio 3) nível (HC-SR04) e temperatura/umidade (DHT11)
//   + tarefas do ESP-IDF: Wi-Fi, HTTP, MQTT, DNS do portal, SNTP, mDNS

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "app.h"
#include "feeder.h"
#include "scheduler.h"
#include "sensors.h"
#include "ui.h"
#include "web_server.h"
#include "wifi_mgr.h"

void app_main(void)
{
  ESP_LOGI("main", "Alimentador v%s", FW_VERSION);
  app_init(); // NVS, configuração salva, fuso horário
  ui_init();
  feeder_init();
  sensors_init();
  scheduler_init();
  wifi_mgr_start();
  web_server_start();
}
