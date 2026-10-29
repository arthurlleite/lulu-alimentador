"""Gera o esquemático da eletrônica (hardware/eletronica/esquematico.svg).

Uso (na raiz do repositório):
    python ferramentas/esquematico.py

Folha A4 no estilo do EasyEDA: blocos por função, componentes com designador
(U1, R1, C1...), fios verdes e etiquetas de rede. Pontos com a mesma etiqueta
(SERVO_PWM, LED_R...) ou o mesmo símbolo de alimentação (+5V, +3.3V, GND)
estão ligados entre si.

Para gerar o PNG (precisa do Google Chrome ou Chromium):
    python ferramentas/esquematico.py --png
"""
import os
import shutil
import subprocess
import sys

L, A = 1600, 1131  # A4 deitado (297 × 210 mm)
COMP = "#880000"   # contorno e pinos dos componentes
FIO = "#008800"    # fios
REDE = "#1d3bbd"   # texto das etiquetas de rede
ALIM = "#c00000"   # símbolos de alimentação
s = []


def texto(x, y, t, tam=12, cor="#000", ancora="start", peso="normal", extra=""):
    s.append(f'<text x="{x}" y="{y}" font-size="{tam}" fill="{cor}" text-anchor="{ancora}" '
             f'font-weight="{peso}"{extra}>{t}</text>')


def linha(x1, y1, x2, y2, cor=COMP, w=1.4, extra=""):
    s.append(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{cor}" stroke-width="{w}"{extra}/>')


def ret(x, y, w, h, cor=COMP, w_linha=1.4, fundo="none", extra=""):
    s.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{fundo}" stroke="{cor}" '
             f'stroke-width="{w_linha}"{extra}/>')


def fio(*pts):
    p = " ".join(f"{x},{y}" for x, y in pts)
    s.append(f'<polyline points="{p}" fill="none" stroke="{FIO}" stroke-width="1.6"/>')


def no(x, y):
    s.append(f'<circle cx="{x}" cy="{y}" r="3.5" fill="{ALIM}"/>')


def nc(x, y):
    """Pino sem ligação (no-connect)."""
    linha(x - 5, y - 5, x + 5, y + 5, "#0000ff", 1.4)
    linha(x - 5, y + 5, x + 5, y - 5, "#0000ff", 1.4)


def rotulo(x, y, nome, lado="esq"):
    """Etiqueta de rede na ponta (x, y) de um fio; o nome fica do lado de fora."""
    d = -1 if lado == "esq" else 1
    pts = [(x, y), (x + d * 6, y - 6), (x + d * 24, y - 6), (x + d * 30, y),
           (x + d * 24, y + 6), (x + d * 6, y + 6)]
    p = " ".join(f"{a},{b}" for a, b in pts)
    s.append(f'<polygon points="{p}" fill="#fff" stroke="#a00000" stroke-width="1.3"/>')
    texto(x + d * 36, y + 4, nome, 11.5, REDE, "end" if d < 0 else "start")


def vcc(x, y, nome):
    """Símbolo de alimentação apontando para cima, com o fio chegando em (x, y)."""
    linha(x, y, x, y - 14, ALIM, 1.4)
    linha(x - 10, y - 14, x + 10, y - 14, ALIM, 2)
    texto(x, y - 20, nome, 11, ALIM, "middle")


def gnd(x, y):
    linha(x, y, x, y + 12, ALIM, 1.4)
    for i, m in enumerate((11, 7, 3)):
        linha(x - m, y + 12 + i * 4, x + m, y + 12 + i * 4, ALIM, 1.6)
    texto(x, y + 36, "GND", 10.5, REDE, "middle")


def resistor_h(x, y, ref, valor):
    """Resistor horizontal entre (x, y) e (x+60, y)."""
    linha(x, y, x + 15, y)
    ret(x + 15, y - 6, 30, 12)
    linha(x + 45, y, x + 60, y)
    texto(x + 30, y - 11, f"{ref} {valor}", 10.5, "#000080", "middle")


