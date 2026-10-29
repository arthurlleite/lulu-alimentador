"""Posição da eletrônica dentro do tubo: folhas de montagem e peças no FreeCAD.

Uma única lista (COMPONENTES) guarda medida e posição de cada peça, nas mesmas
coordenadas do IoT3D.FCStd: eixo Z no centro do tubo, base do tubo em z = -177,
saída da ração (pote) para -Y, parede de trás em +Y. Medidas em mm.

As medidas são envelopes: a maior medida publicada entre as variantes de cada
peça, para sobrar espaço se a peça recebida for um pouco maior. O script mede a
folga de cada peça até o funil, a parede e as outras peças, e para com erro se
alguma ficar abaixo de FOLGA_MIN (contatos de propósito, como a placa no suporte,
ficam de fora da conta).

Folhas (na raiz do repositório; --png precisa do Chrome/Chromium e do Pillow):
    python ferramentas/eletronica_3d.py --png
    -> hardware/3d/imagens/eletronica_posicionamento.svg/.png  (vistas do tubo)
    -> hardware/3d/imagens/eletronica_furacao.svg/.png         (placa e furos)

FreeCAD (0.20 ou mais novo): abra hardware/3d/IoT3D.FCStd, vá em
Macro > Macros... > Arquivo do usuário, aponte para este arquivo e execute.
As peças entram no grupo "Eletronica" (rodar de novo substitui o grupo).
Confira e salve o arquivo.
"""
import math
import os
import shutil
import struct
import subprocess
import sys

BASE_Z = -177.0      # base do tubo
R_INT = 48.75        # raio interno do tubo (Ø 97,5)
R_EXT = 50.75        # raio externo (Ø 101,5)
TOPO_Z = 123.0       # boca do tubo (a tampa impressa vai por fora)
CAP_MM = 40          # bolsa do cap PVC na base: nenhum furo encosta nessa faixa
FOLGA_MIN = 3.0      # folga mínima exigida entre peças (mm)
PLACA_X = 27.5       # face dos componentes da placa perfurada
PARAFUSOS_Z = (-115.5, -130.5)  # furos Ø3,3 do suporte da placa (medidos no STL), em y = 0

# Cada componente:
#   num, ref, nome, forma, medidas, posição, cor, tipo, fonte da medida
#   forma "caixa":    medidas = (dx, dy, dz), posição = canto mínimo (x, y, z)
#   forma "cilindro": medidas = (raio, altura, eixo "+y"/"+z"), posição = centro da base
#   tipo: "placa" (soldado na placa), "parede" (preso na parede), "fora" (por fora do tubo),
#         "solto" (dentro, preso por fios), "base" (a própria placa)
COMPONENTES = [
    dict(num=1, ref="—", nome="Placa perfurada 7×9 cm, em pé no suporte", forma="caixa",
         m=(1.6, 70, 90), p=(PLACA_X, -35, -175), cor="#c8a165", tipo="base",
         fonte="70 × 90 × 1,6 (fenolite, passo 2,54)"),
    dict(num=2, ref="U1", nome="ESP32-S3-DevKitC-1 N16R8", forma="caixa",
         m=(1.6, 28, 69), p=(14.9, 6.8, -163), cor="#3a3a3a", tipo="placa",
         fonte="Espressif 62,7 × 25,4; clones até 69 × 28"),
    dict(num=3, ref="U1", nome="Módulo WROOM-1 (antena para cima)", forma="caixa",
         m=(3.1, 18, 25.5), p=(11.8, 11.8, -119.5), cor="#b8b8b8", tipo="placa",
         fonte="18 × 25,5 × 3,1 (datasheet)"),
    dict(num=4, ref="U1", nome="2 × USB-C da ESP (para baixo)", forma="caixa",
         m=(3.5, 20, 8), p=(11.4, 10.8, -164), cor="#8c8c8c", tipo="placa",
         fonte="9 × 3,3 cada, 1 mm para fora da placa"),
    dict(num=5, ref="—", nome="Barras fêmea 1×22 + espaçador dos pinos", forma="caixa",
         m=(11, 2.54, 55.88), p=(16.5, 6.83, -156.44), cor="#1e1e1e", tipo="placa",
         fonte="fêmea 8,5 + espaçador 2,5; fileiras a 25,4"),
    dict(num=5, ref="—", nome="Barras fêmea 1×22 (lado 2)", forma="caixa",
         m=(11, 2.54, 55.88), p=(16.5, 32.23, -156.44), cor="#1e1e1e", tipo="placa",
         fonte=""),
    dict(num=6, ref="R1–R3", nome="Resistores 220 Ω deitados", forma="caixa",
         m=(3, 10, 10), p=(24.5, -24, -140), cor="#d9b26f", tipo="placa",
         fonte="corpo 6,3 × Ø2,5 (1/4 W)"),
    dict(num=7, ref="C1", nome="Capacitor 1000 µF / 16 V, deitado", forma="cilindro",
         m=(5.25, 21, "+z"), p=(22.25, -20, -172), cor="#2c56a8", tipo="placa",
         fonte="Ø10 × 16 a 20 → envelope Ø10,5 × 21"),
    dict(num=8, ref="—", nome="Barra macho 1×3 do servo", forma="caixa",
         m=(11.5, 7.62, 2.54), p=(16, -10, -170), cor="#1e1e1e", tipo="placa",
         fonte="pino 11,5 de altura total"),
    dict(num=9, ref="—", nome="Barra macho 1×14 (sensores e painel)", forma="caixa",
         m=(11.5, 2.54, 35.56), p=(16, -33, -150), cor="#1e1e1e", tipo="placa",
         fonte="14 × 2,54, um pino por fio"),
    dict(num=10, ref="—", nome="Conector do servo encaixado", forma="caixa",
         m=(14, 8.5, 4.5), p=(2, -10.44, -171), cor="#9a6b3f", tipo="solto",
         fonte="carcaça 3 vias, 14 de comprimento"),
    dict(num=11, ref="—", nome="Conectores dos jumpers encaixados", forma="caixa",
         m=(14, 3.5, 36.6), p=(2, -33.48, -150.5), cor="#9a6b3f", tipo="solto",
         fonte="carcaça 2,5 × 14 por fio"),
    dict(num=12, ref="J1", nome="P4 fêmea com borne", forma="caixa",
         m=(15, 35, 17), p=(-33, -4, -132), cor="#4a4a4a", tipo="solto",
         fonte="35 × 17 × 15 (anúncio KaBuM)"),
    dict(num=13, ref="—", nome="Plugue P4 da fonte encaixado", forma="cilindro",
         m=(5, 22, "+y"), p=(-25.5, 31, -123.5), cor="#707070", tipo="parede",
         fonte="Ø10, atravessa o furo F3"),
    dict(num=14, ref="U2", nome="Sensor de chama da saída", forma="caixa",
         m=(8, 15, 36), p=(18.5, -70.5, -126), cor="#2f6fd0", tipo="fora",
         fonte="KY-026: 36 × 15 × 8"),
    dict(num=15, ref="U3", nome="Sensor de chama do alarme", forma="caixa",
         m=(8, 15, 36), p=(-47, -7.5, -104), cor="#2f6fd0", tipo="parede",
         fonte="KY-026: 36 × 15 × 8"),
    dict(num=16, ref="D1", nome="LED RGB 5 mm", forma="cilindro",
         m=(2.9, 9, "+y"), p=(0, 40.5, -95), cor="#e8e8ff", tipo="parede",
         fonte="Ø5 (aba Ø5,8) × 8,6"),
    dict(num=17, ref="SW1", nome="Chave táctil 12×12, colada por fora", forma="caixa",
         m=(12, 4.3, 12), p=(-6, R_EXT, -116), cor="#5a5a5a", tipo="fora",
         fonte="12 × 12 × 4,3 (anúncio)"),
    dict(num=18, ref="BZ1", nome="Buzzer ativo 3,3 V", forma="cilindro",
         m=(6, 9.5, "+y"), p=(0, 38.5, -125), cor="#262626", tipo="parede",
         fonte="Ø12 × 7,5 a 9,5 → 9,5"),
]

