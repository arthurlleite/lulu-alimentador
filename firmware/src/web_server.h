#pragma once
// Servidor HTTP local (porta 80):
//   GET  /               app (web/index.html) ou configuração, se não houver Wi-Fi
//   GET  /setup          página de configuração (web/setup.html)
//   GET  /api/state      estado em JSON
//   POST /api/cmd        comando JSON (ver commands.h); responde com o estado
//   GET  /api/scan       redes Wi-Fi visíveis
//   POST /api/provision  {"ssid","pass","name"} salva e reinicia
// Qualquer outro endereço, com a rede de configuração ativa, redireciona para
// /setup (é assim que Android e iPhone abrem o portal sozinhos).
void web_server_start(void);
