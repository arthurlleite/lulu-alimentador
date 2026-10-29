# Lista de materiais (o que comprar)

Tudo pensado para comprar no Brasil, em lojas de eletrônica, no Mercado Livre e em loja de material de construção. A eletrônica já foi comprada; falta a estrutura.

> **Eletrônica**: preços reais da compra de 26/10/2026. **Estrutura**: preços pesquisados em outubro de 2026; mudam com promoção e frete. Onde está "estimado", não achei um preço publicado e usei a faixa comum de mercado.

## Eletrônica (definida e comprada)

A eletrônica está fechada: é exatamente o que aparece no [esquemático](../hardware/eletronica/README.md). Parte já estava com a equipe (**acervo**) e o resto foi comprado no Mercado Livre em **26/10/2026** e dividido entre os quatro integrantes (**rateio**).

| Ref. | Item | Qtd usada | Origem | Por que este |
|---|---|---|---|---|
| U1 | **ESP32-S3-DevKitC-1 N16R8** (USB-C) | 1 | acervo | Wi-Fi 2,4 GHz + Bluetooth 5 (LE), dois núcleos de 240 MHz, 16 MB de flash e 8 MB de PSRAM. Compatível com ESP-IDF. |
| M1 | **Servo MG996R 180°** (engrenagem metálica) | 1 | acervo | O [suporte do motor](../hardware/3d/README.md) foi desenhado para ele (54 mm entre as abas); torque e travamento com ração ainda precisam de teste sob carga. **180°, não "360°"**: o firmware posiciona o servo por ângulo. |
| U2, U3 | **Módulo sensor de chama** (receptor IR + LM393, saídas A0/D0) | 2 | rateio | Um fica **na saída da ração** e conta os grãos que passam (confirma que a ração caiu). O outro fica **virado para fora** como alarme de fogo. |
| D1 | **LED RGB 5 mm catodo comum** | 1 | rateio | Status (configurando, online, servindo, alarme). |
| R1–R3 | Resistor **220 Ω** 1/4 W | 3 | acervo | Um por cor do LED. |
| C1 | Capacitor eletrolítico **1000 µF / 16 V** | 1 | rateio | Segura o pico do servo e evita a ESP reiniciar. |
| BZ1 | **Buzzer ativo 3,3 V** | 1 | rateio | Bipa para chamar o pet e avisa erro e chama. Ligado direto no GPIO 16. |
| SW1 | **Chave táctil 12 × 12 mm** | 1 | rateio | Alimentar na hora (toque) e apagar o Wi-Fi (5 s). |
| — | **Fonte 5 V / 5 A**, plugue P4 5,5 mm | 1 | acervo | Alimenta ESP32 e servo, com folga para o pico do servo. |
| J1 | Conector **P4 fêmea com borne** | 1 | acervo | Entrada da fonte na placa. |
| — | **Placa perfurada 7 × 9 cm** + barras de pinos macho e fêmea | 1 | acervo | Montagem fixa, sem protoboard dentro do alimentador. |
| — | Jumpers, fio de silicone 20 AWG, cabo USB-A/USB-C | — | acervo | Ligações e gravação do firmware. |

O HC-SR04 (nível) e o DHT11 (temperatura e umidade) **saíram do projeto**: não fazem parte das peças da equipe. O firmware continua sabendo usá-los, mas eles vêm desligados no `menuconfig` (`ALIM_SENSOR_NIVEL` e `ALIM_DHT11`).

### Itens pequenos para a montagem