def led_h(x, y, cor):
    """LED horizontal: anodo em (x, y), catodo em (x+50, y)."""
    linha(x, y, x + 15, y)
    s.append(f'<polygon points="{x+15},{y-9} {x+15},{y+9} {x+33},{y}" fill="{cor}" stroke="{COMP}" stroke-width="1.3"/>')
    linha(x + 33, y - 9, x + 33, y + 9, COMP, 2)
    linha(x + 33, y, x + 50, y)
    for dx in (0, 7):  # setas de luz
        linha(x + 20 + dx, y - 11, x + 27 + dx, y - 19, COMP, 1)
        s.append(f'<polygon points="{x+27+dx},{y-19} {x+22+dx},{y-17} {x+26+dx},{y-14}" fill="{COMP}"/>')


def ic(x, y, w, ref, valor, esq=(), dir_=(), passo=20, topo=10):
    """Componente retangular. esq/dir_ = [(número, nome)]; devolve as pontas dos pinos."""
    n = max(len(esq), len(dir_))
    h = topo * 2 + (n - 1) * passo
    ret(x, y, w, h, COMP, 1.5, "#fffdf5")
    texto(x + w / 2, y - 8, ref, 11.5, "#000080", "middle", "bold")
    if valor:
        texto(x + w / 2, y + h + 16, valor, 11, "#000080", "middle")
    pontas = {}
    for i, (num, nome) in enumerate(esq):
        py = y + topo + i * passo
        linha(x - 20, py, x, py)
        texto(x - 12, py - 3, num, 8.5, COMP, "middle")
        texto(x + 5, py + 4, nome, 10.5, "#000", "start")
        pontas[nome if nome not in pontas else f"{nome}#{num}"] = (x - 20, py)
    for i, (num, nome) in enumerate(dir_):
        py = y + topo + i * passo
        linha(x + w, py, x + w + 20, py)
        texto(x + w + 12, py - 3, num, 8.5, COMP, "middle")
        texto(x + w - 5, py + 4, nome, 10.5, "#000", "end")
        pontas[nome if nome not in pontas else f"{nome}#{num}"] = (x + w + 20, py)
    return pontas


def bloco(x, y, w, h, titulo):
    ret(x, y, w, h, "#000", 1.2)
    linha(x, y + 44, x + w, y + 44, "#000", 1)
    texto(x + w / 2, y + 31, titulo, 21, "#000", "middle")


# ---------------- folha: moldura com zonas 1–6 e A–D ----------------
s.append(f'<rect width="{L}" height="{A}" fill="#fff"/>')
ret(10, 10, L - 20, A - 20, "#a00000", 1.2)
ret(30, 30, L - 60, A - 60, "#a00000", 1.2)
for i in range(6):
    x0 = 30 + i * (L - 60) / 6
    if i:
        linha(x0, 10, x0, 30, "#a00000", 1)
        linha(x0, A - 30, x0, A - 10, "#a00000", 1)
    for yy in (25, A - 14):
        texto(x0 + (L - 60) / 12, yy, str(i + 1), 12, "#a00000", "middle")
for i, letra in enumerate("ABCD"):
    y0 = 30 + i * (A - 60) / 4
    if i:
        linha(10, y0, 30, y0, "#a00000", 1)
        linha(L - 30, y0, L - 10, y0, "#a00000", 1)
    for xx in (20, L - 20):
        texto(xx, y0 + (A - 60) / 8 + 4, letra, 12, "#a00000", "middle")

