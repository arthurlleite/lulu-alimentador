#pragma once
// Comandos JSON, iguais pelo MQTT (app na internet) e pelo HTTP (app na rede local):
//
//   {"cmd":"feed","portions":2}
//   {"cmd":"schedule","meals":[{"h":7,"m":30,"p":2,"on":true}, ...]}   (até 8)
//   {"cmd":"settings","name":"Rex","beep":true,"level_empty_cm":30,"level_full_cm":4}
//   {"cmd":"status"}
//   {"cmd":"clear_alarm"}
//   {"cmd":"time","epoch":1767225600}     (só local: acerta o relógio sem internet)
//   {"cmd":"wifi_reset"}                  (só local: apaga o Wi-Fi e reabre a configuração)
//
// Um campo "id" opcional volta na resposta, para o app casar pedido e resposta.

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

// Retorna {"ok":bool,"msg":"...","cmd":"...","id":...}; liberar com cJSON_Delete.
cJSON *commands_handle(const char *data, size_t len, bool local);