Pedidos pela montagem e pelas proteções contra curto (ver as [folhas de montagem](../hardware/3d/README.md#onde-fica-a-eletrônica)). Não estão na nota; use o que houver no acervo.

| Item | Qtd | Para quê |
|---|---|---|
| Parafuso M3 × 30 + porca + arruela isolante | 2 | prender o suporte e a placa (furos F8 e F9) |
| Termo-retrátil fino (2 a 3 mm) | ~30 cm | pernas do LED, buzzer e chave, bornes do J1 |
| Abraçadeiras pequenas (nylon) | ~10 | cabo do servo no suporte, J1 e feixes de fios |
| Fita dupla-face de espuma | ~10 cm | colar U2, U3 e a chave isolando o verso |
| Fita isolante | 1 | verso da placa |

### Rateio da compra

Itens da nota usados no Lulu que não estavam no acervo. As outras peças da mesma compra foram reposição do que a equipe já tinha e não entram na conta.

| Item comprado | Embalagem | Preço |
|---|---|---|
| Buzzer ativo 3,3 V | 5 unidades | R$ 18,90 |
| Capacitor eletrolítico 1000 µF / 16 V | 20 unidades | R$ 20,00 |
| Chave táctil 12 × 12 mm | kit | R$ 19,99 |
| Módulo sensor de chama | 2 unidades | R$ 17,99 |
| LED RGB 5 mm catodo comum | 10 unidades | R$ 22,48 |
| **Total** | | **R$ 99,36** |
| **Por integrante (÷ 4)** | | **R$ 24,84** |

## Estrutura

| # | Item | Qtd | Preço aprox. | Observação |
|---|---|---|---|---|
| 16 | **Tubo PVC esgoto 100 mm**, barra de 1 m | 1 | R$ 17 – 25 | O modelo 3D usa **300 mm** de tubo (diâmetro externo 101,5 mm, o padrão do esgoto de 100 mm). Sobra tubo para um segundo protótipo. Série normal, branco. |
| 17 | **Cap (tampão) PVC esgoto 100 mm** | 1 (+1 opcional) | R$ 12 – 25 cada | Fecha a base. A tampa de cima já é impressa ([`tampa_tubo.stl`](../hardware/3d/stl/tampa_tubo.stl)); um segundo cap só se preferir tampa de PVC. |
| 18 | Filamento **PETG** (ou PLA) | ~150 g | R$ 15 – 25 (de um rolo de 1 kg, estimado) | As 8 peças somam ~138 cm³ geométricos. Confirmar consumo no fatiador e avaliar material, acabamento e limpeza nas superfícies de contato com ração. |
| 19 | Parafusos M3 × 10 mm + porcas, e um parafuso longo (~M5 × 50 mm) | kit | R$ 10 – 15 (estimado) | O parafuso longo vira o "agitador" que mexe a ração. |
| 20 | Cola epóxi ou Super Bonder | 1 | R$ 10 – 20 (estimado) | Fixar o braço do servo na peça giratória. |
| 21 | Tinta spray branca (opcional) | 1 | R$ 20 – 30 (estimado) | Acabamento **só por fora**. |

**Subtotal da estrutura: ~R$ 65 – 140**

## Total

- **Eletrônica**: R$ 99,36 já gastos (R$ 24,84 por integrante). O resto saiu do acervo.
- **Estrutura**: ainda a comprar, **~R$ 65 – 140** (estimativa acima).

## Ferramentas

Ferro de solda e estanho, mini retífica (ou serra copo de 25–35 mm) para abrir a saída no tubo, serra para cortar o PVC, chave Phillips, lixa e multímetro.

## Onde os preços foram vistos

- ESP32-S3 N16R8: [Curto Circuito](https://curtocircuito.com.br/placa-devkitc-1-esp32-s3-n16r8.html) (R$ 94,30 na promoção), [SmartKits](https://www.smartkits.com.br/placa-esp32-s3-devkitc-n16r8-com-conector-ipex) (R$ 85,41 à vista), [Mercado Livre](https://www.mercadolivre.com.br/placa-devkitc-1-esp32-s3-n16r8-16mb-flash-8mb-psram-wifi-tipo-c-duplo/p/MLB49198475), [Usina Info](https://www.usinainfo.com.br/esp32/esp32-s3-devkitc-1-n16r8-16mb-flash-com-wifi-e-bluetooth-com-usb-c-9361.html)
- Servo MG996R: [Curto Circuito](https://curtocircuito.com.br/servo-motor-mg996r.html), [Mercado Livre](https://lista.mercadolivre.com.br/servo-mg996r)
- Tubo PVC 100 mm: [Hidrauconex, barra de 6 m](https://www.hidrauconex.com/tubo-cano-pvc-esgoto-branco-de-100mm-barra-6-metros) (R$ 98,10 à vista, ~R$ 16/m), [Mercado Livre](https://lista.mercadolivre.com.br/tubo-pvc-100-mm-esgoto)
- Cap PVC 100 mm: [Hidrauconex](https://www.hidrauconex.com/cap-tampao-pvc-esgoto-de-100mm) (R$ 14,10), [Mercado Livre](https://lista.mercadolivre.com.br/tampao-cap-pvc-100-mm)

## Por que a ESP32-S3 e não outra

| Placa | Wi-Fi | Prós | Contras |
|---|---|---|---|
| **ESP32-S3-DevKitC-1** ✅ | 2,4 GHz b/g/n | atual, muitos pinos livres, USB nativo, aceleração de IA (vetores) para evoluir o projeto, bem suportada no ESP-IDF | um pouco mais cara |
| ESP32 DevKit V1 (clássica) | 2,4 GHz b/g/n | mais barata (~R$ 40), muito material na internet | projeto de 2016; o firmware também roda nela (`-e esp32`) |
| ESP32-C3 | 2,4 GHz b/g/n | barata, RISC-V (a mesma família do PETLIBRO Air) | poucos pinos |
| ESP32-C6 | Wi-Fi 6 2,4 GHz | Wi-Fi 6, Thread/Zigbee | suporte mais novo; ganho pequeno para este projeto |

Nenhuma ESP32 se conecta em Wi-Fi de **5 GHz**: o roteador precisa ter a rede de 2,4 GHz ligada.

