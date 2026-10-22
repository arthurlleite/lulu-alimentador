#pragma once
// Wi-Fi com configuração pelo celular (portal cativo).
//
// - Sem Wi-Fi salvo: cria a rede "Alimentador-XXXX"; ao conectar, o celular abre
//   a página de configuração, onde se escolhe a rede, a senha e o nome do pet.
// - Com Wi-Fi salvo: conecta e tenta de novo para sempre (com espera crescente).
//   Se não conectar em CONFIG_FEEDER_PORTAL_FALLBACK_S, reabre a rede de
//   configuração sem parar de tentar (útil se a senha do roteador mudou).
// - Conectado: sincroniza o relógio (NTP), anuncia alimentador.local (mDNS) e
//   liga o MQTT.

#include <stdbool.h>

void wifi_mgr_start(void);

// Lista de redes visíveis em JSON (malloc; liberar com free).
char *wifi_mgr_scan_json(void);

// Salva a rede escolhida e reinicia em seguida.
void wifi_mgr_provision(const char *ssid, const char *pass);

// Apaga o Wi-Fi salvo e reinicia na rede de configuração.
void wifi_mgr_factory_reset(void);
