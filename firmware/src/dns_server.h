#pragma once
// DNS "cativo": responde qualquer nome com 192.168.4.1 enquanto a rede de
// configuração está ativa, para o celular abrir a página sozinho.
void dns_server_start(void);
void dns_server_stop(void);
