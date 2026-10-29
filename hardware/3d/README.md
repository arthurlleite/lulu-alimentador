# Peças 3D (para a impressora)

Tudo foi modelado no [FreeCAD](https://www.freecad.org/) (gratuito). O arquivo original é o [`IoT3D.FCStd`](IoT3D.FCStd); as peças prontas para fatiar estão em [`stl/`](stl/).

![Montagem: tubo de PVC transparente com as peças impressas dentro](imagens/montagem.png)

## Como funciona

A ração fica no tubo de PVC, apoiada na **rampa inclinada**, que tem um furo. Embaixo da rampa, o **servo** (preso no **suporte do motor**) gira a **tampa do atuador**, que tapa e destapa esse furo. Quando abre, a ração escorrega pelo furo, cai no **funil** e sai pela lateral do tubo, pela **extensão do funil**, direto no pote. Cada abertura é uma porção.

![Peças impressas fora do tubo](imagens/pecas_impressas.png)

## Onde fica a eletrônica

[![Posicionamento da eletrônica dentro do tubo](imagens/eletronica_posicionamento.png)](imagens/eletronica_posicionamento.svg)

A eletrônica fica **embaixo do funil**, no espaço entre a base do tubo e a parte de baixo do funil (60 a 120 mm de altura, maior do lado de trás, porque o funil desce em direção à saída):

- **Placa perfurada 7 × 9 cm em pé**, encostada na aba lisa do [suporte da placa](stl/suporte_placa.stl), presa com 2 parafusos M3 (as porcas ficam dentro do U). Os componentes ficam na face voltada para o centro do tubo; o topo da placa fica 5 mm abaixo do funil.
- **ESP32-S3 nas barras fêmea, com a USB-C para baixo**: para gravar, tire o cap e ligue o cabo por baixo.
- **LED, chave e buzzer** na parede de trás (o lado oposto ao pote), **sensor de chama do alarme** na parede esquerda, **sensor da saída** preso por fora na extensão do funil.
- **Nenhum furo abaixo de 45 mm** da base: é a parte coberta pelo cap.

O desenho e as medidas vêm de [`ferramentas/eletronica_3d.py`](../../ferramentas/eletronica_3d.py), que usa as coordenadas do `IoT3D.FCStd` e confere colisões com as peças impressas. As medidas dos componentes são de catálogo: confira com paquímetro antes de furar.

- **Gerar o desenho**: `python ferramentas/eletronica_3d.py --png` (na raiz do repositório).
- **Ver no FreeCAD**: abra o `IoT3D.FCStd` e rode o mesmo arquivo em **Macro → Macros… → Arquivo do usuário**. As peças entram no grupo `Eletronica`, com as cores do desenho.

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
| Material | **PETG** (melhor para umidade e contato com ração) ou PLA |
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
