#include "estado.h"

#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_random.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "cJSON.h"
#include "nuvem_mqtt.h"

#define NVS_ESPACO "alimentador"
#define VERSAO_CONFIG 3

static const char *TAG = "estado";
static SemaphoreHandle_t s_trava;

estado_t g_estado;

static void config_padrao(config_t *c)
{
  memset(c, 0, sizeof(*c));
  strcpy(c->nome, "Pet");
  c->bip = true;
  c->nivel_vazio_mm = 300;
  c->nivel_cheio_mm = 40;
  c->qtd_refeicoes = 2;
  c->refeicoes[0] = (refeicao_t){.hora = 7, .minuto = 0, .porcoes = 1, .ativa = true};
  c->refeicoes[1] = (refeicao_t){.hora = 19, .minuto = 0, .porcoes = 1, .ativa = true};
}

static void novo_codigo(char *saida)
{
  // 16 caracteres hex = 64 bits aleatórios do gerador de hardware.
  uint32_t a = esp_random(), b = esp_random();
  snprintf(saida, 17, "%08lx%08lx", (unsigned long)a, (unsigned long)b);
}

static void carregar_config(void)
{
  config_t *c = &g_estado.cfg;
  nvs_handle_t h;
  bool carregou = false;
  if (nvs_open(NVS_ESPACO, NVS_READONLY, &h) == ESP_OK)
  {
    uint8_t versao = 0;
    size_t tam = sizeof(*c);
    if (nvs_get_u8(h, "cfg_v", &versao) == ESP_OK && versao == VERSAO_CONFIG &&
        nvs_get_blob(h, "cfg", c, &tam) == ESP_OK && tam == sizeof(*c))
    {
      carregou = true;
    }
    nvs_close(h);
  }
  if (!carregou)
  {
    ESP_LOGI(TAG, "Sem configuração salva, usando os padrões");
    config_padrao(c);
  }
  if (strlen(c->codigo) != 16)
  {
    novo_codigo(c->codigo);
    carregou = false;
  }
  c->nome[sizeof(c->nome) - 1] = '\0';
  if (c->qtd_refeicoes > MAX_REFEICOES)
    c->qtd_refeicoes = 0;
  if (!carregou)
    estado_salvar_config();
}

void estado_iniciar(void)
{
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  s_trava = xSemaphoreCreateMutex();
  memset(&g_estado, 0, sizeof(g_estado));
  g_estado.nivel_pct = -1;
  carregar_config();

  char ssid[33], senha[65];
  g_estado.tem_wifi = estado_ler_wifi(ssid, sizeof(ssid), senha, sizeof(senha));
  if (g_estado.tem_wifi)
    strlcpy(g_estado.ssid, ssid, sizeof(g_estado.ssid));

  setenv("TZ", CONFIG_ALIM_FUSO, 1);
  tzset();
  ESP_LOGI(TAG, "Pet: %s | tópico: %s/%s", g_estado.cfg.nome, CONFIG_ALIM_MQTT_PREFIXO, g_estado.cfg.codigo);
}

void estado_travar(void) { xSemaphoreTake(s_trava, portMAX_DELAY); }
void estado_liberar(void) { xSemaphoreGive(s_trava); }

bool estado_hora_valida(void)
{
  time_t agora = time(NULL);
  struct tm t;
  localtime_r(&agora, &t);
  return t.tm_year >= (2025 - 1900);
}

const char *estado_nome_origem(origem_t origem)
{
  switch (origem)
  {
  case ORIGEM_AGENDA:
    return "agenda";
  case ORIGEM_BOTAO:
    return "botao";
  default:
    return "app";
  }
}

void estado_salvar_config(void)
{
  nvs_handle_t h;
  if (nvs_open(NVS_ESPACO, NVS_READWRITE, &h) != ESP_OK)
    return;
  nvs_set_u8(h, "cfg_v", VERSAO_CONFIG);
  nvs_set_blob(h, "cfg", &g_estado.cfg, sizeof(g_estado.cfg));
  nvs_commit(h);
  nvs_close(h);
}

