#pragma once
// Estado global do alimentador, compartilhado entre as tarefas do FreeRTOS.
// Todo acesso a g_estado deve ficar entre estado_travar() e estado_liberar().

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include "sdkconfig.h"

#define VERSAO_FIRMWARE "2.0.0"
#define MAX_REFEICOES 8
#define MAX_PORCOES 10
#define TAM_HISTORICO 10

// Opções bool desligadas no menuconfig não são definidas no sdkconfig.h.
#ifdef CONFIG_ALIM_SENSOR_NIVEL
#define TEM_NIVEL true
#else
#define TEM_NIVEL false
#endif
#ifdef CONFIG_ALIM_DHT11
#define TEM_DHT11 true
#else
#define TEM_DHT11 false
#endif
#ifdef CONFIG_ALIM_SENSOR_SAIDA
#define TEM_SAIDA true
#else
#define TEM_SAIDA false
#endif
#ifdef CONFIG_ALIM_SENSOR_CHAMA
#define TEM_CHAMA true
#else
#define TEM_CHAMA false
#endif

typedef enum { ORIGEM_APP, ORIGEM_AGENDA, ORIGEM_BOTAO } origem_t;

typedef enum {
  ALARME_NENHUM,
  ALARME_RACAO_BAIXA, // ultrassom: reservatório abaixo do limite
  ALARME_SEM_SAIDA,   // sensor da saída: o servo girou mas nada caiu (vazio ou travado)
  ALARME_CHAMA,       // sensor de chama: tem prioridade sobre os outros
} alarme_t;

typedef enum { REDE_CONFIGURACAO, REDE_CONECTANDO, REDE_WIFI, REDE_ONLINE } status_rede_t;

typedef struct {
  uint8_t hora;
  uint8_t minuto;
  uint8_t porcoes;
  bool ativa;
} refeicao_t;

// Configuração gravada na NVS (flash).
typedef struct {
  char nome[32];            // nome do pet, aparece no app
  char codigo[17];          // código secreto que forma o tópico MQTT
  bool bip;                 // tocar o buzzer antes de servir
  uint16_t nivel_vazio_mm;  // distância sensor -> fundo com o tubo vazio
  uint16_t nivel_cheio_mm;  // distância sensor -> ração com o tubo cheio
  uint8_t qtd_refeicoes;
  refeicao_t refeicoes[MAX_REFEICOES];
} config_t;

typedef struct {
  time_t quando;
  uint8_t porcoes;
  origem_t origem;
  bool ok;
} registro_t;

typedef struct {
  config_t cfg;
  status_rede_t rede;
  bool portal;     // rede de configuração (AP) ativa
  bool tem_wifi;   // há credenciais salvas
  char ssid[33];
  char ip[16];
  bool servindo;
  int nivel_pct;   // -1 = sem leitura
  float nivel_cm;
  bool ambiente_ok;
  float temp_c;
  float umid_pct;
  alarme_t alarme;
  registro_t historico[TAM_HISTORICO]; // mais recente em [0]
  uint8_t tam_historico;
} estado_t;

extern estado_t g_estado;

void estado_iniciar(void);
void estado_travar(void);
void estado_liberar(void);

bool estado_hora_valida(void);
const char *estado_nome_origem(origem_t origem);

// Persistência
void estado_salvar_config(void);
bool estado_ler_wifi(char *ssid, size_t tam_ssid, char *senha, size_t tam_senha);
void estado_gravar_wifi(const char *ssid, const char *senha);
void estado_apagar_wifi(void);

// Registra uma refeição concluída e atualiza os alarmes.
void estado_registrar_refeicao(uint8_t porcoes, origem_t origem, bool ok);

// Estado completo em JSON (malloc; liberar com free). com_segredo acrescenta o
// código do tópico e o link do app, só para quem está na rede local.
char *estado_json(bool com_segredo);

// Avisa o app (MQTT) que algo mudou.
void estado_mudou(void);
