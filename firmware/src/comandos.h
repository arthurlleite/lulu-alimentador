#pragma once
// Comandos JSON, iguais pelo MQTT (app na internet) e pelo HTTP (app na rede local):
//
//   {"comando":"alimentar","porcoes":2}
//   {"comando":"agenda","refeicoes":[{"h":7,"m":30,"p":2,"ativa":true}, ...]}   (até 8)
//   {"comando":"ajustes","nome":"Mel","bip":true,"nivel_vazio_cm":30,"nivel_cheio_cm":4}
//   {"comando":"status"}
//   {"comando":"limpar_alarme"}
//   {"comando":"acertar_hora","unix":1767225600}   (só local: acerta o relógio sem internet)
//   {"comando":"esquecer_wifi"}                    (só local: apaga o Wi-Fi e reabre a configuração)
//
// Um campo "id" opcional volta na resposta, para o app casar pedido e resposta.

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

// Retorna {"ok":bool,"mensagem":"...","comando":"...","id":...}; liberar com cJSON_Delete.
cJSON *comandos_executar(const char *dados, size_t tam, bool local);
