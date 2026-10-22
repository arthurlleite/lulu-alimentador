"""Gera o pré-esquemático da eletrônica (hardware/eletronica/esquematico.svg).

Uso (na raiz do repositório):
    python ferramentas/esquematico.py

As ligações usam etiquetas de rede (como no KiCad): pontos com a mesma
etiqueta (5V, GND, 3V3, GPIO4...) estão ligados entre si.
"""
import os

L, A = 1500, 1060  # largura e altura do desenho
COR = {"5V": "#d62828", "GND": "#333333", "3V3": "#e07a00", "sinal": "#1d5fbf"}
s = []


def cor_rede(rede):
    return COR.get(rede, COR["sinal"])


def texto(x, y, t, tam=14, peso="normal", cor="#222", ancora="start"):
    s.append(f'<text x="{x}" y="{y}" font-size="{tam}" font-weight="{peso}" fill="{cor}" '
             f'text-anchor="{ancora}">{t}</text>')


def linha(x1, y1, x2, y2, cor="#222", w=2):
    s.append(f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{cor}" stroke-width="{w}"/>')


def rotulo(x, y, rede, lado="dir"):
    """Etiqueta de rede em forma de seta, a partir do ponto (x, y)."""
    c = cor_rede(rede)
    w = 12 + 8.2 * len(rede)
    if lado == "dir":
        pts = f"{x},{y} {x+8},{y-10} {x+8+w},{y-10} {x+8+w},{y+10} {x+8},{y+10}"
        tx = x + 8 + w / 2
    else:
        pts = f"{x},{y} {x-8},{y-10} {x-8-w},{y-10} {x-8-w},{y+10} {x-8},{y+10}"
        tx = x - 8 - w / 2
    s.append(f'<polygon points="{pts}" fill="#fff" stroke="{c}" stroke-width="1.6"/>')
    texto(tx, y + 5, rede, 12.5, "bold", c, "middle")


def pino(x, y, comp, rede, nome, lado="dir"):
    """Pino de um componente: traço, nome do pino e etiqueta de rede."""
    c = cor_rede(rede)
    dx = comp if lado == "dir" else -comp
    linha(x, y, x + dx, y, c)
    s.append(f'<circle cx="{x}" cy="{y}" r="3" fill="{c}"/>')
    if nome:
        if lado == "dir":
            texto(x - 6, y + 5, nome, 12.5, "normal", "#444", "end")
        else:
            texto(x + 6, y + 5, nome, 12.5, "normal", "#444", "start")
    rotulo(x + dx, y, rede, lado)


def caixa(x, y, w, h, titulo, sub=""):
    s.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="10" fill="#fffaf3" stroke="#c9a77c" stroke-width="1.5"/>')
    texto(x + 12, y + 22, titulo, 15, "bold", "#5a3a14")
    if sub:
        texto(x + 12, y + 40, sub, 12, "normal", "#8a7461")


def resistor(x, y, valor, vertical=False):
    if vertical:
        s.append(f'<rect x="{x-7}" y="{y}" width="14" height="34" fill="#fff" stroke="#222" stroke-width="2"/>')
        texto(x + 12, y + 22, valor, 12.5, "bold")
    else:
        s.append(f'<rect x="{x}" y="{y-7}" width="34" height="14" fill="#fff" stroke="#222" stroke-width="2"/>')
        texto(x + 17, y - 12, valor, 12.5, "bold", "#222", "middle")


# ---------------- fundo e título ----------------
s.append(f'<rect width="{L}" height="{A}" fill="#ffffff"/>')
texto(30, 44, "Lulu · Alimentador Automático de Pets — pré-esquemático da eletrônica", 22, "bold", "#2b2118")
texto(30, 68, "ESP32-S3-DevKitC-1 · etiquetas iguais = mesmo fio · valores entre parênteses são da ESP32 clássica", 13.5, "normal", "#8a7461")

# ---------------- alimentação ----------------
caixa(30, 95, 330, 250, "Alimentação", "fonte externa, GND comum a tudo")
s.append('<rect x="60" y="160" width="90" height="70" rx="8" fill="#fff" stroke="#222" stroke-width="2"/>')
texto(105, 190, "Fonte", 14, "bold", "#222", "middle")
texto(105, 208, "5 V / 3 A", 13, "normal", "#222", "middle")
linha(150, 175, 200, 175, COR["5V"]); texto(158, 170, "+", 14, "bold", COR["5V"])
linha(150, 215, 200, 215, COR["GND"]); texto(158, 232, "−", 16, "bold", COR["GND"])
rotulo(200, 175, "5V")
rotulo(200, 215, "GND")
# capacitor do servo
texto(60, 266, "C1 1000 µF / 10 V (perto do servo)", 12.5, "bold")
linha(105, 285, 105, 299, COR["5V"]); linha(88, 299, 122, 299, "#222", 2.5)
s.append('<path d="M88 311 Q105 303 122 311" fill="none" stroke="#222" stroke-width="2.5"/>')
linha(105, 307, 105, 325, COR["GND"])
texto(80, 296, "+", 14, "bold", COR["5V"])
linha(105, 285, 160, 285, COR["5V"]); rotulo(160, 285, "5V")
linha(105, 325, 160, 325, COR["GND"]); rotulo(160, 325, "GND")
texto(132, 310, "C1", 12.5, "bold")

# ---------------- ESP32-S3 ----------------
ex, ey, ew, eh = 470, 95, 270, 640
s.append(f'<rect x="{ex}" y="{ey}" width="{ew}" height="{eh}" rx="12" fill="#eef4ff" stroke="#1d5fbf" stroke-width="2.5"/>')
texto(ex + ew / 2, ey + 30, "ESP32-S3-DevKitC-1", 17, "bold", "#1d3f7a", "middle")
texto(ex + ew / 2, ey + 50, "N16R8 · Wi-Fi 2,4 GHz · USB-C", 12.5, "normal", "#4a6696", "middle")
pinos_esq = [("5V", "5V"), ("GND", "GND"), ("3V3", "3V3")]
for i, (nome, rede) in enumerate(pinos_esq):
    pino(ex, ey + 110 + i * 45, 40, rede, "", "esq")
    texto(ex + 10, ey + 115 + i * 45, nome, 13, "bold", "#1d3f7a")
pinos_dir = [
    ("GPIO 4", "SERVO", "(18)"), ("GPIO 5", "TRIG", "(25)"), ("GPIO 6", "ECHO_3V3", "(34)"),
    ("GPIO 7", "DHT", "(4)"), ("GPIO 9", "IR_SAIDA", "(32)"), ("GPIO 15", "BOTAO", "(27)"),
    ("GPIO 16", "BUZZER", "(26)"), ("GPIO 17", "LED_R", "(21)"), ("GPIO 18", "LED_G", "(22)"),
    ("GPIO 8", "LED_B", "(23)"),
]
for i, (nome, rede, classico) in enumerate(pinos_dir):
    y = ey + 110 + i * 46
    pino(ex + ew, y, 40, rede, "", "dir")
    texto(ex + ew - 10, y + 5, f"{nome} {classico}", 13, "bold", "#1d3f7a", "end")
texto(ex + 14, ey + eh - 58, "GPIO 0 = botão BOOT da própria placa:", 12, "normal", "#4a6696")
texto(ex + 14, ey + eh - 40, "faz o mesmo que o botão externo", 12, "normal", "#4a6696")
texto(ex + 14, ey + eh - 22, "(não precisa ligar nada)", 12, "normal", "#4a6696")

# ---------------- componentes ----------------
cx1, cx2 = 900, 1200
w = 270


def comp_servo(x, y):
    caixa(x, y, w, 150, "Servo MG996R", "180°, alimentado direto da fonte")
    pino(x + 120, y + 70, 40, "5V", "vermelho")
    pino(x + 120, y + 100, 40, "GND", "marrom")
    pino(x + 120, y + 130, 40, "SERVO", "laranja (sinal)")


def comp_hcsr04(x, y):
    caixa(x, y, w, 250, "HC-SR04 (nível)", "na tampa de cima, apontando para baixo")
    pino(x + 110, y + 70, 40, "5V", "VCC")
    pino(x + 110, y + 100, 40, "GND", "GND")
    pino(x + 110, y + 130, 40, "TRIG", "TRIG")
    # divisor do eco
    texto(x + 14, y + 165, "ECHO (5 V) → divisor → 3,3 V", 12.5, "bold", "#444")
    yy = y + 195
    s.append(f'<circle cx="{x+30}" cy="{yy}" r="3" fill="{COR["sinal"]}"/>')
    texto(x + 14, yy - 10, "ECHO", 12, "normal", "#444")
    linha(x + 30, yy, x + 60, yy, COR["sinal"]); resistor(x + 60, yy, "R1 1 kΩ")
    linha(x + 94, yy, x + 140, yy, COR["sinal"])
    s.append(f'<circle cx="{x+140}" cy="{yy}" r="3.5" fill="{COR["sinal"]}"/>')
    rotulo(x + 140, yy, "ECHO_3V3")
    linha(x + 140, yy, x + 140, yy + 10, COR["sinal"]); resistor(x + 140, yy + 10, "R2 2 kΩ", True)
    linha(x + 140, yy + 44, x + 140, yy + 50, COR["GND"])
    texto(x + 152, yy + 52, "GND", 12, "bold", COR["GND"])


def comp_dht(x, y):
    caixa(x, y, w, 145, "DHT11 (módulo 3 pinos)", "temperatura e umidade no tubo")
    pino(x + 110, y + 70, 40, "3V3", "VCC")
    pino(x + 110, y + 100, 40, "GND", "GND")
    pino(x + 110, y + 130, 40, "DHT", "DATA")


def comp_ir(x, y):
    caixa(x, y, w, 145, "Sensor IR (FC-51)", "na saída da ração")
    pino(x + 110, y + 70, 40, "3V3", "VCC")
    pino(x + 110, y + 100, 40, "GND", "GND")
    pino(x + 110, y + 130, 40, "IR_SAIDA", "OUT")


def comp_buzzer(x, y):
    caixa(x, y, w, 145, "Buzzer ativo (módulo)", "chama o pet e avisa erro")
    pino(x + 110, y + 70, 40, "3V3", "VCC")
    pino(x + 110, y + 100, 40, "GND", "GND")
    pino(x + 110, y + 130, 40, "BUZZER", "I/O")


def comp_botao(x, y):
    caixa(x, y, w, 120, "Botão", "toque = 1 porção · 5 s = apagar Wi-Fi")
    yy = y + 85
    x += 40
    linha(x + 40, yy, x + 80, yy, COR["sinal"]); s.append(f'<circle cx="{x+80}" cy="{yy}" r="3" fill="#222"/>')
    linha(x + 82, yy - 2, x + 120, yy - 16, "#222", 2.5); s.append(f'<circle cx="{x+122}" cy="{yy}" r="3" fill="#222"/>')
    linha(x + 122, yy, x + 160, yy, COR["GND"])
    rotulo(x + 40, yy, "BOTAO", "esq")
    rotulo(x + 160, yy, "GND")
    texto(x + 100, yy + 26, "pull-up interno", 11.5, "normal", "#8a7461", "middle")
    x -= 40


def comp_led(x, y):
    caixa(x, y, w, 205, "LED RGB catodo comum", "status: azul, amarelo, verde...")
    x0 = x
    x += 28
    for i, (cor_led, rede) in enumerate([("#e53935", "LED_R"), ("#2e9d4b", "LED_G"), ("#1e6fd9", "LED_B")]):
        yy = y + 75 + i * 38
        rotulo(x + 52, yy, rede, "esq")
        linha(x + 52, yy, x + 70, yy, COR["sinal"]); resistor(x + 70, yy, "220 Ω")
        linha(x + 104, yy, x + 130, yy, "#222")
        s.append(f'<polygon points="{x+130},{yy-9} {x+130},{yy+9} {x+148},{yy}" fill="{cor_led}" stroke="#222" stroke-width="1.5"/>')
        linha(x + 148, yy - 9, x + 148, yy + 9, "#222", 2.5)
        linha(x + 148, yy, x + 180, yy, "#222")
    linha(x + 180, y + 75, x + 180, y + 175, COR["GND"])
    linha(x + 180, y + 175, x + 190, y + 175, COR["GND"])
    rotulo(x + 190, y + 175, "GND")
    texto(x0 + 14, y + 196, "anodo comum: perna longa no 3V3", 11.5, "normal", "#8a7461")


comp_servo(cx1, 95)
comp_hcsr04(cx1, 265)
comp_dht(cx1, 535)
comp_ir(cx1, 700)
comp_buzzer(cx2, 95)
comp_botao(cx2, 260)
comp_led(cx2, 400)

# ---------------- notas ----------------
ny = 780
caixa(30, ny, 830, 250, "Notas")
notas = [
    "1. Etiquetas com o mesmo nome estão ligadas (ex.: todo GND junto, todo 5V junto).",
    "2. O servo puxa picos acima de 1 A: alimente pela fonte, nunca pelo 5 V do USB do computador.",
    "3. C1 (1000 µF) fica soldado perto do conector do servo, entre 5V e GND.",
    "4. O ECHO do HC-SR04 sai em 5 V: R1/R2 baixam para 3,3 V (5 × 2k / 3k = 3,3 V).",
    "5. DHT11, sensor IR e buzzer em módulo já trazem resistores; ligue em 3V3.",
    "6. Ao gravar pelo USB, desligue a fonte de 5 V externa.",
    "7. Pinos configuráveis no menuconfig (menu \"Alimentador - Pinos\").",
]
for i, n in enumerate(notas):
    texto(50, ny + 50 + i * 27, n, 13.5)

caixa(cx1, 870, 570, 160, "Legenda")
for i, (nome, c) in enumerate([("5 V (fonte)", COR["5V"]), ("GND", COR["GND"]), ("3,3 V (da placa)", COR["3V3"]), ("sinal (GPIO)", COR["sinal"])]):
    yy = 915 + i * 28
    linha(cx1 + 20, yy, cx1 + 70, yy, c, 3)
    texto(cx1 + 82, yy + 5, nome, 13.5)
texto(cx1 + 300, 920, "Valores entre parênteses ao lado", 13, "normal", "#444")
texto(cx1 + 300, 940, "dos GPIOs = ESP32 clássica (DevKit V1).", 13, "normal", "#444")
texto(cx1 + 300, 975, "github.com/arthurlleite/lulu-alimentador", 13, "bold", "#5a3a14")

svg = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {L} {A}" width="{L}" height="{A}" '
       f'font-family="Segoe UI, Roboto, Helvetica, Arial, sans-serif">\n' + "\n".join(s) + "\n</svg>\n")
saida = os.path.join("hardware", "eletronica", "esquematico.svg")
open(saida, "w", encoding="utf-8", newline="\n").write(svg)
print(saida, len(svg))
