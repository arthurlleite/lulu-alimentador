# Lista de materiais (o que comprar)

Tudo pensado para comprar no Brasil, em lojas de eletrônica (Curto Circuito, Eletrogate, Usina Info, Baú da Eletrônica…), no Mercado Livre e em loja de material de construção. Os sensores são os mesmos da lista de sensores da disciplina de FSE, então dá para trocar pelos do laboratório se faltar algum.

> **Preços pesquisados em outubro de 2026.** Mudam com promoção e frete, então confira antes de comprar. Onde está "estimado", não achei um preço publicado e usei a faixa comum de mercado.

## Eletrônica

| # | Item | Qtd | Preço aprox. | Por que este |
|---|---|---|---|---|
| 1 | **ESP32-S3-DevKitC-1 N16R8** (USB-C) | 1 | ✅ **já possui** | ESP32 atual, com Wi-Fi 2,4 GHz + Bluetooth 5 (LE), dois núcleos de 240 MHz, 16 MB de flash e 8 MB de PSRAM. Compatível com ESP-IDF. A versão N8R2 também serve. |
| 2 | **Servo MG996R 180°** (engrenagem metálica) | 1 | ✅ **já possui** | O [suporte do motor](../hardware/3d/README.md) foi desenhado para ele. Conferir as medidas da unidade; torque disponível e travamento com ração precisam de teste sob carga. **Compre o de 180°, não o "360°"**: o firmware posiciona o servo por ângulo. |
| 3 | **Sensor ultrassônico HC-SR04** | 1 | R$ 8 – 14 | Opcional: mede o nível de ração no tubo e avisa quando está acabando. Se não quiser usar, desligue `ALIM_SENSOR_NIVEL` no menuconfig. |
| 4 | **Módulo DHT11** (3 pinos, com resistor) | 1 | R$ 10 – 15 (estimado) | Temperatura e umidade dentro do reservatório. |
| 5 | **Sensor IR de obstáculo** (FC-51 ou "bloqueio de luz") | 1 | R$ 5 – 10 (estimado) | Confirma que a ração caiu, como o sensor de saída do PETLIBRO. |
| 6 | **Módulo buzzer ativo** | 1 | R$ 4 – 8 (estimado) | Bipa para chamar o pet e avisa erro. |
| 7 | **LED RGB 5 mm catodo comum** + 3 resistores 220 Ω | 1 | R$ 2 – 5 (estimado) | Status (configurando, online, servindo, alarme). |
| 8 | **Botão táctil 12 mm** (ou de painel) | 1 | R$ 2 – 5 (estimado) | Alimentar na hora e apagar o Wi-Fi. |
| 9 | **Fonte 5 V 3 A** (plug P4 ou USB-C) | 1 | ✅ **já possui** | Alimenta ESP32 e servo. 2 A é o mínimo; 3 A dá folga para o pico do servo. |
| 10 | Conector P4 fêmea (ou módulo USB-C de alimentação) | 1 | R$ 3 – 8 (estimado) | Entrada da fonte. |
| 11 | Capacitor eletrolítico **1000 µF / 10 V** ou mais | 1 | R$ 2 – 4 (estimado) | Segura o pico do servo e evita a ESP reiniciar. |
| 12 | Resistores **1 kΩ** e **2 kΩ** | 1 de cada | R$ 1 (estimado) | Divisor do ECHO (5 V → 3,3 V). |
| 13 | Placa perfurada 5×7 cm + barras de pinos macho/fêmea | 1 | R$ 8 – 15 (estimado) | Montagem fixa, sem protoboard dentro do alimentador. |
| 14 | Jumpers fêmea-fêmea e fio 22 AWG | 1 kit | R$ 10 – 15 (estimado) | Ligações. |
| 15 | Cabo USB-C **de dados** | 1 | geralmente já tem | Para gravar o firmware. |

**Subtotal da eletrônica a comprar: ~R$ 55 – 100** (sem ESP32-S3, servo e fonte, que já temos)

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

## Total a comprar: **R$ 120 – 240**

ESP32-S3, servo MG996R e fonte 5 V já estão com a equipe e não entram na conta. O valor maior inclui cola e tinta, que talvez já tenham também.

## Ferramentas

Ferro de solda e estanho, mini retífica (ou serra copo de 25–35 mm) para abrir a saída no tubo, serra para cortar o PVC, chave Phillips, lixa e multímetro.

## Onde os preços foram vistos

- ESP32-S3 N16R8: [Curto Circuito](https://curtocircuito.com.br/placa-devkitc-1-esp32-s3-n16r8.html) (R$ 94,30 na promoção), [SmartKits](https://www.smartkits.com.br/placa-esp32-s3-devkitc-n16r8-com-conector-ipex) (R$ 85,41 à vista), [Mercado Livre](https://www.mercadolivre.com.br/placa-devkitc-1-esp32-s3-n16r8-16mb-flash-8mb-psram-wifi-tipo-c-duplo/p/MLB49198475), [Usina Info](https://www.usinainfo.com.br/esp32/esp32-s3-devkitc-1-n16r8-16mb-flash-com-wifi-e-bluetooth-com-usb-c-9361.html)
- Servo MG996R: [Curto Circuito](https://curtocircuito.com.br/servo-motor-mg996r.html), [Mercado Livre](https://lista.mercadolivre.com.br/servo-mg996r)
- HC-SR04: [Eletrogate](https://www.eletrogate.com/modulo-sensor-de-distancia-ultrassonico-hc-sr04), [Baú da Eletrônica](https://www.baudaeletronica.com.br/produto/sensor-de-distancia-ultrassonico-hc-sr04.html), [Curto Circuito](https://curtocircuito.com.br/sensor-ultrassonico-hc-sr04.html)
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