# Contatos de propósito (pares de números): não entram na conta de folga.
# U1 é uma pilha: barras fêmea (5) embaixo da placa da ESP (2), módulo (3) e USB-C (4) em cima.
CONTATOS = {(1, n) for n in range(2, 10)} | {(2, 3), (2, 4), (2, 5), (3, 4), (3, 5), (4, 5),
                                              (8, 10), (9, 11), (12, 13)}
# Peças impressas com as quais um componente pode encostar. O que fica na face da placa
# está separado do suporte pela própria placa (1,6 mm).
CONTATOS_PECA = {(1, "suporte_placa"), (14, "extensao_funil")} | {(n, "suporte_placa") for n in range(2, 10)}

# Furos no tubo: (id, Ø, ângulo em graus, z do centro, para quê)
# Ângulo medido de cima: 0° = +X (direita), 90° = +Y (trás), 180° = −X (esquerda), 270° = −Y (saída).
FUROS = [
    ("F1", 8, 195, -12, "fios do servo saem (entre a borda do funil e a rampa)"),
    ("F2", 8, 195, -110, "fios do servo voltam, embaixo do funil"),
    ("F3", 12, 121.5, -123.5, "plugue da fonte até o J1"),
    ("F4", 5, 90, -95, "LED RGB (a aba de Ø5,8 fica por dentro)"),
    ("F5", 2, 90, -110, "2 furos a 6 mm um do outro: fios da chave"),
    ("F6", 3, 90, -125, "som do buzzer"),
    ("F7", 5.5, 180, -70, "fotodiodo do U3 olhando para fora"),
    ("F8", 3.3, 0, PARAFUSOS_Z[0], "parafuso M3 × 30 do suporte da placa"),
    ("F9", 3.3, 0, PARAFUSOS_Z[1], "parafuso M3 × 30 do suporte da placa"),
    ("F10", 6, 300, -100, "fios do U2 (sensor da saída) entram no tubo"),
]

# Rotas dos fios: (nome, cor, pontos (x, y, z), trechos por fora do tubo)
# Nenhuma rota passa pelo funil (caminho da ração) nem por dentro de outra peça.
ROTAS = [
    ("Servo: cabo do servo + 3 jumpers macho-fêmea", "#c0392b",
     [(-18.5, -20, -70), (-30, -15, -40), (-44, -12, -14), (-47.1, -12.6, -12), (-52.8, -14.1, -12),
      (-52.8, -14.1, -110), (-47.1, -12.6, -110), (-40, -12, -165), (2, -6.2, -168.7)], {3, 4}),
    ("Fonte 5 V: J1 → trilhos da placa (20 AWG)", "#e67e22",
     [(-25.5, -4, -123.5), (-25.5, -8, -123.5), (-25.5, -8, -158), (8, 5, -174), (27.5, 5, -174)], set()),
    ("Painel: LED, buzzer e chave (jumpers)", "#8e44ad",
     [(0, 33, -110), (0, 33, -155), (0, -31.7, -155), (2, -31.7, -145)], set()),
    ("U3: sensor do alarme (jumpers)", "#2471a3",
     [(-43, 0, -104), (-43, 0, -155), (0, -31.7, -155), (2, -31.7, -135)], set()),
    ("U2: sensor da saída (jumpers)", "#17a589",
     [(22.5, -63, -90), (27.4, -47.5, -100), (24, -42, -100), (8, -31.7, -106), (8, -31.7, -114)], {0}),
]

# Servo MG996R: o eixo é perpendicular à rampa (inclinada 30°). A boca (tampa do
# atuador, Ø46) é colada e parafusada no braço do servo e gira em volta desse eixo.
EIXO_SERVO = (0.5, 0.0, 0.866)
CENTRO_BOCA = (29 * 0.866 - 18 * 0.5, 0.0, -29 * 0.5 - 18 * 0.866)  # (u=29, w=-18) no plano da rampa
RAIO_BOCA = 23.0

# Barra 1×14: pino -> ligação (um fio por pino, sem emendas soltas)
PINOS_1X14 = ["U2 GND", "U2 VCC (3,3 V)", "U2 D0 → GPIO 9", "U3 GND", "U3 VCC (3,3 V)", "U3 D0 → GPIO 10",
              "LED R (via R1) ← GPIO 17", "LED G (via R2) ← GPIO 18", "LED B (via R3) ← GPIO 8", "LED catodo (GND)",
              "Buzzer + ← GPIO 16", "Buzzer − (GND)", "Chave ← GPIO 15", "Chave (GND)"]
PINOS_1X3 = ["GND (marrom)", "+5 V (vermelho)", "sinal ← GPIO 4 (laranja)"]
KEEPOUT_R = 5.0  # raio sem cobre em volta dos parafusos M3 que atravessam a placa
# Furos na extensão do funil (peça impressa), lado a lado: (id, Ø, x, y, z, para quê)
FUROS_EXTENSAO = [
    ("E1", 5, 18, -63, -108, "fotodiodo do U2"),
    ("E2", 5, -18, -63, -108, "luz do outro lado"),
]

PECAS_STL = ("funil_interno", "extensao_funil", "rampa_inclinada", "suporte_motor",
             "tampa_atuador", "suporte_placa", "suporte_rampa", "tampa_tubo")


def caixa_de(c):
    """Caixa envolvente (xmin, xmax, ymin, ymax, zmin, zmax) de um componente."""
    m, p = c["m"], c["p"]
    if c["forma"] == "caixa":
        return p[0], p[0] + m[0], p[1], p[1] + m[1], p[2], p[2] + m[2]
    r, h, eixo = m
    if eixo == "+z":
        return p[0] - r, p[0] + r, p[1] - r, p[1] + r, p[2], p[2] + h
    return p[0] - r, p[0] + r, p[1], p[1] + h, p[2] - r, p[2] + r


