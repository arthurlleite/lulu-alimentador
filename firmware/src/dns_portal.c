#include "dns_portal.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "esp_log.h"

#define PORTA_DNS 53
#define TAM_MAX_PACOTE 512

static const char *TAG = "dns";
static volatile bool s_rodando;
static TaskHandle_t s_tarefa;

// Monta a resposta em cima da própria pergunta: mesmo ID e pergunta, mais uma
// resposta tipo A apontando para o IP do AP.
static int montar_resposta(uint8_t *buf, int tam)
{
  if (tam < 12)
    return -1;
  uint16_t perguntas = (buf[4] << 8) | buf[5];
  if (perguntas != 1)
    return -1;

  // Fim do nome da pergunta + QTYPE(2) + QCLASS(2)
  int p = 12;
  while (p < tam && buf[p] != 0)
    p += buf[p] + 1;
  p += 5;
  if (p > tam || p + 16 > TAM_MAX_PACOTE)
    return -1;
  uint16_t tipo = (buf[p - 4] << 8) | buf[p - 3];

  buf[2] = 0x81; // resposta, recursão desejada
  buf[3] = 0x80; // recursão disponível, sem erro
  buf[6] = 0;
  buf[7] = (tipo == 1) ? 1 : 0; // só responde consultas A
  buf[8] = buf[9] = buf[10] = buf[11] = 0;
  if (tipo != 1)
    return p;

  uint8_t resposta[16] = {
      0xC0, 0x0C,             // ponteiro para o nome da pergunta
      0x00, 0x01, 0x00, 0x01, // tipo A, classe IN
      0x00, 0x00, 0x00, 0x3C, // TTL 60 s
      0x00, 0x04,             // 4 bytes
      192, 168, 4, 1,
  };
  memcpy(buf + p, resposta, sizeof(resposta));
  return p + sizeof(resposta);
}

static void tarefa_dns(void *arg)
{
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  struct sockaddr_in end = {
      .sin_family = AF_INET,
      .sin_port = htons(PORTA_DNS),
      .sin_addr.s_addr = htonl(INADDR_ANY),
  };
  if (sock < 0 || bind(sock, (struct sockaddr *)&end, sizeof(end)) < 0)
  {
    ESP_LOGE(TAG, "Não foi possível abrir a porta 53");
    if (sock >= 0)
      close(sock);
    s_tarefa = NULL;
    vTaskDelete(NULL);
    return;
  }
  struct timeval tv = {.tv_sec = 1};
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  uint8_t buf[TAM_MAX_PACOTE];
  while (s_rodando)
  {
    struct sockaddr_in origem;
    socklen_t tam_origem = sizeof(origem);
    int tam = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&origem, &tam_origem);
    if (tam <= 0)
      continue;
    int saida = montar_resposta(buf, tam);
    if (saida > 0)
      sendto(sock, buf, saida, 0, (struct sockaddr *)&origem, tam_origem);
  }
  close(sock);
  s_tarefa = NULL;
  vTaskDelete(NULL);
}

void dns_portal_iniciar(void)
{
  s_rodando = true; // se a tarefa ainda estava saindo, ela continua
  if (s_tarefa)
    return;
  xTaskCreate(tarefa_dns, "dns", 3072, NULL, 3, &s_tarefa);
}

void dns_portal_parar(void)
{
  s_rodando = false; // a tarefa sai em até 1 s (timeout do recvfrom)
}
