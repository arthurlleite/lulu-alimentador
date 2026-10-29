#pragma once
// Ponte com o app na internet via broker MQTT público (TLS).
//
// Tópicos (<base> = prefixo/código secreto, ex.: alimentador/3f9a0c1d2e4b5a6f):
//   <base>/comando   app -> ESP   comandos JSON (ver comandos.h)
//   <base>/estado    ESP -> app   estado completo, retido
//   <base>/evento    ESP -> app   respostas e avisos (resposta, alimentado, racao_baixa, chama)
//   <base>/online    ESP -> app   "1"/"0", retido (testamento/LWT)

void nuvem_iniciar(void);
void nuvem_publicar_estado(void);
void nuvem_publicar_evento(const char *json);
