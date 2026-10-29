# 3.2 — Características técnicas do PETLIBRO Air PLAF108

[Voltar ao relatório](README.md) · [Referências completas](referencias.md)

**Legenda:** **observado** identifica informação do produto ou de uma unidade desmontada; **recurso do chip** não comprova utilização pelo firmware original; **NI** significa não identificado nas fontes citadas. NI não significa ausência. As fontes consultadas para itens NI são indicadas para delimitar a pesquisa.

## 1. Unidade de processamento

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 1.1 Fabricante e modelo | Espressif; módulo ESP32-C3-WROOM-02 observado na unidade desmontada. | [F3](referencias.md#f3) |
| 1.2 Arquitetura e núcleos | Recurso do módulo: RISC-V de 32 bits, um núcleo. | [F4](referencias.md#f4) |
| 1.3 Frequência | CPU até 160 MHz; frequência efetivamente configurada no produto: NI. O cristal de 40 MHz é uma referência de clock, não a frequência comprovada de execução da CPU. | [F4](referencias.md#f4), [F3](referencias.md#f3) |
| 1.4 Unidades dedicadas | Aceleradores criptográficos disponíveis no SoC. Uso no produto e presença de outros coprocessadores externos: NI. | [F5](referencias.md#f5), [F3](referencias.md#f3) |

## 2. Memória

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 2.1 RAM | Módulo com 400 KB de SRAM, incluindo 16 KB destinados a cache, e 8 KB de SRAM RTC. Ocupação pelo firmware original: NI. | [F4](referencias.md#f4) |
| 2.2 ROM / Flash | 384 KB de ROM no chip. A desmontagem relata leitura de 4 MB de flash; trata-se de SPI flash no módulo, externa ao die do SoC. A família do módulo admite outras capacidades. | [F4](referencias.md#f4), [F3](referencias.md#f3) |
| 2.3 EEPROM | EEPROM independente ou sua utilização no produto: NI. Flash e EEPROM não são tratadas como equivalentes sem evidência de implementação. | [F3](referencias.md#f3), [F4](referencias.md#f4) |

## 3. Sistema operacional

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 3.1 Tipo | NI no firmware original; não é possível classificá-lo como RTOS, GPOS ou bare-metal apenas pelo modelo do chip. | [F3](referencias.md#f3) |
| 3.2 Versão | NI. As versões de ESPHome usadas na substituição comunitária não identificam o sistema originalmente instalado. | [F3](referencias.md#f3) |

## 4. Sensores e atuadores

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 4.1 Sensores | O fabricante documenta alertas de ração baixa/obstrução. A desmontagem relata sinais de alimentação, saída de ração e rotação do motor. Modelos e fabricantes dos sensores individuais: NI. | [F2](referencias.md#f2), [F3](referencias.md#f3) |
| 4.2 Atuadores | Mecanismo motorizado de dispensação e indicadores luminosos. Modelo, corrente e torque do motor: NI. | [F1](referencias.md#f1), [F2](referencias.md#f2), [F3](referencias.md#f3) |
| 4.3 Conexão à MCU | Associação completa entre sensores, drivers, barramentos e GPIOs da placa original: NI nesta revisão. Não se atribuem os pinos do Lulu ao produto comercial. | [F3](referencias.md#f3) |

## 5. Interfaces de comunicação com fio

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 5.1 Interfaces | TX/RX/GND/VCC são documentados na placa para acesso serial. O módulo oferece UART, SPI, I2C, I2S e USB Serial/JTAG; o uso de cada uma na placa comercial é NI. Porta/conector de energia não comprova uma interface USB de dados. | [F3](referencias.md#f3), [F4](referencias.md#f4) |

## 6. Interfaces sem fio

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 6.1 Tecnologias | Wi-Fi de 2,4 GHz documentado para o produto. Bluetooth LE 5 é recurso do módulo; seu uso pelo firmware original é NI. Thread, Matter, Zigbee, Z-Wave, LoRaWAN, redes celulares, UWB e NFC: não identificados nas fontes. | [F10](referencias.md#f10), [F4](referencias.md#f4) |
| 6.2 Rádio / aplicação | Rádio integrado ao ESP32-C3 na unidade observada. Aplicativo PETLIBRO; protocolo de aplicação do firmware original (MQTT, HTTP, CoAP etc.): NI. | [F3](referencias.md#f3), [F10](referencias.md#f10) |

## 7. Entrada e saída

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 7.1 GPIO | Até 15 GPIOs expostos pelo módulo; ocupação exata na placa comercial: NI. | [F4](referencias.md#f4), [F3](referencias.md#f3) |
| 7.2 ADC / DAC | O SoC dispõe de ADC SAR de 12 bits; não se identificou DAC analógico dedicado no datasheet. Uso do ADC no produto e condicionamento dos sinais: NI. | [F5](referencias.md#f5), [F6](referencias.md#f6), [F3](referencias.md#f3) |
| 7.3 PWM | Controlador LED PWM disponível no módulo; frequência, resolução e utilização pelo firmware original: NI. | [F4](referencias.md#f4), [F3](referencias.md#f3) |

## 8. Energia e alimentação

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 8.1 Fonte | Alimentação externa e bateria recarregável de íons de lítio de 5200 mAh. Autonomia de até 30 dias é declaração do fabricante, não medida pela equipe. | [F1](referencias.md#f1), [F11](referencias.md#f11) |
| 8.2 Gerenciamento / PMIC | Há circuito de alimentação na placa; modelo do carregador, reguladores e comutação entre fontes: NI. | [F3](referencias.md#f3) |
| 8.3 Economia | O chip oferece modos de baixo consumo; estratégia e consumo real do firmware original: NI. | [F5](referencias.md#f5), [F3](referencias.md#f3) |
| 8.4 Backup | Bateria integrada documentada; detalhes de persistência do estado durante quedas e eventual backup dedicado de RTC: NI. | [F1](referencias.md#f1), [F3](referencias.md#f3) |
| 8.5 Reset / watchdog | Reset de configuração é documentado; o chip dispõe de watchdogs. Ativação, limites e política do firmware original: NI. | [F2](referencias.md#f2), [F5](referencias.md#f5) |
| 8.6 Brown-out | Proteção disponível no SoC; habilitação, limiar e supervisor externo no produto: NI. | [F5](referencias.md#f5), [F3](referencias.md#f3) |

## 9. Firmware e atualizações

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 9.1 Firmware | Software original integrado ao aplicativo/serviço PETLIBRO; código-fonte e versão da unidade estudada: NI. Existe substituição comunitária, tratada como software distinto. | [F3](referencias.md#f3), [F10](referencias.md#f10) |
| 9.2 OTA | A substituição comunitária admite OTA; não se confirmou mecanismo, assinatura ou política OTA do firmware original. | [F3](referencias.md#f3) |

## 10. Proteção e segurança

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 10.1 Criptografia | AES, SHA, RSA e HMAC disponíveis no SoC. Algoritmos e configuração efetivamente usados: NI. | [F5](referencias.md#f5), [F7](referencias.md#f7) |
| 10.2 HSM / Secure Element | Não se identificou componente externo dedicado. Os recursos internos de assinatura/chaves não comprovam um Secure Element separado. | [F3](referencias.md#f3), [F7](referencias.md#f7) |
| 10.3 Secure Boot / Flash Encryption | Recursos disponíveis na plataforma; estado dos eFuses e habilitação na unidade comercial: NI. | [F7](referencias.md#f7), [F3](referencias.md#f3) |
| 10.4 Acesso / transporte | Uso de aplicativo e conta do serviço documentado; TLS, autenticação de dispositivos, autorização e VPN no protocolo original: NI. | [F3](referencias.md#f3), [F10](referencias.md#f10) |

## 11. Armazenamento externo

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 11.1 SD, NAND, eMMC, HDD/SSD | Não identificados nas fontes do produto. A SPI flash do módulo já foi descrita em 2.2; não se afirma existência ou ausência de armazenamento adicional sem levantamento da placa. | [F3](referencias.md#f3), [F4](referencias.md#f4) |

## 12. Interface com o usuário

| Item | Característica e limite da evidência | Fonte |
| --- | --- | --- |
| 12.1 Displays / indicadores | Indicadores luminosos e aplicativo; display alfanumérico, OLED ou LCD no alimentador: NI nas fontes consultadas. | [F1](referencias.md#f1), [F11](referencias.md#f11) |
| 12.2 Botões / toque | Controles físicos e reset documentados. Tecnologia de toque capacitivo: NI. | [F2](referencias.md#f2), [F11](referencias.md#f11) |
| 12.3 Outras interfaces | Aplicativo móvel e notificações. Interface web local, comandos de voz e outras interfaces nativas do PLAF108: NI. | [F1](referencias.md#f1), [F2](referencias.md#f2) |

## Tecnologias críticas

O funcionamento depende do **controle embarcado**, do **mecanismo de dispensação**, da **referência de tempo e retenção dos horários** e da **alimentação elétrica**. Para a experiência conectada, também são críticos **Wi-Fi, aplicativo e serviço remoto**. A detecção de falhas melhora a observabilidade da dispensação. A tecnologia de segurança exata do produto permanece desconhecida; isso não reduz sua importância para impedir comandos indevidos. [F1–F3](referencias.md), [artigos T1, T2 e T4](artigos.md)

### Informações ainda não identificadas

Não foram confirmados SO/versão, clock efetivo, modelo dos sensores e motor, circuito de PMIC, protocolo de aplicação, política OTA e configuração de segurança. Essas lacunas ficam explicitadas em vez de preenchidas com suposições. Uma ampliação futura da pesquisa pode utilizar registros de certificação da revisão exata, documentação adicional ou uma desmontagem própria.