def ponto_furo(angulo, z, raio=R_EXT):
    a = math.radians(angulo)
    return raio * math.cos(a), raio * math.sin(a), z


# ======================= dentro do FreeCAD =======================
def no_freecad():
    import FreeCAD
    import Part  # noqa: F401  (registra os tipos Part::Box e Part::Cylinder)

    doc = FreeCAD.ActiveDocument
    if doc is None:
        raiz = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        doc = FreeCAD.openDocument(os.path.join(raiz, "hardware", "3d", "IoT3D.FCStd"))
    antigo = doc.getObject("Eletronica")
    if antigo:
        antigo.removeObjectsFromDocument()
        doc.removeObject("Eletronica")
    grupo = doc.addObject("App::DocumentObjectGroup", "Eletronica")
    for c in COMPONENTES:
        rotulo = f"{c['num']:02d}_{c['ref']}_{c['nome']}".replace("—", "").replace(" ", "_")
        if c["forma"] == "caixa":
            obj = doc.addObject("Part::Box", "Eletr")
            obj.Length, obj.Width, obj.Height = c["m"]
            obj.Placement = FreeCAD.Placement(FreeCAD.Vector(*c["p"]), FreeCAD.Rotation())
        else:
            obj = doc.addObject("Part::Cylinder", "Eletr")
            obj.Radius, obj.Height = c["m"][0], c["m"][1]
            # o cilindro nasce no eixo +Z; os de eixo +Y giram -90° em X
            giro = FreeCAD.Rotation() if c["m"][2] == "+z" else FreeCAD.Rotation(FreeCAD.Vector(1, 0, 0), -90)
            obj.Placement = FreeCAD.Placement(FreeCAD.Vector(*c["p"]), giro)
        obj.Label = rotulo
        grupo.addObject(obj)
        if FreeCAD.GuiUp:
            obj.ViewObject.ShapeColor = tuple(int(c["cor"][i:i + 2], 16) / 255 for i in (1, 3, 5))
    doc.recompute()
    FreeCAD.Console.PrintMessage(f"Eletronica: {len(COMPONENTES)} peças adicionadas. Confira e salve.\n")


# ======================= geometria (sem FreeCAD) =======================
def triangulos(caminho):
    d = open(caminho, "rb").read()
    n = struct.unpack("<I", d[80:84])[0]
    tri = []
    for i in range(n):
        f = struct.unpack("<12f", d[84 + i * 50:84 + i * 50 + 48])
        tri.append((f[3:6], f[6:9], f[9:12]))
    return tri


def amostrar(tri, n=4):
    """Pontos espalhados sobre a superfície dos triângulos (para medir folgas)."""
    pts = []
    for a, b, c in tri:
        for u in range(n + 1):
            for w in range(n + 1 - u):
                pts.append(tuple((a[q] * (n - u - w) + b[q] * u + c[q] * w) / n for q in range(3)))
    return pts


def dist_caixa_ponto(cx, p):
    d = [max(cx[2 * i] - p[i], 0, p[i] - cx[2 * i + 1]) for i in range(3)]
    return math.sqrt(sum(v * v for v in d))


def dist_caixas(a, b):
    d = [max(a[2 * i] - b[2 * i + 1], 0, b[2 * i] - a[2 * i + 1]) for i in range(3)]
    return math.sqrt(sum(v * v for v in d))


def raio_max(cx):
    return max(math.hypot(x, y) for x in cx[0:2] for y in cx[2:4])


def raio_min(cx):
    x = min(max(0, cx[0]), cx[1])
    y = min(max(0, cx[2]), cx[3])
    return math.hypot(x, y)


def calcular_folgas(pecas_pts):
    """Folga mínima de cada componente e com o quê. Devolve {índice: (mm, com o quê)}."""
    folgas = {}
    for i, c in enumerate(COMPONENTES):
        cx = caixa_de(c)
        melhor = (1e9, "")
        for nome, pts in pecas_pts.items():
            if (c["num"], nome) in CONTATOS_PECA:
                continue
            d = min(dist_caixa_ponto(cx, p) for p in pts)
            melhor = min(melhor, (d, nome.replace("_", " ")))
        if c["tipo"] not in ("parede", "fora"):
            melhor = min(melhor, (R_INT - raio_max(cx), "parede do tubo"))
        if c["tipo"] == "fora" and c["num"] != 17:
            melhor = min(melhor, (raio_min(cx) - R_EXT, "parede do tubo"))
        for j, o in enumerate(COMPONENTES):
            if j == i or o["num"] == c["num"]:
                continue
            par = tuple(sorted((c["num"], o["num"])))
            if par in CONTATOS:
                continue
            melhor = min(melhor, (dist_caixas(cx, caixa_de(o)), f"{o['ref'] if o['ref'] != '—' else o['nome'].split(',')[0]}"))
        if c["tipo"] in ("placa", "base"):  # porcas M3 (Ø6,4) dos parafusos do suporte, na face da placa
            for z in PARAFUSOS_Z:
                porca = (PLACA_X - 2.4, PLACA_X, -3.2, 3.2, z - 3.2, z + 3.2)
                if c["tipo"] == "placa":
                    melhor = min(melhor, (dist_caixas(cx, porca), "porca do suporte"))
        folgas[i] = melhor
    return folgas


def conferir_rotas(pecas_pts):
    """Distância mínima de cada rota (trechos por dentro) até o funil e os componentes."""
    saida = []
    for nome, _, pts, fora in ROTAS:
        amostras = []
        for k in range(len(pts) - 1):
            if k in fora:
                continue
            a, b = pts[k], pts[k + 1]
            n = max(2, int(math.dist(a, b) / 2))
            amostras += [tuple(a[q] + (b[q] - a[q]) * i / n for q in range(3)) for i in range(n + 1)]
        d_funil = min((min(math.dist(p, q) for q in pts_), nome_.replace("_", " "))
                      for nome_, pts_ in pecas_pts.items() if nome_ not in ("tampa_tubo",)
                      for p in amostras[2:-2:2])
        # distância até o cilindro que a boca varre ao girar
        def ate_boca(p):
            d = [p[i] - CENTRO_BOCA[i] for i in range(3)]
            ax = sum(d[i] * EIXO_SERVO[i] for i in range(3))
            rad = math.sqrt(max(0.0, sum(v * v for v in d) - ax * ax))
            return max(rad - RAIO_BOCA, abs(ax) - 16, 0.0) if abs(ax) > 16 else max(rad - RAIO_BOCA, 0.0)
        d_boca = min(ate_boca(p) for p in amostras)
        # componentes: só os trechos do meio (o primeiro e o último são onde o fio se liga)
        miolo = []
        for k in range(1, len(pts) - 2):
            if k in fora:
                continue
            a, b = pts[k], pts[k + 1]
            n = max(2, int(math.dist(a, b) / 2))
            miolo += [tuple(a[q] + (b[q] - a[q]) * i / n for q in range(3)) for i in range(n + 1)]
        d_comp = min((dist_caixa_ponto(caixa_de(c), p), c["nome"]) for c in COMPONENTES for p in miolo
                     if c["num"] not in (1,)) if miolo else (99, "")
        comp = sum(math.dist(pts[k], pts[k + 1]) for k in range(len(pts) - 1))
        saida.append((nome, comp, d_funil, d_comp, d_boca))
    return saida