# ---------------- Alimentação ----------------
bloco(30, 30, 510, 320, "Alimentação")
# fonte externa (fora da placa), ligada ao J1 pelo plugue P4
s.append(f'<circle cx="110" cy="190" r="32" fill="#fffdf5" stroke="{COMP}" stroke-width="1.5"/>')
texto(110, 182, "+", 16, COMP, "middle")
texto(110, 210, "−", 18, COMP, "middle")
texto(110, 255, "Fonte externa", 11, "#000080", "middle", "bold")
texto(110, 270, "5 V / 5 A · plugue P4 5,5 mm", 10, "#000080", "middle")
linha(110, 158, 110, 140, COMP, 1.4, ' stroke-dasharray="4 3"')
linha(110, 140, 215, 140, COMP, 1.4, ' stroke-dasharray="4 3"')
linha(110, 222, 110, 232, COMP, 1.4, ' stroke-dasharray="4 3"')
linha(110, 232, 190, 232, COMP, 1.4, ' stroke-dasharray="4 3"')
linha(190, 232, 190, 180, COMP, 1.4, ' stroke-dasharray="4 3"')
linha(190, 180, 215, 180, COMP, 1.4, ' stroke-dasharray="4 3"')
j1 = ic(235, 130, 70, "J1", "P4 fêmea c/ borne", [("1", "+"), ("2", "−")], passo=40, topo=10)
# J1 desenhado com os pinos à esquerda (plugue) e saída pelos bornes à direita
linha(305, 140, 340, 140)
linha(305, 180, 340, 180)
texto(320, 137, "1", 8.5, COMP, "middle")
texto(320, 177, "2", 8.5, COMP, "middle")
fio((340, 140), (420, 140), (420, 110))
vcc(420, 110, "+5V")
fio((340, 180), (420, 180), (420, 215))
gnd(420, 215)
texto(280, 300, "Sem chave nem fusível: desligue tirando o plugue.", 10.5, "#444", "middle")
texto(280, 318, "USB-C da placa: só para gravar (com a fonte desligada).", 10.5, "#444", "middle")

# ---------------- Processamento: ESP32-S3-DevKitC-1 (44 pinos) ----------------
bloco(540, 30, 540, 610, "Processamento")
esq = [("1", "3V3"), ("2", "3V3"), ("3", "RST"), ("4", "GPIO4"), ("5", "GPIO5"), ("6", "GPIO6"),
       ("7", "GPIO7"), ("8", "GPIO15"), ("9", "GPIO16"), ("10", "GPIO17"), ("11", "GPIO18"),
       ("12", "GPIO8"), ("13", "GPIO3"), ("14", "GPIO46"), ("15", "GPIO9"), ("16", "GPIO10"),
       ("17", "GPIO11"), ("18", "GPIO12"), ("19", "GPIO13"), ("20", "GPIO14"), ("21", "5V"),
       ("22", "GND")]
dir_ = [("44", "GND"), ("43", "TXD"), ("42", "RXD"), ("41", "GPIO1"), ("40", "GPIO2"),
        ("39", "GPIO42"), ("38", "GPIO41"), ("37", "GPIO40"), ("36", "GPIO39"), ("35", "GPIO38"),
        ("34", "GPIO37"), ("33", "GPIO36"), ("32", "GPIO35"), ("31", "GPIO0"), ("30", "GPIO45"),
        ("29", "GPIO48"), ("28", "GPIO47"), ("27", "GPIO21"), ("26", "GPIO20"), ("25", "GPIO19"),
        ("24", "GND"), ("23", "GND")]
u1 = ic(740, 120, 160, "U1", "ESP32-S3-DevKitC-1 N16R8", esq, dir_)
# 3V3 (pinos 1 e 2) -> +3.3V
x1, y1 = u1["3V3"]
x2, y2 = u1["3V3#2"]
fio((x1, y1), (680, y1), (680, 112))
fio((x2, y2), (680, y2), (680, y1))
no(680, y1)
vcc(680, 112, "+3.3V")
# 5V e GND do lado esquerdo
x, y = u1["5V"]
fio((x, y), (690, y), (690, y - 22))
vcc(690, y - 22, "+5V")
x, y = u1["GND"]
fio((x, y), (690, y), (690, y + 10))
gnd(690, y + 10)
# GND do lado direito (44, 24, 23)
xa, ya = u1["GND#44"]
xb, yb = u1["GND#24"]
xc, yc = u1["GND#23"]
fio((xa, ya), (960, ya), (960, yc + 10))
fio((xb, yb), (960, yb))
fio((xc, yc), (960, yc))
no(960, yb)
no(960, yc)
gnd(960, yc + 10)
# sinais
SINAIS = {"GPIO4": "SERVO_PWM", "GPIO15": "BOTAO", "GPIO16": "BUZZER", "GPIO17": "LED_R",
          "GPIO18": "LED_G", "GPIO8": "LED_B", "GPIO9": "SAIDA_D0", "GPIO10": "CHAMA_D0"}
