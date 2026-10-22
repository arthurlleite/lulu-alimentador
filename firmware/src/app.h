#pragma once
// Estado global do alimentador, compartilhado entre as tarefas do FreeRTOS.
// Todo acesso a g_app deve ficar entre app_lock() e app_unlock().

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include "sdkconfig.h"

#define FW_VERSION "2.0.0"
#define MAX_MEALS 8
#define MAX_PORTIONS 10
#define HISTORY_LEN 10

typedef enum { SRC_APP, SRC_SCHEDULE, SRC_BUTTON } feed_source_t;

typedef enum {
  ALARM_NONE,
  ALARM_LOW_FOOD,   // ultrassom: reservatório abaixo do limite
  ALARM_NO_OUTPUT,  // sensor IR: o servo girou mas nada caiu (vazio ou travado)
} alarm_t;

typedef enum { NET_SETUP, NET_CONNECTING, NET_WIFI, NET_ONLINE } net_status_t;

typedef struct {
  uint8_t hour;
  uint8_t minute;
  uint8_t portions;
  bool enabled;
} meal_t;

// Configuração persistida na NVS (flash).
typedef struct {
  char name[32];        // nome do pet, aparece no app
  char code[17];        // código secreto que forma o tópico MQTT
  bool beep;            // tocar o buzzer antes de servir
  uint16_t level_empty_mm;  // distância sensor -> fundo com o tubo vazio
  uint16_t level_full_mm;   // distância sensor -> ração com o tubo cheio
  uint8_t meal_count;
  meal_t meals[MAX_MEALS];
} settings_t;

typedef struct {
  time_t ts;
  uint8_t portions;
  feed_source_t src;
  bool ok;
} feed_log_t;

typedef struct {
  settings_t cfg;
  net_status_t net;
  bool portal;     // rede de configuração (AP) ativa
  bool has_wifi;   // há credenciais salvas
  char ssid[33];
  char ip[16];
  bool feeding;
  int level_pct;   // -1 = sem leitura
  float level_cm;
  bool env_ok;
  float temp_c;
  float hum_pct;
  alarm_t alarm;
  feed_log_t history[HISTORY_LEN];  // mais recente em [0]
  uint8_t history_len;
} app_state_t;

extern app_state_t g_app;

void app_init(void);
void app_lock(void);
void app_unlock(void);

bool app_time_valid(void);
const char *app_source_name(feed_source_t src);

// Persistência
void app_save_settings(void);
bool app_get_wifi(char *ssid, size_t ssid_len, char *pass, size_t pass_len);
void app_set_wifi(const char *ssid, const char *pass);
void app_erase_wifi(void);

// Registra uma refeição concluída e atualiza alarmes.
void app_record_feed(uint8_t portions, feed_source_t src, bool ok);

// Estado completo em JSON (malloc; liberar com free). include_secret adiciona o
// código do tópico e o link do app, só para quem está na rede local.
char *app_state_json(bool include_secret);

// Avisa o app (MQTT) que algo mudou.
void app_changed(void);