def envoltoria(pts):
    pts = sorted(set(pts))
    if len(pts) < 3:
        return pts

    def lado(o, a, b):
        return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])

    baixo, cima = [], []
    for p in pts:
        while len(baixo) >= 2 and lado(baixo[-2], baixo[-1], p) <= 0:
            baixo.pop()
        baixo.append(p)
    for p in reversed(pts):
        while len(cima) >= 2 and lado(cima[-2], cima[-1], p) <= 0:
            cima.pop()
        cima.append(p)
    return baixo[:-1] + cima[:-1]


# ======================= desenho =======================
class Folha:
    def __init__(self, largura=1600, altura=1131):
        self.L, self.A = largura, altura
        self.s = [f'<rect width="{largura}" height="{altura}" fill="#fff"/>']

    def texto(self, x, y, t, tam=12, cor="#000", ancora="start", peso="normal"):
        self.s.append(f'<text x="{x:.1f}" y="{y:.1f}" font-size="{tam}" fill="{cor}" '
                      f'text-anchor="{ancora}" font-weight="{peso}">{t}</text>')

    def linha(self, x1, y1, x2, y2, cor="#000", w=1, extra=""):
        self.s.append(f'<line x1="{x1:.1f}" y1="{y1:.1f}" x2="{x2:.1f}" y2="{y2:.1f}" '
                      f'stroke="{cor}" stroke-width="{w}"{extra}/>')

    def ret(self, x0, y0, x1, y1, fundo="none", borda="#000", w=1, op=1.0, extra=""):
        self.s.append(f'<rect x="{min(x0, x1):.1f}" y="{min(y0, y1):.1f}" width="{abs(x1 - x0):.1f}" '
                      f'height="{abs(y1 - y0):.1f}" fill="{fundo}" fill-opacity="{op}" stroke="{borda}" '
                      f'stroke-width="{w}"{extra}/>')

    def circulo(self, x, y, r, fundo="none", borda="#000", w=1):
        self.s.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{r:.1f}" fill="{fundo}" stroke="{borda}" stroke-width="{w}"/>')

    def silhueta(self, tri2d, fundo, borda):
        """União exata dos triângulos projetados (todos no mesmo sentido, regra nonzero)."""
        partes = []
        for a, b, c in tri2d:
            area = (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0])
            if abs(area) < 2.0:  # área dobrada em px²
                continue
            if area < 0:
                b, c = c, b
            partes.append("M%.0f %.0fL%.0f %.0fL%.0f %.0fZ" % (a[0], a[1], b[0], b[1], c[0], c[1]))
        # contorno por baixo (traço largo) + preenchimento por cima = silhueta com borda
        d = "".join(partes)
        self.s.append(f'<path d="{d}" fill="{borda}" stroke="{borda}" stroke-width="1.6" stroke-linejoin="round"/>')
        self.s.append(f'<path d="{d}" fill="{fundo}" stroke="none"/>')

    def bolha(self, x, y, n):
        self.circulo(x, y, 7.5, "#fff", "#000", 0.9)
        self.texto(x, y + 3.6, str(n), 9.5, "#000", "middle", "bold")

    def cota(self, x1, y1, x2, y2, rotulo, desloc=0, cor="#c00000"):
        """Linha de cota com setas, deslocada perpendicularmente."""
        dx, dy = x2 - x1, y2 - y1
        L = math.hypot(dx, dy) or 1
        nx, ny = -dy / L * desloc, dx / L * desloc
        a = (x1 + nx, y1 + ny)
        b = (x2 + nx, y2 + ny)
        self.linha(x1, y1, a[0], a[1], cor, 0.6)
        self.linha(x2, y2, b[0], b[1], cor, 0.6)
        self.linha(a[0], a[1], b[0], b[1], cor, 0.9)
        for p, q in ((a, b), (b, a)):
            ux, uy = (q[0] - p[0]) / L * 6, (q[1] - p[1]) / L * 6
            self.s.append(f'<polygon points="{p[0]:.1f},{p[1]:.1f} {p[0] + ux - uy * 0.4:.1f},{p[1] + uy + ux * 0.4:.1f} '
                          f'{p[0] + ux + uy * 0.4:.1f},{p[1] + uy - ux * 0.4:.1f}" fill="{cor}"/>')
        mx, my = (a[0] + b[0]) / 2, (a[1] + b[1]) / 2
        if abs(dx) >= abs(dy):
            self.texto(mx, my - 4, rotulo, 10.5, cor, "middle", "bold")
        else:
            self.s.append(f'<text x="{mx - 4:.1f}" y="{my:.1f}" font-size="10.5" fill="{cor}" font-weight="bold" '
                          f'text-anchor="middle" transform="rotate(-90 {mx - 4:.1f} {my:.1f})">{rotulo}</text>')

    def salvar(self, caminho, png):
        svg = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {self.L} {self.A}" width="{self.L}" '
               f'height="{self.A}" font-family="Arial, Helvetica, sans-serif">\n' + "\n".join(self.s) + "\n</svg>\n")
        open(caminho, "w", encoding="utf-8", newline="\n").write(svg)
        print(caminho, len(svg))
        if not png:
            return
        chrome = next(filter(None, map(shutil.which, ("google-chrome", "chromium", "chromium-browser"))), None)
        if not chrome:
            sys.exit("Chrome/Chromium não encontrado: abra o SVG no navegador e exporte o PNG.")
        arq = os.path.abspath(caminho[:-4] + ".png")
        subprocess.run([chrome, "--headless=new", "--disable-gpu", "--hide-scrollbars",
                        "--force-device-scale-factor=1.5", f"--window-size={self.L},{self.A + 200}",
                        f"--screenshot={arq}", "file://" + os.path.abspath(caminho)],
                       check=True, capture_output=True)
        from PIL import Image
        Image.open(arq).crop((0, 0, int(self.L * 1.5), int(self.A * 1.5))).save(arq, optimize=True)
        print(arq)


def br(v, casas=1):
    return f"{v:.{casas}f}".replace(".", ",")


def rotulo_med(c):
    m = c["m"]
    if c["forma"] == "caixa":
        return "×".join(f"{v:g}".replace(".", ",") for v in m)
    return f"Ø{2 * m[0]:g} × {m[1]:g}".replace(".", ",")


