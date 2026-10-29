// Alimentador Automático de Pets — ESP32 + ESP-IDF (FreeRTOS)
//
// Tarefas:
//   dosador    (prio 6) fila de refeições -> servo + sensor da saída
//   agenda     (prio 5) confere os horários a cada segundo
//   interface  (prio 4) LED RGB de status e botão
//   chama      (prio 4) alarme de fogo (sensor de chama)
//   sensores   (prio 3) nível (HC-SR04) e temperatura/umidade (DHT11), se ligados
//   + tarefas do ESP-IDF: Wi-Fi, HTTP, MQTT, DNS do portal, SNTP, mDNS

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "estado.h"
#include "dosador.h"
#include "agenda.h"
#include "sensores.h"
#include "interface.h"
#include "servidor_web.h"
#include "rede_wifi.h"

void app_main(void) // nome exigido pelo ESP-IDF
{
  ESP_LOGI("main", "Alimentador v%s", VERSAO_FIRMWARE);
  estado_iniciar(); // NVS, configuração salva, fuso horário
  interface_iniciar();
  dosador_iniciar();
  sensores_iniciar();
  agenda_iniciar();
  rede_wifi_iniciar();
  servidor_web_iniciar();
}
