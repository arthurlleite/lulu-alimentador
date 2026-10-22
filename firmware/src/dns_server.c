#include "dns_server.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "esp_log.h"

#define DNS_PORT 53
#define MAX_PACKET 512

static const char *TAG = "dns";
static volatile bool s_running;
static TaskHandle_t s_task;

// Monta a resposta em cima da própria pergunta: mesmo ID e pergunta, mais uma
// resposta tipo A apontando para o IP do AP.
static int build_reply(uint8_t *buf, int len)
{
  if (len < 12)
    return -1;
  uint16_t qd = (buf[4] << 8) | buf[5];
  if (qd != 1)
    return -1;

  // Fim do nome da pergunta + QTYPE(2) + QCLASS(2)
  int p = 12;
  while (p < len && buf[p] != 0)
    p += buf[p] + 1;
  p += 5;
  if (p > len || p + 16 > MAX_PACKET)
    return -1;
  uint16_t qtype = (buf[p - 4] << 8) | buf[p - 3];

  buf[2] = 0x81; // resposta, recursão desejada
  buf[3] = 0x80; // recursão disponível, sem erro
  buf[6] = 0;
  buf[7] = (qtype == 1) ? 1 : 0; // só responde consultas A
  buf[8] = buf[9] = buf[10] = buf[11] = 0;
  if (qtype != 1)
    return p;

  uint8_t answer[16] = {
      0xC0, 0x0C,             // ponteiro para o nome da pergunta
      0x00, 0x01, 0x00, 0x01, // tipo A, classe IN
      0x00, 0x00, 0x00, 0x3C, // TTL 60 s
      0x00, 0x04,             // 4 bytes
      192, 168, 4, 1,
  };
  memcpy(buf + p, answer, sizeof(answer));
  return p + sizeof(answer);
}

static void dns_task(void *arg)
{
  int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  struct sockaddr_in addr = {
      .sin_family = AF_INET,
      .sin_port = htons(DNS_PORT),
      .sin_addr.s_addr = htonl(INADDR_ANY),
  };
  if (sock < 0 || bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    ESP_LOGE(TAG, "Não foi possível abrir a porta 53");
    if (sock >= 0)
      close(sock);
    s_task = NULL;
    vTaskDelete(NULL);
    return;
  }
  struct timeval tv = {.tv_sec = 1};
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

  uint8_t buf[MAX_PACKET];
  while (s_running)
  {
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);
    int len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &from_len);
    if (len <= 0)
      continue;
    int out = build_reply(buf, len);
    if (out > 0)
      sendto(sock, buf, out, 0, (struct sockaddr *)&from, from_len);
  }
  close(sock);
  s_task = NULL;
  vTaskDelete(NULL);
}

void dns_server_start(void)
{
  s_running = true; // se a tarefa ainda estava saindo, ela continua
  if (s_task)
    return;
  xTaskCreate(dns_task, "dns", 3072, NULL, 3, &s_task);
}

void dns_server_stop(void)
{
  s_running = false; // a tarefa sai em até 1 s (timeout do recvfrom)
}