bool estado_ler_wifi(char *ssid, size_t tam_ssid, char *senha, size_t tam_senha)
{
  nvs_handle_t h;
  if (nvs_open(NVS_ESPACO, NVS_READONLY, &h) != ESP_OK)
    return false;
  bool ok = nvs_get_str(h, "ssid", ssid, &tam_ssid) == ESP_OK &&
            nvs_get_str(h, "senha", senha, &tam_senha) == ESP_OK && ssid[0] != '\0';
  nvs_close(h);
  return ok;
}

void estado_gravar_wifi(const char *ssid, const char *senha)
{
  nvs_handle_t h;
  ESP_ERROR_CHECK(nvs_open(NVS_ESPACO, NVS_READWRITE, &h));
  nvs_set_str(h, "ssid", ssid);
  nvs_set_str(h, "senha", senha);
  nvs_commit(h);
  nvs_close(h);
}

void estado_apagar_wifi(void)
{
  nvs_handle_t h;
  if (nvs_open(NVS_ESPACO, NVS_READWRITE, &h) != ESP_OK)
    return;
  nvs_erase_key(h, "ssid");
  nvs_erase_key(h, "senha");
  nvs_commit(h);
  nvs_close(h);
}

void estado_registrar_refeicao(uint8_t porcoes, origem_t origem, bool ok)
{
  estado_travar();
  memmove(&g_estado.historico[1], &g_estado.historico[0], sizeof(registro_t) * (TAM_HISTORICO - 1));
  g_estado.historico[0] = (registro_t){
      .quando = estado_hora_valida() ? time(NULL) : 0,
      .porcoes = porcoes,
      .origem = origem,
      .ok = ok,
  };
  if (g_estado.tam_historico < TAM_HISTORICO)
    g_estado.tam_historico++;
  g_estado.servindo = false;
  if (!ok)
    g_estado.alarme = ALARME_SEM_SAIDA;
  else if (g_estado.alarme == ALARME_SEM_SAIDA)
    g_estado.alarme = ALARME_NENHUM;
  estado_liberar();
}

static const char *nome_rede(status_rede_t r)
{
  switch (r)
  {
  case REDE_CONFIGURACAO:
    return "configuracao";
  case REDE_CONECTANDO:
    return "conectando";
  case REDE_WIFI:
    return "wifi";
  default:
    return "online";
  }
}

static const char *nome_alarme(alarme_t a)
{
  switch (a)
  {
  case ALARME_RACAO_BAIXA:
    return "racao_baixa";
  case ALARME_SEM_SAIDA:
    return "sem_saida";
  default:
    return NULL;
  }
}

static cJSON *refeicao_json(const refeicao_t *r)
{
  cJSON *o = cJSON_CreateObject();
  cJSON_AddNumberToObject(o, "h", r->hora);
  cJSON_AddNumberToObject(o, "m", r->minuto);
  cJSON_AddNumberToObject(o, "p", r->porcoes);
  cJSON_AddBoolToObject(o, "ativa", r->ativa);
  return o;
}

