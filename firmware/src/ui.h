#pragma once
// LED RGB de status, buzzer ativo e botão físico.

void ui_init(void);

// Bipes curtos (bloqueia quem chama durante o toque).
void ui_beep(int count, int ms);
