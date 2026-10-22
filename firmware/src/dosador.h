#pragma once
// Dosador: servo que abre a passagem da ração, uma vez por porção.
#include <stdbool.h>
#include <stdint.h>
#include "estado.h"

void dosador_iniciar(void);

// Coloca uma refeição na fila. Retorna false se a fila estiver cheia.
bool dosador_pedir(uint8_t porcoes, origem_t origem);