def folha_vistas(tri, folgas, png):
    f = Folha()
    f.texto(40, 50, "Lulu · posicionamento da eletrônica dentro do tubo", 20, "#000", "start", "bold")
    f.texto(40, 72, "Coordenadas do IoT3D.FCStd. Alturas medidas a partir da base do tubo. "
            "Componentes desenhados pelo envelope (maior medida publicada).", 12, "#444")
    E, ZTOPO = 2.3, 140
    vistas = [
        ("A · De trás (olhando de +Y para −Y)", 200, 170, lambda x, y, z: (-x, -z), lambda cx: -cx[3], "y"),
        ("B · Lado esquerdo (olhando de −X para +X)", 530, 170, lambda x, y, z: (-y, -z), lambda cx: cx[0], "x"),
    ]
    for titulo, cx0, y0, proj, dist, parede in vistas:
        def tela(x, y, z, proj=proj, cx0=cx0, y0=y0):
            u, v = proj(x, y, z)
            return cx0 + u * E, y0 + (v + ZTOPO) * E
        f.texto(cx0, y0 - 52, titulo, 14, "#000", "middle", "bold")
        for lado in (-1, 1):
            px, py = (lado * R_EXT, 0) if parede == "y" else (0, lado * R_EXT)
            a, b = tela(px, py, TOPO_Z), tela(px, py, BASE_Z)
            f.linha(a[0], a[1], b[0], b[1], "#7a9bb5", 2)
        _, yc = tela(0, 0, BASE_Z + CAP_MM)
        _, yb = tela(0, 0, BASE_Z - 4)
        f.ret(cx0 - (R_EXT + 3) * E, yc, cx0 + (R_EXT + 3) * E, yb, "#dfe8ef", "#7a9bb5", 1, 0.55, ' stroke-dasharray="5 3"')
        f.texto(cx0 + (R_EXT + 6) * E, yc + 14, "cap", 10, "#557")
        for nome in PECAS_STL:
            t2 = [tuple(tela(*p) for p in t) for t in tri[nome]]
            funil = "funil" in nome
            f.silhueta(t2, "#f6e2d3" if funil else ("#cfe3cf" if nome == "suporte_placa" else "#ececec"),
                       "#b08a70" if funil else "#9a9a9a")
        rx = cx0 - (R_EXT + 22) * E
        for h in range(0, 301, 50):
            _, yy = tela(0, 0, BASE_Z + h)
            f.linha(rx, yy, rx + 8, yy, "#666")
            f.texto(rx - 4, yy + 4, str(h), 10, "#666", "end")
        _, ya = tela(0, 0, BASE_Z)
        _, yt = tela(0, 0, BASE_Z + 300)
        f.linha(rx + 4, ya, rx + 4, yt, "#666")
        f.texto(rx - 4, yt - 12, "mm", 10, "#666", "end")
        for c in sorted(COMPONENTES, key=lambda c: -dist(caixa_de(c))):
            cx = caixa_de(c)
            cantos = [tela(x, y, z) for x in cx[0:2] for y in cx[2:4] for z in cx[4:6]]
            us, vs = [p[0] for p in cantos], [p[1] for p in cantos]
            f.ret(min(us), min(vs), max(us), max(vs), c["cor"], "#000", 0.8, 0.92)
        for fid, d, ang, z, _ in FUROS:
            x, y, _ = ponto_furo(ang, z)
            if (parede == "y" and y > 15) or (parede == "x" and x < -15):
                px, py = tela(x, y, z)
                f.circulo(px, py, max(d / 2 * E, 2.5), "none", "#c00000", 1.6)
                f.texto(px + 8, py - 5, fid, 10, "#c00000", "start", "bold")
        for _, cor, pts, fora in ROTAS:
            for k in range(len(pts) - 1):
                a, b = tela(*pts[k]), tela(*pts[k + 1])
                f.linha(a[0], a[1], b[0], b[1], cor, 2, ' stroke-dasharray="7 4"' if k in fora else "")
        if parede == "x":
            f.texto(cx0 + (R_EXT + 2) * E, y0 - 28, "saída / pote (−Y) →", 11, "#557", "end")
        else:
            f.texto(cx0, y0 - 28, "← direita (+X)        esquerda (−X) →", 11, "#557", "middle")
        vistos = set()
        for c in COMPONENTES:
            if c["num"] in vistos:
                continue
            vistos.add(c["num"])
            cx = caixa_de(c)
            u, v = tela((cx[0] + cx[1]) / 2, (cx[2] + cx[3]) / 2, (cx[4] + cx[5]) / 2)
            f.bolha(u, v, c["num"])

    # C: corte de cima, abaixo do funil
    cx0, cy0, e = 1015, 560, 3.1
    f.texto(cx0, 118, "C · De cima, abaixo do funil (z &lt; −85)", 14, "#000", "middle", "bold")
    f.circulo(cx0, cy0, R_EXT * e, "#f4f8fb", "#7a9bb5", 2)
    f.circulo(cx0, cy0, R_INT * e, "#fff", "#7a9bb5", 1.2)

    def tela_c(x, y):
        return cx0 + x * e, cy0 - y * e

    for nome in ("funil_interno", "extensao_funil", "suporte_placa"):
        t2 = [tuple(tela_c(p[0], p[1]) for p in t) for t in tri[nome] if max(p[2] for p in t) < -85]
        f.silhueta(t2, "#f6e2d3" if "funil" in nome else "#cfe3cf", "#b08a70" if "funil" in nome else "#6a9a6a")
    for c in sorted(COMPONENTES, key=lambda c: caixa_de(c)[5]):
        cx = caixa_de(c)
        a, b = tela_c(cx[0], cx[3]), tela_c(cx[1], cx[2])
        f.ret(a[0], a[1], b[0], b[1], c["cor"], "#000", 0.8, 0.92)
    for _, cor, pts, fora in ROTAS:
        for k in range(len(pts) - 1):
            if pts[k][2] > -85 and pts[k + 1][2] > -85:
                continue
            a, b = tela_c(*pts[k][:2]), tela_c(*pts[k + 1][:2])
            f.linha(a[0], a[1], b[0], b[1], cor, 2, ' stroke-dasharray="7 4"' if k in fora else "")
    for fid, d, ang, z, _ in FUROS:
        x, y, _ = ponto_furo(ang, z, R_EXT + 4)
        px, py = tela_c(x, y)
        f.texto(px, py + 4, fid, 9.5, "#c00000", "middle", "bold")
    vistos = set()
    for c in COMPONENTES:
        if c["num"] in vistos or c["num"] in (5,):
            continue
        vistos.add(c["num"])
        cx = caixa_de(c)
        u, v = tela_c((cx[0] + cx[1]) / 2, (cx[2] + cx[3]) / 2)
        f.bolha(u, v, c["num"])
    f.texto(cx0, cy0 - R_EXT * e - 28, "trás (+Y)", 12, "#557", "middle", "bold")
    f.texto(cx0, cy0 + R_EXT * e + 108, "frente (−Y): saída da ração para o pote", 12, "#557", "middle", "bold")
    f.texto(cx0 + R_EXT * e + 26, cy0 + 4, "+X", 12, "#557", "start", "bold")
    f.texto(cx0 - R_EXT * e - 26, cy0 + 4, "−X", 12, "#557", "end", "bold")
    for k, (nome, cor, _, _) in enumerate(ROTAS):
        yy = cy0 + R_EXT * e + 172 + k * 17
        f.linha(cx0 - 230, yy - 4, cx0 - 200, yy - 4, cor, 2.5)
        f.texto(cx0 - 192, yy, nome, 11, "#222")
    f.texto(cx0 - 230, cy0 + R_EXT * e + 172 + len(ROTAS) * 17 + 2, "Fios: linha cheia por dentro, tracejada por fora do tubo.", 10.5, "#555")
    f.texto(cx0, cy0 + R_EXT * e + 130, "Verde: suporte da placa. A placa encosta na aba lisa; 2 parafusos M3 × 30", 11, "#444", "middle")
    f.texto(cx0, cy0 + R_EXT * e + 146, "atravessam parede, suporte e placa (furos F8 e F9), com a porca na face da placa.", 11, "#444", "middle")

    # tabela de componentes com folga
    tx, ty = 1235, 118
    f.texto(tx, ty, "Componentes (X × Y × Z, mm)", 14, "#000", "start", "bold")
    f.texto(tx, ty + 16, f"folga = menor distância até funil, parede ou outra peça (mín. {FOLGA_MIN:g} mm)", 10, "#555")
    vistos, lin = set(), 0
    for i, c in enumerate(COMPONENTES):
        if c["num"] in vistos:
            continue
        vistos.add(c["num"])
        cx = caixa_de(c)
        mm, com = min(folgas[j] for j, o in enumerate(COMPONENTES) if o["num"] == c["num"])
        yy = ty + 40 + lin * 49
        f.ret(tx, yy - 11, tx + 12, yy + 1, c["cor"], "#000", 0.6)
        ref = "" if c["ref"] == "—" else c["ref"] + " · "
        f.texto(tx + 18, yy, f"{c['num']}. {ref}{c['nome']}", 11.5)
        f.texto(tx + 18, yy + 14, f"{rotulo_med(c)} · altura {cx[4] - BASE_Z:.0f} a {cx[5] - BASE_Z:.0f} mm", 10.5, "#555")
        f.texto(tx + 18, yy + 28, f"folga {mm:.1f} mm ({com})".replace(".", ","), 10.5, "#1d7a2e" if mm >= FOLGA_MIN else "#c00000")
        lin += 1
    f.texto(40, 1100, "Gerado por ferramentas/eletronica_3d.py · confira as peças recebidas com paquímetro antes de furar · "
            "furos e cotas da placa na folha eletronica_furacao.", 11, "#555")
    f.salvar(os.path.join("hardware", "3d", "imagens", "eletronica_posicionamento.svg"), png)