for pino, rede in SINAIS.items():
    x, y = u1[pino]
    fio((x, y), (660, y))
    rotulo(660, y, rede)
x, y = u1["GPIO0"]
texto(x + 6, y + 4, "← botão BOOT da placa", 10, "#666")
_, y35 = u1["GPIO37"]
_, y37 = u1["GPIO35"]
linha(990, y35 - 6, 996, y35 - 6, "#666", 1)
linha(996, y35 - 6, 996, y37 + 6, "#666", 1)
linha(990, y37 + 6, 996, y37 + 6, "#666", 1)
texto(1002, (y35 + y37) / 2 - 2, "PSRAM octal", 10, "#666")
texto(1002, (y35 + y37) / 2 + 11, "(não usar)", 10, "#666")
for nome in ("RST", "TXD", "RXD"):
    x, y = u1[nome]
    nc(x, y)

# ---------------- Atuador do dosador ----------------
bloco(1080, 30, 490, 370, "Atuador do Dosador")
m1 = ic(1360, 125, 120, "M1", "Servo MG996R 180°", [("1", "V+"), ("2", "SINAL"), ("3", "GND")], passo=30, topo=20)
s.append(f'<circle cx="1440" cy="170" r="22" fill="none" stroke="{COMP}" stroke-width="1.5"/>')
texto(1440, 177, "M", 18, COMP, "middle")
texto(1300, 142, "vermelho", 9.5, "#666", "middle")
texto(1300, 172, "laranja", 9.5, "#666", "middle")
texto(1300, 202, "marrom", 9.5, "#666", "middle")
x, y = m1["V+"]
fio((x, y), (1270, y), (1270, y - 30))
vcc(1270, y - 30, "+5V")
x, y = m1["SINAL"]
fio((x, y), (1230, y))
rotulo(1230, y, "SERVO_PWM")
x, y = m1["GND"]
fio((x, y), (1270, y), (1270, y + 25))
gnd(1270, y + 25)
# C1 junto ao conector do servo
fio((1190, 290), (1190, 310))
vcc(1190, 290, "+5V")
linha(1176, 310, 1204, 310, COMP, 2.2)
s.append(f'<path d="M1176,322 Q1190,314 1204,322" fill="none" stroke="{COMP}" stroke-width="2.2"/>')
texto(1172, 309, "+", 12, COMP, "end")
fio((1190, 318), (1190, 340))
gnd(1190, 340)
texto(1214, 305, "C1", 11.5, "#000080", "start", "bold")
texto(1214, 320, "1000 µF / 16 V", 11, "#000080")
texto(1300, 300, "Soldar C1 junto ao conector", 10.5, "#444")
texto(1300, 315, "do servo: segura o pico de", 10.5, "#444")
texto(1300, 330, "partida (&gt; 1 A) e evita que", 10.5, "#444")
texto(1300, 345, "a ESP32 reinicie.", 10.5, "#444")

