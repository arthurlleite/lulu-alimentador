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

1. Monte a placa conforme o [guia de ligações](../hardware/eletronica/README.md) (não esqueça o divisor de tensão do ECHO e o capacitor do servo).
2. Fixe a placa com o [suporte da placa](../hardware/3d/stl/suporte_placa.stl).
3. **HC-SR04**: na [tampa do tubo](../hardware/3d/stl/tampa_tubo.stl), faça dois furos de 16 mm e prenda o sensor apontando para baixo.
4. **Sensor IR**: na lateral da extensão do funil, apontando para onde a ração passa.
5. **DHT11**: perto do topo, dentro do tubo.
6. **LED, botão e buzzer**: num lugar visível, por fora.

## 5. Primeira configuração

1. Grave o firmware pelo [instalador no navegador](https://arthurlleite.github.io/lulu-alimentador/instalar.html).
2. Conecte o celular na rede `Alimentador-XXXX`, informe o nome do pet e o Wi-Fi da casa.
3. Abra o app e vá em **Ajustes → Calibração**:
   - com o tubo **vazio**, toque em "vazio = agora";
   - encha com ração e toque em "cheio = agora";
   - salve.
4. **Calibre a porção**: sirva 5 porções num pote vazio, pese e divida por 5. Para mudar a quantidade por porção, ajuste `ALIM_TEMPO_ABERTO_MS` no `menuconfig` (padrão 800 ms).
5. Monte a agenda no app.

## 6. Acabamento

- Feche a base com o cap de PVC.
- Se for pintar, pinte **só por fora** (o protótipo foi pintado de branco).
- Mantenha a tampa fechada: a umidade estraga a ração (o DHT11 avisa no app se passar de 70 %).

## Problemas comuns

| Sintoma | Causa provável | O que fazer |
|---|---|---|
| A ESP reinicia quando o servo mexe | queda de tensão | fonte de 3 A, capacitor de 1000 µF perto do servo, fios curtos |
| Servo treme parado | ruído no sinal | o firmware corta o PWM depois de servir; confira o GND comum |
| App avisa "nada caiu" | ração travada ou sensor IR desalinhado | sacuda o tubo; ajuste o trimpot do sensor IR |
| Nível em 0 % ou 100 % fixo | calibração ou eco no tubo | refaça a calibração; centralize o HC-SR04 |
| LED azul piscando sempre | senha do Wi-Fi errada ou rede 5 GHz | refaça a configuração com uma rede de 2,4 GHz |
