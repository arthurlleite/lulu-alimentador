#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "app.h"

void feeder_init(void);

// Coloca uma refeição na fila. Retorna false se a fila estiver cheia.
bool feeder_request(uint8_t portions, feed_source_t src);
