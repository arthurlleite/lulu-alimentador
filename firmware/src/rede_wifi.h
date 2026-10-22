#pragma once
// Wi-Fi com configuração pelo celular (portal cativo).
//
// - Sem Wi-Fi salvo: cria a rede "Alimentador-XXXX"; ao conectar, o celular abre
//   a página de configuração, onde se escolhe a rede, a senha e o nome do pet.
// - Com Wi-Fi salvo: conecta e tenta de novo para sempre (com espera crescente).
//   Se não conectar em CONFIG_ALIM_PORTAL_APOS_S, reabre a rede de configuração
//   sem parar de tentar (útil se a senha do roteador mudou).
// - Conectado: acerta o relógio (NTP), anuncia alimentador.local (mDNS) e liga
//   o MQTT.

#include <stdbool.h>

void rede_wifi_iniciar(void);

// Lista de redes visíveis em JSON (malloc; liberar com free).
char *rede_wifi_buscar_json(void);

// Salva a rede escolhida e reinicia em seguida.
void rede_wifi_configurar(const char *ssid, const char *senha);

// Apaga o Wi-Fi salvo e reinicia na rede de configuração.
void rede_wifi_esquecer(void);
