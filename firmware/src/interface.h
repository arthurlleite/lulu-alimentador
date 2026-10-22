#pragma once
// Interface física: LED RGB de status, buzzer ativo e botão.

void interface_iniciar(void);

// Bipes curtos (bloqueia quem chama durante o toque).
void interface_bipar(int vezes, int ms);
