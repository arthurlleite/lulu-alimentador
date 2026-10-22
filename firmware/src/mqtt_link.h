#pragma once
// Ponte com o app na internet via broker MQTT público (TLS).
//
// Tópicos (<base> = prefixo/código secreto, ex.: alimentador/3f9a0c1d2e4b5a6f):
//   <base>/cmd     app -> ESP   comandos JSON (ver commands.h)
//   <base>/state   ESP -> app   estado completo, retido
//   <base>/event   ESP -> app   respostas e avisos (fed, low_food, ack)
//   <base>/online  ESP -> app   "1"/"0", retido (testamento/LWT)

void mqtt_link_start(void);
void mqtt_link_publish_state(void);
void mqtt_link_publish_event(const char *json);
