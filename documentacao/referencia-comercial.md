# Produto de referência: PETLIBRO Air (PLAF108)

O projeto usa como base um produto comercial, como pede o [Trabalho 2 de Fundamentos de Sistemas Embarcados (2026/2)](https://gitlab.com/fse_fga/trabalhos-2026_2/trabalho-2-2026-2): estudar um sistema embarcado de mercado e propor uma reprodução com ESP32 e os sensores da [lista da disciplina](https://gitlab.com/fse_fga/trabalhos-2026_2/trabalho-2-2026-2/-/blob/main/Lista_de_Sensores.md).

O escolhido é o **[PETLIBRO Air Smart Feeder](https://au.petlibro.com/products/air-wifi-feeder)** (modelo PLAF108).

## Por que ele

1. **Usa um chip da Espressif por dentro**: um módulo [ESP32-C3-WROOM-02](https://www.espressif.com/sites/default/files/documentation/esp32-c3-wroom-02_datasheet_en.pdf), da mesma família da ESP32 que vamos usar ([teardown da Pinecone Data](https://www.pineconedata.com/2026-06-19-flashing-esphome-on-wifi-pet-feeder/)).
2. **Tem documentação aberta**: há teardown publicado e firmwares de código aberto ([taylorfinnell/petlibro-esphome](https://github.com/taylorfinnell/petlibro-esphome) e [pineconedata/esphome-petlibro-plaf108](https://github.com/pineconedata/esphome-petlibro-plaf108)) que mostram em qual pino está cada sensor e atuador. O próprio enunciado recomenda produtos com documentação mais aberta.
3. **As funções batem com as nossas**: agenda de até 10 refeições, porções por refeição, alerta de ração baixa, detecção de ração travada, app pelo celular e Wi-Fi 2,4 GHz.

## Especificações

| Característica | PETLIBRO Air | Fonte |
|---|---|---|
| Capacidade | 2 L | [página oficial](https://au.petlibro.com/products/air-wifi-feeder) |
| Refeições | até 10 por dia, 1 a 16 porções cada | [página oficial](https://au.petlibro.com/products/air-wifi-feeder) |
| Porção | 1/24 de xícara (10,4 ml); refeição máxima ~80 g | [página oficial](https://au.petlibro.com/products/air-wifi-feeder) |
| Ração aceita | grãos de 2 a 15 mm | [página oficial](https://au.petlibro.com/products/air-wifi-feeder) |
| Energia | fonte + bateria de lítio recarregável de 5200 mAh (até 30 dias) | [página oficial](https://au.petlibro.com/products/air-wifi-feeder) |
| Conectividade | Wi-Fi 2,4 GHz, app PETLIBRO (Android/iOS) | [página oficial](https://au.petlibro.com/products/air-wifi-feeder) |
| Alertas | ração baixa, bateria baixa, ração travada ("STUCK") | [página oficial](https://au.petlibro.com/products/air-wifi-feeder), [central de ajuda](https://petlibro.com/pages/how-the-air-wifi-feeder-works-plaf108) |
| Dimensões e peso | 310 × 265 × 180 mm, 1,3 kg | [página oficial](https://au.petlibro.com/products/air-wifi-feeder) |
| Material | aço inox (pote) | [página oficial](https://au.petlibro.com/products/air-wifi-feeder) |
| Microcontrolador | ESP32-C3-WROOM-02 (RISC-V, 1 núcleo), cristal de 40 MHz, flash de 4 MB | [teardown](https://www.pineconedata.com/2026-06-19-flashing-esphome-on-wifi-pet-feeder/), [datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3-wroom-02_datasheet_en.pdf) |

## O que tem dentro (pinos)

Levantado da configuração aberta [`.common.plaf108.yaml`](https://github.com/taylorfinnell/petlibro-esphome/blob/master/plaf108/.common.plaf108.yaml):

| GPIO do ESP32-C3 | Função no PETLIBRO |
|---|---|
| GPIO 6 / GPIO 7 | motor para a direita / esquerda (ponte H) |
| GPIO 8 | sensor de giro do motor (pulso a cada 1/4 de volta = uma porção) |
| GPIO 2 | chave de posição do motor (é um pino de *strapping*: atrapalha a gravação se estiver fechada) |
| GPIO 18 | sensor de ração na saída (IR) |
| GPIO 9 | liga o sensor de ração |
| GPIO 10 | LED de alarme |
| GPIO 19 | botão de reset |
| GPIO 0 / GPIO 4 / GPIO 5 | bateria, bateria conectada, fonte conectada |

## Do PETLIBRO para o nosso alimentador

| Função | PETLIBRO Air | Lulu (este projeto) |
|---|---|---|
| Processador | ESP32-C3 (RISC-V, 1 núcleo, 160 MHz) | [ESP32-S3](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf) (Xtensa LX7, 2 núcleos, 240 MHz) |
| Sistema | firmware proprietário (os projetos abertos usam ESPHome) | ESP-IDF 5.3 + FreeRTOS |
| Reservatório | 2 L | tubo PVC 100 mm × 300 mm (~1 L acima da rampa; cresce com um tubo mais longo) |
| Dosagem | motor DC + rotor, conta 1/4 de volta por porção | servo MG996R abre/fecha uma passagem por porção |
| Confirma que a ração saiu | sensor IR na saída | sensor de chama (receptor IR) na saída, contando os grãos |
| Nível de ração | alerta de ração baixa | não tem: o app avisa quando o servo abre e nada cai |
| Segurança | não tem | segundo sensor de chama como alarme de fogo |
| Alarme | LED vermelho + notificação | LED RGB, buzzer e notificação no app |
| Agenda | até 10 refeições | até 8 refeições com porções próprias |
| App | app PETLIBRO (nuvem do fabricante) | app web: rede local (HTTP) + internet (MQTT/TLS) |
| Configuração do Wi-Fi | pelo app PETLIBRO | portal cativo: rede `Alimentador-XXXX`, sem instalar app |
| Energia | fonte + bateria de 5200 mAh | fonte 5 V 5 A (sem bateria nesta versão) |

## O que fica simplificado

- **Sem bateria de reserva**: se faltar energia, o alimentador para. Um próximo passo é um módulo carregador TP4056 com uma célula 18650 e um conversor elevador para 5 V.
- **Servo no lugar de motor DC com rotor**: mais simples de controlar (não precisa de ponte H nem de sensor de giro), mas a porção depende do tempo aberto, não de um volume fixo. Calibre pesando a ração.
- **Capacidade menor**: o tubo de 300 mm guarda cerca de 1 L acima da rampa. Para chegar aos 2 L do PETLIBRO, basta um tubo de ~450 mm.

## Outros produtos para o comparativo

O Trabalho 2 pede pelo menos 5 produtos relacionados. Pontos de partida com fonte:

| Produto | O que já se sabe | Fonte |
|---|---|---|
| [PetKit Fresh Element Solo](https://petkit.com/products/fresh-element-solo) | ESP32-WROOM-32D; motor com sensor de giro e sensor óptico de ração | [ESPHome Devices](https://devices.esphome.io/devices/petkit-fresh-element-solo-pet-feeder/) |
| [Xiaomi Smart Pet Food Feeder](https://www.mi.com/global/product/xiaomi-smart-pet-food-feeder/) (XWPF01MG) | entrada 5,9 V / 1 A, 4 pilhas AA de reserva, porta giratória, grãos de 5–12 mm | [página oficial](https://www.mi.com/global/product/xiaomi-smart-pet-food-feeder/), [manual](https://www.manualslib.com/manual/2914848/Xiaomi-Xwpf01mg-Eu.html) |
| Aqara Pet Feeder C1 | Zigbee 3.0, funciona localmente | [Local Smart Home Guide](https://localsmarthomeguide.com/articles/local-only-pet-feeder-options-for-home-assistant/) |
| Alimentadores genéricos Tuya | módulo Wi-Fi que fala com outro microcontrolador por serial | [fórum do Home Assistant](https://community.home-assistant.io/t/home-assistant-enabled-cat-feeder-with-an-esp32/951605) |

Os itens ainda sem fonte (FCC ID, Anatel, memória, segurança etc.) precisam ser levantados pelo grupo antes de entrar no relatório.
