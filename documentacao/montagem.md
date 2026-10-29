# Montagem passo a passo

Baseado na montagem real do primeiro protótipo, mostrada [neste vídeo](https://www.youtube.com/watch?v=HcdWtQP4j1U), com as melhorias desta versão. Antes de começar, tenha tudo da [lista de materiais](materiais.md) e as peças impressas ([`hardware/3d`](../hardware/3d/README.md)).

![Montagem no FreeCAD](../hardware/3d/imagens/montagem.png)

## 1. Tubo de PVC

1. Corte **300 mm** do tubo de esgoto de 100 mm. Lixe as bordas.
2. Faça o **furo de saída** na lateral, na altura da boca do funil: marque com a [extensão do funil](../hardware/3d/stl/extensao_funil.stl), fure em volta com a mini retífica e abra até a peça passar justa.
3. Faça um furo pequeno (~8 mm) para os fios do servo e dos sensores, perto da placa.

## 2. Servo e tampa do atuador

1. Teste o servo **antes** de colar qualquer coisa: grave o firmware, ligue e toque em **Alimentar agora** no app. Ele deve ir de 0° a 180° e voltar.
2. Com o servo na posição **fechada** (0°), cole o braço plástico do servo na [tampa do atuador](../hardware/3d/stl/tampa_atuador.stl) com epóxi ou Super Bonder.
3. Prenda o servo no [suporte do motor](../hardware/3d/stl/suporte_motor.stl) com os parafusos que vêm com ele.
4. *(Opcional, recomendado)* parafuse um parafuso longo na tampa do atuador apontando para cima. Ele mexe a ração a cada abertura e evita que os grãos travem sobre o furo (foi a solução usada no protótipo).

## 3. Rampa e funil

1. Encaixe o [funil interno](../hardware/3d/stl/funil_interno.stl) no tubo, com a boca alinhada ao furo de saída, e passe a [extensão do funil](../hardware/3d/stl/extensao_funil.stl) por fora.
2. Monte o suporte do motor com o servo embaixo da [rampa inclinada](../hardware/3d/stl/rampa_inclinada.stl), com a tampa do atuador cobrindo o furo da rampa.
3. Cole o [suporte da rampa](../hardware/3d/stl/suporte_rampa.stl) na parede do tubo e apoie a rampa nele.
4. Gire a mão: abrindo a tampa do atuador, a ração deve escorregar pelo furo e cair no funil.

## 4. Eletrônica

Siga as duas folhas de montagem ([posicionamento](../hardware/3d/imagens/eletronica_posicionamento.svg) e [furação e placa](../hardware/3d/imagens/eletronica_furacao.svg)). Elas têm a altura e a posição de cada furo, as cotas da placa e o mapa de pinos.

1. **Marque os furos**: enrole uma fita de papel no tubo, marque o centro da saída da ração e meça cada furo pela distância **s** (pela volta) e pela altura a partir da base (tabela da folha de furação). Fure F1 a F10 e os furos E1 e E2 na extensão do funil.
2. **Prepare a placa**: fure Ø3,3 nos dois pontos dos parafusos (a = 35, b = 44,5 e 59,5), raspe as ilhas de cobre no círculo de Ø10 em volta de cada furo e solde barras, resistores, C1 (faixa no GND) e os trilhos de +5 V e GND, conforme o [esquemático](../hardware/eletronica/README.md).
3. **Isole o verso da placa**: corte os terminais a no máximo 2 mm e cubra com fita isolante.
4. **Fixe a placa** no suporte com 2 parafusos M3 × 30 (arruela isolante do lado da placa) e encaixe a ESP32 com a USB-C para baixo.
5. **Servo**: prenda o cabo no suporte do motor com abraçadeira, do lado oposto à boca. Passe-o por F1, desça por fora até F2 e ligue-o à barra 1×3 com 3 jumpers macho-fêmea (marrom no GND). Fixe a emenda com fita.
6. **Sensores e painel**: U2 colado por fora na extensão (fotodiodo em E1, fios por F10), U3 na parede esquerda (fotodiodo dobrado para fora por F7), LED em F4, chave colada por fora sobre F5, buzzer atrás de F6. Use fita dupla-face de espuma e termo-retrátil em cada perna soldada.
7. **J1**: aperte os fios de 20 AWG nos bornes, cubra com termo-retrátil e prenda o J1 com abraçadeira; o plugue da fonte entra por F3.
8. **Antes de ligar**: com o multímetro, confira que **+5 V e GND não têm continuidade**. Só então ligue a fonte.

## 5. Primeira configuração

1. Grave o firmware pelo [instalador no navegador](https://arthurlleite.github.io/lulu-alimentador/instalar.html).
2. Conecte o celular na rede `Alimentador-XXXX`, informe o nome do pet e o Wi-Fi da casa.
3. **Calibre a porção**: sirva 5 porções num pote vazio, pese e divida por 5. Para mudar a quantidade por porção, ajuste `ALIM_TEMPO_ABERTO_MS` no `menuconfig` (padrão 800 ms).
4. Monte a agenda no app.

## 6. Acabamento

- Feche a base com o cap de PVC.
- Se for pintar, pinte **só por fora** (o protótipo foi pintado de branco).
- Mantenha a tampa fechada: a umidade estraga a ração.

## Problemas comuns

| Sintoma | Causa provável | O que fazer |
|---|---|---|
| A ESP reinicia quando o servo mexe | queda de tensão | fonte de 5 A, capacitor de 1000 µF perto do servo, fios curtos |
| Servo treme parado | ruído no sinal | o firmware corta o PWM depois de servir; confira o GND comum |
| App avisa "nada caiu" | ração travada ou sensor da saída desalinhado | sacuda o tubo; ajuste o trimpot do sensor da saída |
| Alarme de chama sem fogo | sol ou lâmpada incandescente no sensor | vire o sensor ou gire o trimpot para menos sensibilidade |
| LED azul piscando sempre | senha do Wi-Fi errada ou rede 5 GHz | refaça a configuração com uma rede de 2,4 GHz |