char *estado_json(bool com_segredo)
{
  cJSON *raiz = cJSON_CreateObject();
  bool hora_ok = estado_hora_valida();
  time_t agora = time(NULL);

  estado_travar();
  const estado_t *e = &g_estado;
  cJSON_AddStringToObject(raiz, "nome", e->cfg.nome);
  cJSON_AddStringToObject(raiz, "firmware", VERSAO_FIRMWARE);
  cJSON_AddStringToObject(raiz, "rede", nome_rede(e->rede));
  cJSON_AddBoolToObject(raiz, "hora_ok", hora_ok);
  if (hora_ok)
    cJSON_AddNumberToObject(raiz, "agora", (double)agora);
  cJSON_AddBoolToObject(raiz, "servindo", e->servindo);
  cJSON_AddBoolToObject(raiz, "bip", e->cfg.bip);
  const char *alarme = nome_alarme(e->alarme);
  if (alarme)
    cJSON_AddStringToObject(raiz, "alarme", alarme);
  else
    cJSON_AddNullToObject(raiz, "alarme");

  cJSON *wifi = cJSON_AddObjectToObject(raiz, "wifi");
  cJSON_AddStringToObject(wifi, "ssid", e->ssid);
  cJSON_AddStringToObject(wifi, "ip", e->ip);
  cJSON_AddBoolToObject(wifi, "portal", e->portal);

  if (e->nivel_pct >= 0)
  {
    cJSON *nivel = cJSON_AddObjectToObject(raiz, "nivel");
    cJSON_AddNumberToObject(nivel, "pct", e->nivel_pct);
    cJSON_AddNumberToObject(nivel, "cm", (int)(e->nivel_cm * 10) / 10.0);
  }
  else
    cJSON_AddNullToObject(raiz, "nivel");
  cJSON *cal = cJSON_AddObjectToObject(raiz, "calibracao");
  cJSON_AddNumberToObject(cal, "vazio_cm", e->cfg.nivel_vazio_mm / 10.0);
  cJSON_AddNumberToObject(cal, "cheio_cm", e->cfg.nivel_cheio_mm / 10.0);

  if (e->ambiente_ok)
  {
    cJSON *amb = cJSON_AddObjectToObject(raiz, "ambiente");
    cJSON_AddNumberToObject(amb, "temp", e->temp_c);
    cJSON_AddNumberToObject(amb, "umid", e->umid_pct);
  }
  else
    cJSON_AddNullToObject(raiz, "ambiente");

  cJSON *lista = cJSON_AddArrayToObject(raiz, "refeicoes");
  int melhor = -1, melhor_em = 24 * 60 + 1;
  struct tm t;
  localtime_r(&agora, &t);
  int agora_min = t.tm_hour * 60 + t.tm_min;
  for (int i = 0; i < e->cfg.qtd_refeicoes; i++)
  {
    const refeicao_t *r = &e->cfg.refeicoes[i];
    cJSON_AddItemToArray(lista, refeicao_json(r));
    if (!r->ativa)
      continue;
    int em = (r->hora * 60 + r->minuto - agora_min + 24 * 60) % (24 * 60);
    if (em == 0)
      em = 24 * 60;
    if (em < melhor_em)
    {
      melhor_em = em;
      melhor = i;
    }
  }
  if (melhor >= 0 && hora_ok)
  {
    cJSON *prox = refeicao_json(&e->cfg.refeicoes[melhor]);
    cJSON_AddNumberToObject(prox, "em_min", melhor_em);
    cJSON_AddItemToObject(raiz, "proxima", prox);
  }
  else
    cJSON_AddNullToObject(raiz, "proxima");

  cJSON *hist = cJSON_AddArrayToObject(raiz, "historico");
  for (int i = 0; i < e->tam_historico; i++)
  {
    const registro_t *r = &e->historico[i];
    cJSON *o = cJSON_CreateObject();
    cJSON_AddNumberToObject(o, "quando", (double)r->quando);
    cJSON_AddNumberToObject(o, "p", r->porcoes);
    cJSON_AddStringToObject(o, "origem", estado_nome_origem(r->origem));
    cJSON_AddBoolToObject(o, "ok", r->ok);
    cJSON_AddItemToArray(hist, o);
  }

  if (com_segredo)
  {
    char link[512];
    snprintf(link, sizeof(link), "%s?d=%s", CONFIG_ALIM_URL_APP, e->cfg.codigo);
    cJSON_AddStringToObject(raiz, "codigo", e->cfg.codigo);
    cJSON_AddStringToObject(raiz, "link_app", link);
    cJSON_AddStringToObject(raiz, "endereco_local", CONFIG_ALIM_NOME_MDNS ".local");
  }
  estado_liberar();

  char *saida = cJSON_PrintUnformatted(raiz);
  cJSON_Delete(raiz);
  return saida;
}

void estado_mudou(void)
{
  nuvem_publicar_estado();
}
