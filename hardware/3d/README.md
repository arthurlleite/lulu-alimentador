# Peças 3D (para a impressora)

Tudo foi modelado no [FreeCAD](https://www.freecad.org/) (gratuito). O arquivo original é o [`IoT3D.FCStd`](IoT3D.FCStd); as peças prontas para fatiar estão em [`stl/`](stl/).

![Renderização da montagem no FreeCAD: tubo transparente com as peças modeladas](imagens/montagem.png)

## Como funciona

A ração fica no tubo de PVC, apoiada na **rampa inclinada**, que tem um furo. Embaixo da rampa, o **servo** (preso no **suporte do motor**) gira a **tampa do atuador**, que tapa e destapa esse furo. Quando abre, a ração escorrega pelo furo, cai no **funil** e sai pela lateral do tubo, pela **extensão do funil**, direto no pote. Cada abertura é uma porção.

![Renderização das peças modeladas fora do tubo; não é fotografia de impressão](imagens/pecas_impressas.png)

## Onde fica a eletrônica

Duas folhas de montagem, geradas a partir das coordenadas do `IoT3D.FCStd`:

| Folha | O que mostra |
|---|---|
| [Posicionamento](imagens/eletronica_posicionamento.svg) | três vistas do tubo com cada componente, os furos e o caminho de cada fio; tabela com medidas, altura e **folga mínima** de cada peça |
| [Furação e placa](imagens/eletronica_furacao.svg) | placa perfurada com cotas, parede do tubo planificada com os furos F1 a F10, mapa de pinos, comprimento dos fios e checklist contra curto |

[![Posicionamento da eletrônica dentro do tubo](imagens/eletronica_posicionamento.png)](imagens/eletronica_posicionamento.svg)

[![Furação do tubo e montagem da placa](imagens/eletronica_furacao.png)](imagens/eletronica_furacao.svg)

### Como foi calculado

- **Medidas com folga**: cada componente usa a maior medida publicada entre as variantes à venda. A ESP32-S3-DevKitC-1 original tem 62,7 × 25,4 mm, mas há clones de até 69 × 28 mm, com as fileiras de pinos a 25,4 mm. O capacitor de 1000 µF / 16 V vem em 10 × 16 a 10 × 20 mm. Conectores encaixados (servo e jumpers) e as porcas do suporte também entram na conta.
- **Folga mínima de 3 mm** entre qualquer componente e o funil, a parede ou outro componente. O script para com erro se alguma peça ficar abaixo disso. A menor folga atual é 3,2 mm (a placa até a parede).
- **Fios**: cada rota é conferida contra o funil, os componentes e o giro da boca do servo. O comprimento leva 30 % de sobra.
- **Furos dos parafusos do suporte** medidos no STL: Ø3,3 em y = 0, a 46,5 e 61,5 mm da base.
- **Cap**: considerado com 40 mm de bolsa; nenhum furo encosta nessa faixa. Meça o seu cap: se a bolsa for maior que 43 mm, o furo F9 fica coberto.

### Resumo da montagem

- **Placa perfurada 7 × 9 cm em pé**, encostada na aba lisa do [suporte da placa](stl/suporte_placa.stl). Dois parafusos M3 × 30 atravessam a parede, o suporte e a placa (F8 e F9), com a porca na face da placa. O topo da placa fica 5 mm abaixo do funil.
- **ESP32-S3 nas barras fêmea, com a USB-C para baixo**: para gravar, tire o cap e ligue o cabo por baixo. As barras ficam nas colunas de furos 1 e 11 (25,4 mm), longe das porcas.
- **Servo e boca**: a boca menor (tampa do atuador) é colada e parafusada no braço do servo e gira em volta do eixo dele, perpendicular à rampa. O cabo sai pelo lado do suporte oposto à boca e fica preso nele com abraçadeira (a mais de 15 mm do giro). Depois passa por F1, desce por fora até F2 e chega à placa com 3 jumpers macho-fêmea de extensão.
- **LED, chave e buzzer** na parede de trás, **sensor de chama do alarme** na parede esquerda, **sensor da saída** colado por fora na extensão do funil, com os fios entrando por F10.
- **Nenhum fio** passa pelo funil, pela saída da ração ou pelo giro da boca.

As medidas são de catálogo: confira as peças recebidas com paquímetro antes de furar.

- **Gerar as folhas**: `python ferramentas/eletronica_3d.py --png` (na raiz do repositório). O mesmo comando refaz a conferência de folgas.
- **Ver no FreeCAD**: abra o `IoT3D.FCStd` e rode o mesmo arquivo em **Macro → Macros… → Arquivo do usuário**. As peças entram no grupo `Eletronica`, com as cores das folhas.

## Peças

| Peça | Arquivo | Tamanho (mm) | Volume | O que faz |
|---|---|---|---|---|
| ![](imagens/rampa_inclinada.png) **Rampa inclinada** | [`rampa_inclinada.stl`](stl/rampa_inclinada.stl) | 97,5 × 97,5 × 80,4 | 18,9 cm³ | Disco inclinado que segura a ração; o furo é a passagem controlada pelo servo |
| ![](imagens/tampa_atuador.png) **Tampa do atuador** | [`tampa_atuador.stl`](stl/tampa_atuador.stl) | 54,8 × 46,0 × 49,0 | 6,9 cm³ | Gira com o eixo do servo e abre/fecha o furo da rampa |
| ![](imagens/suporte_motor.png) **Suporte do motor** | [`suporte_motor.stl`](stl/suporte_motor.stl) | 52,5 × 54,0 × 72,1 | 11,9 cm³ | Prende o servo MG996R (54 mm entre as abas) embaixo da rampa |
| ![](imagens/funil_interno.png) **Funil interno** | [`funil_interno.stl`](stl/funil_interno.stl) | 97,5 × 97,5 × 84,3 | 62,2 cm³ | Recebe a ração e leva até a saída lateral |
| ![](imagens/extensao_funil.png) **Extensão do funil** | [`extensao_funil.stl`](stl/extensao_funil.stl) | 36,0 × 26,5 × 44,9 | 6,7 cm³ | Bico que passa pelo furo do tubo e joga a ração no pote |
| ![](imagens/tampa_tubo.png) **Tampa do tubo** | [`tampa_tubo.stl`](stl/tampa_tubo.stl) | 106,5 × 106,5 × 17,0 | 27,7 cm³ | Tampa de cima, por onde se coloca a ração (encaixa por fora do tubo) |
| ![](imagens/suporte_rampa.png) **Suporte da rampa** | [`suporte_rampa.stl`](stl/suporte_rampa.stl) | 13,4 × 17,4 × 20,0 | 1,0 cm³ | Calço colado no tubo para apoiar a rampa |
| ![](imagens/suporte_placa.png) **Suporte da placa** | [`suporte_placa.stl`](stl/suporte_placa.stl) | 19,6 × 17,0 × 30,0 | 2,9 cm³ | Prende a placa da eletrônica na parede do tubo |

**Total: ~138 cm³ ≈ 100 a 170 g de filamento**, dependendo do preenchimento.

## O que não é impresso

| Peça | Medida | Onde comprar |
|---|---|---|
| Tubo PVC esgoto 100 mm | **300 mm** de comprimento, Ø externo 101,5 mm | barra de 1 m em loja de material de construção ([Hidrauconex](https://www.hidrauconex.com/tubo-cano-pvc-esgoto-branco-de-100mm-barra-6-metros), [Mercado Livre](https://lista.mercadolivre.com.br/tubo-pvc-100-mm-esgoto)) |
| Cap PVC esgoto 100 mm | fecha a base | [exemplo](https://www.hidrauconex.com/cap-tampao-pvc-esgoto-de-100mm) |
| Servo MG996R | 40,7 × 19,7 × 42,9 mm | ver a [lista de materiais](../../documentacao/materiais.md) |

## Configuração de impressão sugerida

| Parâmetro | Valor |
|---|---|
| Material | **PETG** ou PLA para prototipagem; avaliar material, acabamento e limpeza das superfícies do conjunto que terão contato com ração |
| Camada | 0,2 mm |
| Paredes | 3 perímetros |
| Preenchimento | 20 % (giroide); 40 % no suporte do motor e na tampa do atuador |
| Suporte | só no funil interno e na extensão (ângulos de saída) |
| Orientação | rampa e tampa do tubo com a face grande na mesa; funil com a boca larga para baixo |

Fatiadores gratuitos: [PrusaSlicer](https://www.prusa3d.com/page/prusaslicer_424/), [Cura](https://ultimaker.com/software/ultimaker-cura/) ou [OrcaSlicer](https://github.com/SoftFever/OrcaSlicer).

> **Antes de imprimir tudo**, meça o diâmetro **interno** do seu tubo com um paquímetro. O modelo considera ~97,5 mm (parede de ~2 mm). Se o seu tubo for diferente, ajuste o diâmetro das peças internas no FreeCAD ou escale no fatiador.

## Melhorias em relação à primeira versão

Ideias tiradas da montagem real ([vídeo](https://www.youtube.com/watch?v=HcdWtQP4j1U)):

- **Suporte da rampa integrado**: na primeira montagem o suporte foi impresso à parte e colado. O próximo passo é juntar o [`suporte_rampa`](stl/suporte_rampa.stl) na própria rampa e evitar cola.
- **Agitador**: um parafuso longo preso na tampa do atuador mexe a ração a cada abertura e evita que os grãos "façam ponte" sobre o furo.
- **Furos para os sensores novos**: um furo lateral na extensão do funil para o sensor de chama da saída (e um furo em frente, para a luz entrar) e um rasgo por fora para o sensor de chama do alarme.
- **Parafusos em vez de cola** sempre que der, para poder desmontar e limpar.

## Gerar as imagens e os STL de novo

Com o FreeCAD instalado, rode na pasta do repositório:

```bash
freecadcmd ferramentas/exportar_3d.py
```

O script ([`ferramentas/exportar_3d.py`](../../ferramentas/exportar_3d.py)) abre o `IoT3D.FCStd` e regrava os STL desta pasta.

