# 3.5 — Comparativo com cinco outros produtos comerciais

[Voltar ao relatório](README.md) · [Referências](referencias.md)

São comparados o **PETLIBRO Air PLAF108** e **cinco outros produtos** da categoria. O Lulu é a proposta de reprodução e não conta como produto de mercado para atingir esse mínimo. Consulta: **29/10/2026**.

As características são das páginas/manuais citados. **NI** significa não identificado nas fontes consultadas, não ausência do recurso. Revisões de hardware, mercados e aplicativos podem diferir; dados de modelos parecidos não foram transferidos automaticamente.

## Funções, comunicação e energia

| Produto e identificação | Capacidade nominal | Programação / interface | Comunicação documentada | Alimentação / reserva | Fonte |
| --- | --- | --- | --- | --- | --- |
| **PETLIBRO Air — PLAF108** (referência) | 2 L | Até 10 refeições/dia; 1–16 porções; aplicativo PETLIBRO | Wi-Fi 2,4 GHz | Fonte e bateria Li-ion recarregável de 5200 mAh | [F1](referencias.md#f1), [F10](referencias.md#f10) |
| **PETKIT Fresh Element Solo — P570** | 3 L | Agenda pelo app PETKIT; 1–5 porções por refeição; máximo de horários: NI | Wi-Fi 2,4 GHz | USB 5 V; cinco pilhas AAA de reserva | [C1](referencias.md#c1) |
| **Xiaomi Smart Pet Food Feeder — XWPF01MG**, variantes regionais | 3,6 L | Horário e dosagem pelo Xiaomi Home; máximo de horários: NI | Comunicação pelo app; página de especificações não detalha o rádio | Entrada 5,9 V / 1 A; reserva por pilhas alcalinas documentada | [C2](referencias.md#c2), [C3](referencias.md#c3) |
| **Aqara Smart Pet Feeder C1 — PETC1-M01** | 4 L | Agenda e acionamento pelo Aqara Home; calibração de gramas por porção | Zigbee IEEE 802.15.4; hub Aqara para integração remota | USB 5 V / 1 A; pilhas D/LR20 opcionais | [C4](referencias.md#c4), [C5](referencias.md#c5) |
| **PetSafe Smart Feed, 2ª geração — PFD00-16828** | 24 xícaras, unidade mantida do fabricante | Até 12 refeições/dia; 1/8 a 4 xícaras/refeição; app | Wi-Fi | Fonte; quatro pilhas D alcalinas de reserva | [C6](referencias.md#c6), [C7](referencias.md#c7) |
| **PETLIBRO Granary Wi-Fi — PLAF103** | 5 L | Até 10 refeições/dia; 1–50 porções; app da linha PETLIBRO | Wi-Fi 2,4/5 GHz na versão descrita pelo suporte | Conector USB-C; modalidade de reserva deve ser conferida na revisão exata | [C8](referencias.md#c8) |

Não se convertem xícaras em litros sem identificar a convenção volumétrica da fonte. O Aqara C1 não se torna um dispositivo Wi-Fi porque seu hub utiliza Wi-Fi/Ethernet. Estatística em gramas calculada por calibração de uma porção não comprova pesagem contínua do alimento consumido. [C4](referencias.md#c4)

## Comparação pelos doze grupos do enunciado

As fontes de cada coluna identificam o levantamento daquele produto. Para os itens desconhecidos, NI explicita que os documentos citados não permitiram a identificação. Os detalhes e subitens do produto principal estão em [caracteristicas.md](caracteristicas.md).

| Grupo | Air PLAF108 — [F1–F7](referencias.md) | Solo P570 — [C1](referencias.md#c1) | Xiaomi — [C2/C3](referencias.md#c2) | Aqara C1 — [C4/C5](referencias.md#c4) | Smart Feed — [C6/C7](referencias.md#c6) | Granary PLAF103 — [C8](referencias.md#c8) |
| --- | --- | --- | --- | --- | --- | --- |
| 1. Processamento | ESP32-C3 observado em uma unidade; clock efetivo NI [F3](referencias.md#f3) | MCU/clock NI | MCU/clock NI | MCU/clock NI | MCU/clock NI | MCU/clock NI |
| 2. Memória | Recursos do módulo e 4 MB de flash da unidade em [F3/F4](referencias.md#f4) | RAM/flash/EEPROM NI | RAM/flash/EEPROM NI | RAM/flash/EEPROM NI | RAM/flash/EEPROM NI | RAM/flash/EEPROM NI |
| 3. SO | Tipo/versão originais NI | Tipo/versão NI | Tipo/versão NI | Tipo/versão NI | Tipo/versão NI | Tipo/versão NI |
| 4. Sensores/atuadores | Dosador e alertas; modelos de sensores NI | Dispensação motorizada e alertas; modelos NI | Estrutura de seis compartimentos, agitador e lâminas; modelos dos componentes NI | Roda de dispensação; modelos NI | Dispensação e modo lento; modelos NI | Detecção de sobra/obstrução; modelos NI |
| 5. Interfaces com fio | Serial de serviço na placa [F3](referencias.md#f3) | USB de energia; barramentos internos NI | Entrada de energia; barramentos internos NI | USB-A de energia; barramentos internos NI | Fonte; barramentos internos NI | USB-C de energia; barramentos internos NI |
| 6. Interfaces sem fio | Wi-Fi; protocolo de aplicação original NI | Wi-Fi 2,4 GHz; aplicação interna NI | Rádio/protocolo NI neste levantamento | Zigbee; rádio/protocolo entre hub e nuvem NI | Wi-Fi; protocolo de aplicação NI | Wi-Fi 2,4/5 GHz conforme suporte; aplicação interna NI |
| 7. I/O | GPIO/ADC/PWM disponíveis no chip; uso completo NI | GPIO/ADC/DAC/PWM NI | GPIO/ADC/DAC/PWM NI | GPIO/ADC/DAC/PWM NI | GPIO/ADC/DAC/PWM NI | GPIO/ADC/DAC/PWM NI |
| 8. Energia | Fonte/bateria; PMIC e política de sleep NI | USB + AAA; PMIC/sleep NI | Fonte + pilhas; PMIC/sleep NI | USB + D/LR20; PMIC/sleep NI | Fonte + D; PMIC/sleep NI | USB-C; PMIC/sleep NI |
| 9. Firmware/OTA | Firmware original/OTA NI; substituição comunitária distinta | App informa avisos de atualização; mecanismo/assinatura NI | Versão/OTA NI | Versão/OTA NI | Versão/OTA NI | Versão/OTA NI |
| 10. Segurança | Recursos do chip documentados; habilitação NI | Secure Boot/Flash/HSM/TLS NI | Secure Boot/Flash/HSM/TLS NI | Secure Boot/Flash/HSM/TLS NI | Secure Boot/Flash/HSM/TLS NI | Secure Boot/Flash/HSM/TLS NI |
| 11. Armazenamento externo | SD/eMMC e similares NI | SD/eMMC e similares NI | SD/eMMC e similares NI | SD/eMMC e similares NI | SD/eMMC e similares NI | SD/eMMC e similares NI |
| 12. UI | Aplicativo, controles e indicadores | Aplicativo e alimentação manual | Xiaomi Home | Aqara Home; botão e mensagem gravada | Aplicativo; alimentação imediata | Aplicativo; notificações na versão documentada |

## Implicações para a reprodução

- **Continuidade:** os exemplos com reserva de energia mostram uma funcionalidade não reproduzida pelo Lulu atual. O projeto precisa explicitar esse limite, sem prometer autonomia equivalente.
- **Dosagem:** os limites comerciais são expressos em porções/volume e exigem interpretação cuidadosa. No Lulu, a passagem temporizada depende de calibração e ensaio de repetibilidade.
- **Conectividade:** Wi-Fi direto e Zigbee com hub implicam arquiteturas diferentes. O Lulu opta por Wi-Fi com operação HTTP local e integração MQTT remota.
- **Transparência técnica:** a documentação de consumo geralmente descreve capacidade e funções, mas não MCU, memória ou segurança. Identificar esses campos como NI é mais adequado que inferi-los pelo aplicativo.
- **Escopo:** o projeto prioriza agenda, dosador e diagnóstico básico. Câmera, reconhecimento individual, pesagem e integração com assistentes não são requisitos desta reprodução.

As comparações não constituem um teste experimental entre os seis aparelhos. Não foram medidas precisão, confiabilidade, autonomia ou segurança de nenhum deles pela equipe.