# ---------------- Sensores de chama ----------------
bloco(1080, 400, 490, 530, "Detecção de Saída e de Chama")
pinos_chama = [("1", "A0"), ("2", "D0"), ("3", "GND"), ("4", "VCC")]
u2 = ic(1400, 520, 110, "U2", "Sensor de chama (saída)", pinos_chama)
u3 = ic(1400, 720, 110, "U3", "Sensor de chama (alarme)", pinos_chama)
texto(1455, 625, "olha a ração caindo", 10, "#666", "middle")
texto(1455, 825, "olha o ambiente", 10, "#666", "middle")
BUS_GND, BUS_VCC = 1320, 1340
vcc(BUS_VCC, 480, "+3.3V")
fio((BUS_VCC, 480), (BUS_VCC, u3["VCC"][1]))
fio((BUS_GND, u2["GND"][1]), (BUS_GND, 840))
gnd(BUS_GND, 840)
for u, rede in ((u2, "SAIDA_D0"), (u3, "CHAMA_D0")):
    x, y = u["VCC"]
    fio((x, y), (BUS_VCC, y))
    x, y = u["GND"]
    fio((x, y), (BUS_GND, y))
    x, y = u["D0"]
    fio((x, y), (1240, y))
    rotulo(1240, y, rede)
    nc(*u["A0"])
no(BUS_VCC, u2["VCC"][1])
no(BUS_GND, u3["GND"][1])
texto(1100, 900, "Módulo com LM393: D0 vai a 0 ao receber IR. Sensibilidade no trimpot.", 10.5, "#444")
texto(1100, 916, "Conferir a ordem dos pinos na serigrafia do módulo comprado.", 10.5, "#444")

# ---------------- Indicadores e interação ----------------
bloco(30, 350, 510, 580, "Indicadores e Interação com Usuário")
# LED RGB catodo comum com resistores de 220 Ω
ret(250, 420, 120, 190, "#888", 1, extra=' stroke-dasharray="5 4"')
texto(380, 632, "D1 · LED RGB 5 mm catodo comum", 11, "#000080", "end", "bold")
for i, (rede, ref, cor) in enumerate((("LED_R", "R1", "#e53935"), ("LED_G", "R2", "#43a047"),
                                      ("LED_B", "R3", "#1e63d0"))):
    y = 450 + i * 60
    rotulo(130, y, rede)
    fio((130, y), (165, y))
    resistor_h(165, y, ref, "220Ω")
    fio((225, y), (270, y))
    led_h(270, y, cor)
    fio((320, y), (400, y))
texto(345, 445, "R", 10, "#666", "middle")
texto(345, 505, "G", 10, "#666", "middle")
texto(345, 565, "B", 10, "#666", "middle")
fio((400, 450), (400, 600))
no(400, 510)
no(400, 570)
gnd(400, 600)
texto(430, 520, "perna longa", 10, "#666")
texto(430, 534, "= catodo", 10, "#666")
# buzzer ativo
rotulo(130, 720, "BUZZER")
fio((130, 720), (210, 720))
texto(214, 714, "+", 12, COMP, "start")
linha(210, 720, 230, 720)
ret(230, 705, 50, 30, COMP, 1.5, "#fffdf5")
s.append(f'<path d="M238,705 A17,17 0 0 1 272,705" fill="#fffdf5" stroke="{COMP}" stroke-width="1.5"/>')
linha(280, 720, 300, 720)
fio((300, 720), (340, 720), (340, 735))
gnd(340, 735)
texto(255, 680, "BZ1", 11.5, "#000080", "middle", "bold")
texto(255, 755, "Buzzer ativo 3,3 V", 11, "#000080", "middle")
# chave táctil
rotulo(130, 850, "BOTAO")
fio((130, 850), (215, 850))
s.append(f'<circle cx="219" cy="850" r="4" fill="#fff" stroke="{COMP}" stroke-width="1.4"/>')
s.append(f'<circle cx="271" cy="850" r="4" fill="#fff" stroke="{COMP}" stroke-width="1.4"/>')
linha(212, 836, 278, 836, COMP, 2)
linha(245, 836, 245, 822, COMP, 1.4)
linha(236, 822, 254, 822, COMP, 1.4)
fio((275, 850), (340, 850), (340, 862))
gnd(340, 862)
texto(245, 812, "SW1", 11.5, "#000080", "middle", "bold")
texto(245, 875, "Chave táctil 12×12 mm", 11, "#000080", "middle")
texto(400, 712, "GPIO16 direto:", 10, "#666")
texto(400, 726, "buzzer de 3,3 V", 10, "#666")
texto(400, 740, "(≈ 25 mA)", 10, "#666")
texto(400, 842, "pull-up interno;", 10, "#666")
texto(400, 856, "toque = 1 porção,", 10, "#666")
texto(400, 870, "5 s = apagar Wi-Fi", 10, "#666")

