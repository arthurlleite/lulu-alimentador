#pragma once
// Servidor HTTP local (porta 80):
//   GET  /                 app (web/index.html), ou configuração se não houver Wi-Fi
//   GET  /configurar       página de configuração (web/configurar.html)
//   GET  /api/estado       estado em JSON
//   POST /api/comando      comando JSON (ver comandos.h); responde com o estado
//   GET  /api/redes        redes Wi-Fi visíveis
//   POST /api/configurar   {"ssid","senha","nome"} salva e reinicia
// Qualquer outro endereço, com a rede de configuração ativa, redireciona para
// /configurar (é assim que Android e iPhone abrem o portal sozinhos).
void servidor_web_iniciar(void);