def folha_furacao(tri, rotas, png):
    f = Folha(1600, 1560)
    f.texto(40, 50, "Lulu · furação do tubo e montagem da placa", 20, "#000", "start", "bold")
    f.texto(40, 72, "Cotas em mm. Confira as peças recebidas com paquímetro antes de furar ou soldar.", 12, "#444")

    # ---------- placa vista pela face dos componentes (de dentro do tubo, olhando para +X) ----------
    e = 4.6
    ox, oy = 110, 600  # canto inferior esquerdo da placa na tela
    f.texto(ox + 35 * e, 112, "Placa perfurada 70 × 90 (face dos componentes)", 14, "#000", "middle", "bold")
    f.texto(ox + 35 * e, 130, "vista de dentro do tubo; a borda esquerda fica do lado de trás (+Y)", 11, "#555", "middle")

    def tp(y, z):  # coordenadas da placa: a = 35 − y (da borda esquerda), b = z + 175 (da borda de baixo)
        return ox + (35 - y) * e, oy - (z + 175) * e

    a0, b0 = tp(35, -175)
    a1, b1 = tp(-35, -85)
    f.ret(a0, b0, a1, b1, "#e8cf9f", "#8a6a3a", 1.4)
    k = 0
    while 1.5 + k * 2.54 < 70:  # furos da placa (passo 2,54)
        j = 0
        while 1.5 + j * 2.54 < 90:
            f.s.append(f'<circle cx="{a0 + (1.5 + k * 2.54) * e:.1f}" cy="{b0 - (1.5 + j * 2.54) * e:.1f}" r="0.9" fill="#a8885a"/>')
            j += 1
        k += 1
    for c in COMPONENTES:
        if c["tipo"] not in ("placa",) and c["num"] not in (10, 11):
            continue
        cx = caixa_de(c)
        p, q = tp(cx[3], cx[5]), tp(cx[2], cx[4])
        f.ret(p[0], p[1], q[0], q[1], c["cor"], "#000", 0.8, 0.55 if c["num"] in (10, 11) else 0.9,
              ' stroke-dasharray="4 3"' if c["num"] in (10, 11) else "")
    for z in PARAFUSOS_Z:
        px, py = tp(0, z)
        f.s.append(f'<circle cx="{px:.1f}" cy="{py:.1f}" r="{KEEPOUT_R * e:.1f}" fill="#fff0f0" stroke="#c00000" '
                   f'stroke-width="1" stroke-dasharray="4 3"/>')
        f.circulo(px, py, 3.2 * e, "#ffffff", "#c00000", 1.2)
        f.circulo(px, py, 1.65 * e, "#c00000", "#c00000", 1)
    vistos = set()
    for c in COMPONENTES:
        if c["tipo"] != "placa" or c["num"] in vistos:
            continue
        vistos.add(c["num"])
        cx = caixa_de(c)
        u, v = tp((cx[2] + cx[3]) / 2, (cx[4] + cx[5]) / 2)
        f.bolha(u, v, c["num"])
    # cotas, todas por fora da placa
    def cota_h(xa, xb, y, rot):
        f.linha(xa, y, xb, y, "#c00000", 0.9)
        for x in (xa, xb):
            f.linha(x, y - 5, x, y + 5, "#c00000", 0.9)
        f.texto((xa + xb) / 2, y - 4, rot, 10.5, "#c00000", "middle", "bold")

    def cota_v(x, ya, yb, rot):
        f.linha(x, ya, x, yb, "#c00000", 0.9)
        for y in (ya, yb):
            f.linha(x - 5, y, x + 5, y, "#c00000", 0.9)
        f.s.append(f'<text x="{x + 13:.1f}" y="{(ya + yb) / 2:.1f}" font-size="10.5" fill="#c00000" font-weight="bold" '
                   f'text-anchor="middle" transform="rotate(-90 {x + 13:.1f} {(ya + yb) / 2:.1f})">{rot}</text>')

    cota_h(a0, a1, b0 + 22, "70")
    cota_v(a0 - 34, b0, b1, "90")
    px, _ = tp(0, 0)
    cota_h(a0, px, b1 - 16, "35")
    for k, z in enumerate(PARAFUSOS_Z):
        _, py = tp(0, z)
        f.linha(px, py, a1 + 22 + k * 30, py, "#c00000", 0.5, ' stroke-dasharray="3 2"')
        cota_v(a1 + 22 + k * 30, b0, py, br(z + 175))
    f.texto(ox + 35 * e, oy + 70, "Furos Ø3,3 dos parafusos M3 (vermelho) em a = 35, b = 44,5 e 59,5: fure a placa nesses pontos.", 11, "#333", "middle")
    f.texto(ox + 35 * e, oy + 86, f"Tracejado vermelho = área sem cobre (Ø{2 * KEEPOUT_R:g}): raspe as ilhas e não passe trilha aí.", 11, "#333", "middle")
    f.texto(ox + 35 * e, oy + 102, "Tracejado = conectores encaixados (avançam 14 mm para dentro).", 11, "#333", "middle")
    # posições dos componentes na placa
    f.texto(40, oy + 140, "Posição na placa (a = da borda esquerda, b = da borda de baixo; altura = quanto avança da face)", 12.5, "#000", "start", "bold")
    lin = 0
    vistos = set()
    for c in COMPONENTES:
        if c["tipo"] != "placa" or c["num"] in vistos:
            continue
        vistos.add(c["num"])
        partes = [o for o in COMPONENTES if o["num"] == c["num"]]
        txts = []
        for o in partes:
            cx = caixa_de(o)
            txts.append(f"a {br(35 - cx[3])}–{br(35 - cx[2])}, b {br(cx[4] + 175)}–{br(cx[5] + 175)}")
        cx = caixa_de(c)
        altura = PLACA_X - cx[0]
        f.texto(40, oy + 162 + lin * 18, f"{c['num']}. {c['ref'] + ' · ' if c['ref'] != '—' else ''}{c['nome']}: "
                f"{' | '.join(txts)}; altura {br(altura)}", 11)
        lin += 1

    # ---------- planificação da parede ----------
    circ = 2 * math.pi * R_EXT
    px0, py0, es = 660, 700, 2.85
    hmax = 180
    f.texto(px0 + circ * es / 2, 112, "Parede do tubo planificada (vista de fora)", 14, "#000", "middle", "bold")
    f.texto(px0 + circ * es / 2, 132, f"enrole uma fita de papel no tubo, marque a linha da saída e meça s pela volta "
            f"(circunferência {circ:.1f} mm)".replace(".", ","), 11, "#555", "middle")

    def tw(ang, z):
        s = R_EXT * math.radians((ang - 270) % 360)
        return px0 + s * es, py0 - (z - BASE_Z) * es

    xa, ya = tw(270, BASE_Z)
    xb, yb2 = px0 + circ * es, py0 - hmax * es
    f.ret(xa, ya, xb, yb2, "#f4f8fb", "#7a9bb5", 1.4)
    f.ret(xa, ya, xb, py0 - CAP_MM * es, "#dfe8ef", "#7a9bb5", 1, 0.9, ' stroke-dasharray="5 3"')
    f.texto(xb - 6, py0 - CAP_MM * es + 14, f"cap ({CAP_MM} mm): sem furos", 10.5, "#557", "end")
    for h in range(0, hmax + 1, 20):
        yy = py0 - h * es
        f.linha(xa - 6, yy, xa, yy, "#666")
        f.texto(xa - 9, yy + 4, str(h), 10, "#666", "end")
        f.linha(xa, yy, xb, yy, "#e3e9ee", 0.6)
    f.texto(xa - 9, yb2 - 10, "altura", 10, "#666", "end")
    for ang, nome, anc in ((270, "saída", "start"), (0, "direita (+X)", "middle"), (90, "trás (+Y)", "middle"),
                           (180, "esquerda (−X)", "middle"), (269.999, "saída", "end")):
        x, _ = tw(ang, 0)
        s = R_EXT * math.radians((ang - 270) % 360)
        f.linha(x, ya, x, yb2, "#7a9bb5", 1, ' stroke-dasharray="3 3"')
        f.texto(x, ya + 16, f"{s:.0f}", 10, "#666", anc)
        f.texto(x, yb2 - 8, nome, 10.5, "#557", anc, "bold")
    f.texto(px0 + circ * es / 2, ya + 32, "s (mm), medido pela volta a partir do centro da saída, no sentido saída → direita → trás", 10.5, "#666", "middle")
    # furo da saída (já existente): contorno da extensão onde ela atravessa a parede
    pts = []
    for t in tri["extensao_funil"]:
        for p in t:
            if abs(math.hypot(p[0], p[1]) - R_EXT) < 2.0:
                ang = math.degrees(math.atan2(p[1], p[0])) % 360
                pts.append(tw(ang, p[2]))
    # a saída fica em volta de 270°: divide em dois lados da folha
    esq = [p for p in pts if p[0] < px0 + circ * es / 2]
    dir_ = [(p[0] - circ * es, p[1]) for p in pts if p[0] >= px0 + circ * es / 2]
    contorno = envoltoria(esq + dir_)
    f.s.append(f'<clipPath id="parede"><rect x="{xa:.1f}" y="{yb2:.1f}" width="{xb - xa:.1f}" height="{ya - yb2:.1f}"/></clipPath>')
    for desloc in (0, circ * es):
        f.s.append('<polygon clip-path="url(#parede)" points="' + " ".join(f"{x + desloc:.1f},{y:.1f}" for x, y in contorno) +
                   '" fill="#f6e2d3" stroke="#b08a70" stroke-width="1.2"/>')
    _, ysai = tw(270, -102)
    f.texto(xa + 8, ysai - 30, "S (já existe)", 10, "#b08a70", "start", "bold")
    for fid, d, ang, z, _ in FUROS:
        x, y = tw(ang, z)
        if fid == "F5":
            for dd in (-3, 3):
                f.circulo(x + dd * es, y, d / 2 * es, "#fff", "#c00000", 1.4)
        else:
            f.circulo(x, y, d / 2 * es, "#fff", "#c00000", 1.4)
        f.texto(x + d / 2 * es + 4, y - 4, fid, 11, "#c00000", "start", "bold")
    # SW1 colada por fora (contorno)
    x, y = tw(90, -110)
    f.ret(x - 6 * es, y - 6 * es, x + 6 * es, y + 6 * es, "none", "#5a5a5a", 1, 1, ' stroke-dasharray="3 2"')
    f.texto(x - 6 * es - 4, y + 4, "SW1", 10, "#5a5a5a", "end")

    # tabela de furos
    tx, ty = 660, 800
    f.texto(tx, ty, "Furos no tubo", 13, "#000", "start", "bold")
    cab = ("Furo", "Ø (mm)", "ângulo", "s (mm)", "altura (mm)", "para quê")
    cols = (0, 52, 112, 172, 232, 318)
    for c0, t in zip(cols, cab):
        f.texto(tx + c0, ty + 22, t, 10.5, "#000", "start", "bold")
    f.linha(tx, ty + 27, tx + 900, ty + 27, "#999", 0.8)
    for i, (fid, d, ang, z, para) in enumerate(FUROS):
        yy = ty + 44 + i * 18
        s = R_EXT * math.radians((ang - 270) % 360)
        vals = (fid, br(d, 1).replace(",0", ""), br(ang, 1).replace(",0", "") + "°", f"{s:.0f}", br(z - BASE_Z), para)
        for c0, t in zip(cols, vals):
            f.texto(tx + c0, yy, t, 10.5, "#c00000" if c0 == 0 else "#000", "start", "bold" if c0 == 0 else "normal")
    yy = ty + 44 + len(FUROS) * 18 + 10
    f.texto(tx, yy, "Na extensão do funil (peça impressa): " + "; ".join(
        f"{fid} Ø{d} a {z - BASE_Z:.0f} mm de altura e {abs(y) - R_EXT:.0f} mm para fora do tubo, {para}"
        for fid, d, x, y, z, para in FUROS_EXTENSAO), 10.5, "#333")
    f.texto(tx, yy + 18, f"Parafusos do suporte: M3 × 30 (parede 2 + suporte 19,6 + placa 1,6 + porca 2,4 = 25,6 mm).", 10.5, "#333")

    # ---------- ligações ----------
    y0 = 1130
    f.linha(40, y0 - 30, 1560, y0 - 30, "#bbb", 1)
    f.texto(40, y0, "Barra 1×14 (de cima para baixo)", 13, "#000", "start", "bold")
    for i, nome in enumerate(PINOS_1X14):
        f.texto(40, y0 + 22 + i * 17, f"{i + 1:2d}", 11, "#c00000", "start", "bold")
        f.texto(64, y0 + 22 + i * 17, nome, 11)
    f.texto(330, y0, "Barra 1×3 do servo", 13, "#000", "start", "bold")
    for i, nome in enumerate(PINOS_1X3):
        f.texto(330, y0 + 22 + i * 17, f"{i + 1}", 11, "#c00000", "start", "bold")
        f.texto(348, y0 + 22 + i * 17, nome, 11)
    f.texto(330, y0 + 86, "Marque o lado do marrom (GND) na placa:", 10.5, "#555")
    f.texto(330, y0 + 101, "invertido, o servo recebe 5 V no sinal.", 10.5, "#555")
    f.texto(330, y0 + 130, "Fios (comprimento da rota + 30 %)", 13, "#000", "start", "bold")
    for i, (nome, comp, *_r) in enumerate(rotas):
        f.texto(330, y0 + 152 + i * 17, f"{br(comp * 1.3 / 10 + 3, 0)} cm · {nome}", 11)
    f.texto(330, y0 + 152 + len(rotas) * 17 + 4, "Jumpers do kit têm 20 cm: o servo usa o cabo dele + 3 jumpers M-F.", 10.5, "#555")
    cx_ = 820
    f.texto(cx_, y0, "Contra curto-circuito", 13, "#c00000", "start", "bold")
    itens = [
        "Parafusos M3 atravessam a placa: raspe as ilhas de cobre no círculo tracejado e use arruela isolante.",
        "Corte os terminais soldados a no máximo 2 mm e cubra o verso da placa com fita isolante (fica a 3 mm da parede).",
        "Bornes do J1 (+5 V e GND expostos): aperte os fios, cubra com termo-retrátil ou fita e prenda o J1 com abraçadeira.",
        "Trilhos +5 V e GND da placa: deixe ao menos uma coluna de furos livre entre eles e qualquer outro sinal.",
        "C1 tem polaridade: faixa (−) no GND. Invertido ele esquenta e pode estourar.",
        "LED e buzzer: termo-retrátil em cada perna soldada; a perna longa do LED é o catodo (GND).",
        "U2 e U3: cole com fita dupla-face de espuma, que isola o verso do módulo.",
        "Prenda os fios com abraçadeira nas paredes; nenhum fio passa pelo funil, pela saída S ou pela boca do servo.",
        "Barras macho: pinos sem uso ficam cobertos (corte a barra no tamanho certo: 1×14 e 1×3).",
        "Antes de ligar a fonte: com o multímetro, +5 V e GND não podem ter continuidade.",
    ]
    for i, txt in enumerate(itens):
        f.texto(cx_, y0 + 22 + i * 30, f"{i + 1}.", 11, "#c00000", "start", "bold")
        palavras, lin, linhas = txt.split(), "", []
        for p in palavras:
            if len(lin) + len(p) > 100:
                linhas.append(lin)
                lin = ""
            lin += p + " "
        linhas.append(lin)
        for k, l in enumerate(linhas):
            f.texto(cx_ + 18, y0 + 22 + i * 30 + k * 13, l.strip(), 11)
    f.salvar(os.path.join("hardware", "3d", "imagens", "eletronica_furacao.svg"), png)