# ---------------- Notas ----------------
bloco(540, 640, 540, 290, "Notas")
notas = [
    "1. Etiquetas e símbolos de alimentação com o mesmo nome estão ligados.",
    "2. A fonte de 5 V alimenta a ESP32 (pino 5V) e o servo. O servo nunca vai",
    "    no 5 V do USB do computador.",
    "3. U2 fica na saída: cada grão que passa entre ele e a luz do outro lado",
    "    gera um pulso no D0 e confirma que a ração caiu.",
    "4. U3 fica virado para fora: chama acende o LED vermelho, toca o buzzer",
    "    e avisa no app.",
    "5. Os pinos são configuráveis no menuconfig (\"Alimentador - Pinos\").",
    "6. HC-SR04 e DHT11 saíram do projeto: ALIM_SENSOR_NIVEL e ALIM_DHT11",
    "    ficam desligados. Os pinos 5, 6 e 7 ficam livres.",
]
for i, n in enumerate(notas):
    texto(560, 710 + i * 21, n, 12)

# ---------------- Lista de componentes ----------------
ret(30, 930, 970, A - 960, "#000", 1.2)
texto(45, 952, "Lista de componentes", 14, "#000", "start", "bold")
texto(985, 952, "rateio = comprado em 26/10/2026 e dividido entre os 4 integrantes · acervo = já tínhamos",
      10.5, "#444", "end")
bom = [
    ("U1", "ESP32-S3-DevKitC-1 N16R8", "1", "acervo"),
    ("M1", "Servo MG996R 180°", "1", "acervo"),
    ("U2, U3", "Módulo sensor de chama (kit com 2)", "2", "rateio"),
    ("D1", "LED RGB 5 mm catodo comum", "1", "rateio"),
    ("R1–R3", "Resistor 220 Ω 1/4 W", "3", "acervo"),
    ("C1", "Capacitor eletrolítico 1000 µF / 16 V", "1", "rateio"),
    ("BZ1", "Buzzer ativo 3,3 V", "1", "rateio"),
    ("SW1", "Chave táctil 12×12 mm", "1", "rateio"),
    ("J1", "Conector P4 fêmea com borne", "1", "acervo"),
    ("—", "Fonte 5 V / 5 A, plugue P4", "1", "acervo"),
    ("—", "Placa perfurada 7×9 cm, barras de pinos", "1", "acervo"),
    ("—", "Jumpers, fio 20 AWG, cabo USB-C", "—", "acervo"),
]
for col in range(2):
    x0 = 45 + col * 480
    texto(x0, 975, "Ref.", 10.5, "#000", "start", "bold")
    texto(x0 + 60, 975, "Componente", 10.5, "#000", "start", "bold")
    texto(x0 + 360, 975, "Qtd", 10.5, "#000", "middle", "bold")
    texto(x0 + 420, 975, "Origem", 10.5, "#000", "middle", "bold")
    linha(x0, 980, x0 + 450, 980, "#999", 0.8)
    for i, (ref, nome, qtd, origem) in enumerate(bom[col * 6:col * 6 + 6]):
        yy = 997 + i * 17
        texto(x0, yy, ref, 10.5)
        texto(x0 + 60, yy, nome, 10.5)
        texto(x0 + 360, yy, qtd, 10.5, "#000", "middle")
        texto(x0 + 420, yy, origem, 10.5, ALIM if origem == "rateio" else "#000", "middle")
linha(510, 960, 510, A - 40, "#ccc", 0.8)

# ---------------- carimbo ----------------
cx, cy, cw = 1000, 930, L - 30 - 1000
ret(cx, cy, cw, A - 30 - cy, "#000", 1.2)
for yy in (cy + 46, cy + 69, cy + 92, cy + 115, cy + 140):
    linha(cx, yy, cx + cw, yy, "#000", 0.8)
linha(cx + 360, cy + 23, cx + cw, cy + 23, "#000", 0.8)
linha(cx + 90, cy, cx + 90, cy + 115, "#000", 0.8)
for xx in (cx + 360, cx + 450):
    linha(xx, cy, xx, cy + 69, "#000", 0.8)
for xx in (cx + 190, cx + 380):
    linha(xx, cy + 115, xx, cy + 140, "#000", 0.8)
linha(cx + 260, cy + 140, cx + 260, A - 30, "#000", 0.8)
texto(cx + 8, cy + 28, "Esquema", 11)
texto(cx + 225, cy + 29, "Lulu · Alimentador automático", 14, "#000", "middle")
texto(cx + 368, cy + 16, "Criado em", 11)
texto(cx + 510, cy + 16, "2026-10-22", 11, "#000", "middle")
texto(cx + 368, cy + 39, "Atualizado", 11)
texto(cx + 510, cy + 39, "2026-10-29", 11, "#000", "middle")
texto(cx + 8, cy + 62, "Placa", 11)
texto(cx + 225, cy + 62, "Perfurada 7×9 cm", 11, "#000", "middle")
texto(cx + 368, cy + 62, "Página", 11)
texto(cx + 510, cy + 62, "Circuito final", 11, "#000", "middle")
texto(cx + 8, cy + 85, "Desenho", 11)
texto(cx + 100, cy + 85, "Arthur Carvalho Leite", 11)
texto(cx + 8, cy + 108, "Revisão", 11)
texto(cx + 100, cy + 108, "Equipe Lulu (4 integrantes)", 11)
texto(cx + 95, cy + 133, "Versão  V2.0", 11, "#000", "middle")
texto(cx + 285, cy + 133, "Tamanho  A4", 11, "#000", "middle")
texto(cx + 475, cy + 133, "Página 1 de 1", 11, "#000", "middle")
texto(cx + 130, cy + 160, "Lulu · FSE 2026/2", 13, "#1d3bbd", "middle", "bold")
texto(cx + 415, cy + 160, "github.com/arthurlleite/lulu-alimentador", 11, "#000", "middle")

svg = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {L} {A}" width="{L}" height="{A}" '
       f'font-family="Arial, Helvetica, sans-serif">\n' + "\n".join(s) + "\n</svg>\n")
saida = os.path.join("hardware", "eletronica", "esquematico.svg")
open(saida, "w", encoding="utf-8", newline="\n").write(svg)
print(saida, len(svg))

if "--png" in sys.argv:
    chrome = next(filter(None, map(shutil.which, ("google-chrome", "chromium", "chromium-browser"))), None)
    if not chrome:
        sys.exit("Chrome/Chromium não encontrado: abra o SVG no navegador e exporte o PNG.")
    png = os.path.abspath(os.path.join("hardware", "eletronica", "esquematico.png"))
    subprocess.run([chrome, "--headless=new", "--disable-gpu", "--hide-scrollbars",
                    "--force-device-scale-factor=1.5", f"--window-size={L},{A + 200}",
                    f"--screenshot={png}", "file://" + os.path.abspath(saida)],
                   check=True, capture_output=True)
    # a janela do Chrome sai mais alta que a folha; recorta no tamanho A4
    from PIL import Image
    Image.open(png).crop((0, 0, int(L * 1.5), int(A * 1.5))).save(png, optimize=True)
    print(png)