def desenhar(png=False):
    stl = os.path.join(os.getcwd(), "hardware", "3d", "stl")
    tri = {nome: triangulos(os.path.join(stl, nome + ".stl")) for nome in PECAS_STL}
    pontos = {nome: amostrar(t) for nome, t in tri.items()}
    folgas = calcular_folgas(pontos)
    ruins = []
    for i, c in enumerate(COMPONENTES):
        mm, com = folgas[i]
        print(f"{c['num']:2d} {c['ref']:6s} {c['nome'][:44]:44s} folga {mm:6.1f} mm  ({com})")
        if mm < FOLGA_MIN:
            ruins.append(c["nome"])
    if ruins:
        sys.exit(f"Folga abaixo de {FOLGA_MIN} mm: {', '.join(ruins)}")
    rotas = conferir_rotas(pontos)
    for nome, comp, peca, comp_, boca in rotas:
        print(f"rota {nome[:40]:40s} {comp:6.1f} mm · peça impressa {peca[0]:5.1f} · componente {comp_[0]:5.1f} · boca {boca:5.1f}")
        if comp_[0] < 2.5 or boca < 10:
            sys.exit(f"Fio perto demais: {nome}")
    folha_vistas(tri, folgas, png)
    folha_furacao(tri, rotas, png)


try:
    import FreeCAD  # noqa: F401
    NO_FREECAD = True
except ImportError:
    NO_FREECAD = False

if NO_FREECAD:
    no_freecad()
elif __name__ == "__main__":
    desenhar("--png" in sys.argv)
