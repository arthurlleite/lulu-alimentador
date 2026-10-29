"""Posição da eletrônica dentro do tubo: desenho em 3 vistas e peças no FreeCAD.

Uma única lista (COMPONENTES) guarda medida e posição de cada peça, nas mesmas
coordenadas do IoT3D.FCStd: eixo Z no centro do tubo, base do tubo em z = -177,
saída da ração (pote) para -Y. Medidas em mm.

Desenho (na raiz do repositório; --png precisa do Chrome/Chromium e do Pillow):
    python ferramentas/eletronica_3d.py --png
    -> hardware/3d/imagens/eletronica_posicionamento.svg/.png

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
CAP_MM = 45          # altura coberta pelo cap PVC na base: nenhum furo abaixo disso

# (nº, ref, nome, forma, medidas, posição, cor)
#   caixa:    medidas = (dx, dy, dz), posição = canto mínimo (x, y, z)
#   cilindro: medidas = (raio, altura, eixo "+y" ou "-x"), posição = centro da base
# A placa perfurada fica em pé, paralela à parede +X, aparafusada na aba lisa do
# suporte (x = 29,1); os componentes ficam na face voltada para o centro (-X).
PLACA_X = 27.5  # face dos componentes
COMPONENTES = [
    (1, "—", "Placa perfurada 7×9 cm (em pé, no suporte)", "caixa", (1.6, 70, 90), (PLACA_X, -35, -175), "#c8a165"),
    (2, "U1", "ESP32-S3-DevKitC-1 (USB-C para baixo)", "caixa", (1.6, 25.4, 69), (17.4, 5, -163), "#3a3a3a"),
    (3, "U1", "Módulo ESP32-S3-WROOM-1 (antena para cima)", "caixa", (3.1, 18, 25.5), (14.3, 8.7, -119.5), "#b8b8b8"),
    (4, "U1", "Conectores USB-C da placa", "caixa", (3.5, 20, 7.5), (13.9, 7.7, -164), "#8c8c8c"),
    (5, "—", "Barra fêmea 1×22 (ESP, lado 1)", "caixa", (8.5, 2.54, 56), (19, 5, -156.5), "#1e1e1e"),
    (5, "—", "Barra fêmea 1×22 (ESP, lado 2)", "caixa", (8.5, 2.54, 56), (19, 27.86, -156.5), "#1e1e1e"),
    (6, "R1–R3", "Resistores 220 Ω do LED", "caixa", (3, 10, 10), (24.5, -10, -125), "#d9b26f"),
    (7, "C1", "Capacitor 1000 µF / 16 V (Ø10 × 20)", "cilindro", (5, 20, "-x"), (PLACA_X, -20, -110), "#2c56a8"),
    (8, "—", "Barra macho 1×3 do servo", "caixa", (11, 7.62, 2.54), (16.5, -24, -100), "#1e1e1e"),
    (9, "—", "Barra macho 1×12 (sensores, LED, botão, buzzer)", "caixa", (11, 2.54, 30.5), (16.5, -32, -160), "#1e1e1e"),
    (10, "J1", "P4 fêmea com borne (entrada da fonte)", "caixa", (12, 28, 11), (-35, 3, -128), "#4a4a4a"),
    (11, "U2", "Sensor de chama da saída (na extensão do funil)", "caixa", (32, 14, 7), (19, -68, -113), "#2f6fd0"),
    (12, "U3", "Sensor de chama do alarme (parede esquerda)", "caixa", (7, 14, 32), (-47, -7, -100), "#2f6fd0"),
    (13, "D1", "LED RGB 5 mm (atravessa a parede)", "cilindro", (2.5, 9, "+y"), (0, 40.5, -95), "#e8e8ff"),
    (14, "SW1", "Chave táctil 12×12 (colada por fora)", "caixa", (12, 4.3, 12), (-6, R_EXT, -116), "#5a5a5a"),
    (15, "BZ1", "Buzzer ativo Ø12 × 9,5", "cilindro", (6, 9.5, "+y"), (0, 38.5, -125), "#262626"),
]

# (furo, Ø mm, onde, centro (x, y, z), para quê)
FUROS = [
    ("F1", "8", "parede, do lado do servo", (44.4, -20, -20), "fios do servo saem (acima da borda do funil)"),
    ("F2", "8", "parede direita, logo após a borda da placa", (30.4, -38, -95), "fios do servo voltam até a placa"),
    ("F3", "12", "parede de trás, à esquerda", (-29, 39.2, -122.5), "plugue da fonte até o J1"),
    ("F4", "5,5", "parede de trás, centro", (0, R_INT, -95), "LED RGB"),
    ("F5", "2 × 2", "parede de trás, centro", (0, R_INT, -110), "fios da chave táctil"),
    ("F6", "3", "parede de trás, centro", (0, R_INT, -125), "som do buzzer"),
    ("F7", "5,5", "parede esquerda", (-R_INT, 0, -71), "fotodiodo do U3 olhando para fora"),
    ("F8", "5", "extensão do funil, lado +X", (18, -61, -110), "fotodiodo do U2"),
    ("F9", "5", "extensão do funil, lado −X", (-18, -61, -110), "luz do outro lado do U2"),
]


def caixa_de(c):
    """Caixa envolvente (xmin, xmax, ymin, ymax, zmin, zmax) de um componente."""
    _, _, _, forma, m, p, _ = c
    if forma == "caixa":
        return p[0], p[0] + m[0], p[1], p[1] + m[1], p[2], p[2] + m[2]
    r, h, eixo = m
    if eixo == "-x":
        return p[0] - h, p[0], p[1] - r, p[1] + r, p[2] - r, p[2] + r
    return p[0] - r, p[0] + r, p[1], p[1] + h, p[2] - r, p[2] + r


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
        num, ref, nome, forma, m, p, cor = c
        rotulo = f"{num:02d}_{ref}_{nome}".replace("—", "").replace(" ", "_")
        if forma == "caixa":
            obj = doc.addObject("Part::Box", "Eletr")
            obj.Length, obj.Width, obj.Height = m
            obj.Placement = FreeCAD.Placement(FreeCAD.Vector(*p), FreeCAD.Rotation())
        else:
            obj = doc.addObject("Part::Cylinder", "Eletr")
            obj.Radius, obj.Height = m[0], m[1]
            # gira o eixo do cilindro de +Z para +Y ou para -X
            giro = FreeCAD.Vector(1, 0, 0) if m[2] == "+y" else FreeCAD.Vector(0, 1, 0)
            obj.Placement = FreeCAD.Placement(FreeCAD.Vector(*p), FreeCAD.Rotation(giro, -90))
        obj.Label = rotulo
        grupo.addObject(obj)
        if FreeCAD.GuiUp:
            rgb = tuple(int(cor[i:i + 2], 16) / 255 for i in (1, 3, 5))
            obj.ViewObject.ShapeColor = rgb
    doc.recompute()
    FreeCAD.Console.PrintMessage(f"Eletronica: {len(COMPONENTES)} peças adicionadas. Confira e salve.\n")


# ======================= desenho em SVG =======================
def triangulos(caminho):
    d = open(caminho, "rb").read()
    n = struct.unpack("<I", d[80:84])[0]
    pts = []
    for i in range(n):
        f = struct.unpack("<12f", d[84 + i * 50:84 + i * 50 + 48])
        pts += [f[3:6], f[6:9], f[9:12]]
    return pts


def envoltoria(pts):
    """Envoltória convexa 2D (cadeia monotônica)."""
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


def desenhar(png=False):
    raiz = os.getcwd()
    stl = os.path.join(raiz, "hardware", "3d", "stl")
    pecas = {}
    for nome in ("funil_interno", "extensao_funil", "rampa_inclinada", "suporte_motor",
                 "tampa_atuador", "suporte_placa", "suporte_rampa", "tampa_tubo"):
        pecas[nome] = [(round(x, 1), round(y, 1), round(z, 1)) for x, y, z in triangulos(os.path.join(stl, nome + ".stl"))]

    L, A = 1600, 1131
    s = [f'<rect width="{L}" height="{A}" fill="#fff"/>']

    def texto(x, y, t, tam=12, cor="#000", ancora="start", peso="normal"):
        s.append(f'<text x="{x:.1f}" y="{y:.1f}" font-size="{tam}" fill="{cor}" text-anchor="{ancora}" '
                 f'font-weight="{peso}">{t}</text>')

    def poli(pts, fundo, borda, w=1, op=1.0, extra=""):
        p = " ".join(f"{a:.1f},{b:.1f}" for a, b in pts)
        s.append(f'<polygon points="{p}" fill="{fundo}" fill-opacity="{op}" stroke="{borda}" stroke-width="{w}"{extra}/>')

    # Vistas: (título, x0 da tela, y0 da tela, escala, função 3D -> 2D, filtro de z das peças)
    E = 2.3
    # (título, centro x, topo, escala, projeção, distância ao observador, parede vista)
    vistas = [
        ("A · De trás (olhando de +Y para −Y)", 190, 150, E, lambda x, y, z: (-x, -z), lambda c: -c[3], "y"),
        ("B · Lado esquerdo (olhando de −X para +X)", 520, 150, E, lambda x, y, z: (-y, -z), lambda c: c[0], "x"),
    ]
    ZTOPO = 140
    for titulo, cx, y0, e, proj, dist, parede in vistas:
        def tela(x, y, z, proj=proj, cx=cx, y0=y0, e=e):
            u, v = proj(x, y, z)
            return cx + u * e, y0 + (v + ZTOPO) * e
        texto(cx, y0 - 50, titulo, 15, "#000", "middle", "bold")
        # tubo (paredes) e cap
        for lado in (-1, 1):
            px, py = (lado * R_EXT, 0) if parede == "y" else (0, lado * R_EXT)
            a = tela(px, py, TOPO_Z)
            b = tela(px, py, BASE_Z)
            s.append(f'<line x1="{a[0]:.1f}" y1="{a[1]:.1f}" x2="{b[0]:.1f}" y2="{b[1]:.1f}" stroke="#7a9bb5" stroke-width="2"/>')
        x1, yc = cx - (R_EXT + 3) * e, y0 + (-BASE_Z - CAP_MM + ZTOPO) * e
        s.append(f'<rect x="{x1:.1f}" y="{yc:.1f}" width="{2 * (R_EXT + 3) * e:.1f}" height="{(CAP_MM + 4) * e:.1f}" '
                 f'fill="#dfe8ef" fill-opacity="0.55" stroke="#7a9bb5" stroke-dasharray="5 3"/>')
        texto(cx + (R_EXT + 6) * e, yc + 14, "cap PVC", 10, "#557", "start")
        # peças impressas (contorno)
        for nome, pts in pecas.items():
            h = envoltoria([tela(*p) for p in pts[::2]])
            poli(h, "#f2d7c4" if "funil" in nome else "#e6e6e6", "#b08a70" if "funil" in nome else "#999", 1, 0.55)
        # régua de altura a partir da base do tubo
        rx = cx - (R_EXT + 22) * e
        for h in range(0, 301, 50):
            _, yy = tela(0, 0, BASE_Z + h)
            s.append(f'<line x1="{rx:.1f}" y1="{yy:.1f}" x2="{rx + 8:.1f}" y2="{yy:.1f}" stroke="#666"/>')
            texto(rx - 4, yy + 4, str(h), 10, "#666", "end")
        _, ya = tela(0, 0, BASE_Z)
        _, yb = tela(0, 0, BASE_Z + 300)
        s.append(f'<line x1="{rx + 4:.1f}" y1="{ya:.1f}" x2="{rx + 4:.1f}" y2="{yb:.1f}" stroke="#666"/>')
        texto(rx - 4, yb - 12, "mm", 10, "#666", "end")
        # componentes, do fundo para a frente
        for c in sorted(COMPONENTES, key=lambda c: -dist(caixa_de(c))):
            x0_, x1_, y0_, y1_, z0_, z1_ = caixa_de(c)
            cantos = [tela(x, y, z) for x in (x0_, x1_) for y in (y0_, y1_) for z in (z0_, z1_)]
            us = [p[0] for p in cantos]
            vs = [p[1] for p in cantos]
            poli([(min(us), min(vs)), (max(us), min(vs)), (max(us), max(vs)), (min(us), max(vs))], c[6], "#000", 0.8, 0.9)
        # furos na parede visível
        for f, d, onde, (hx, hy, hz), _ in FUROS:
            if (parede == "y" and hy > 20) or (parede == "x" and hx < -20):
                px, py = tela(hx, hy, hz)
                s.append(f'<circle cx="{px:.1f}" cy="{py:.1f}" r="4" fill="none" stroke="#c00000" stroke-width="1.6"/>')
                texto(px + 7, py - 5, f, 10, "#c00000")
        # números dos componentes
        vistos = set()
        for c in COMPONENTES:
            if c[0] in vistos:
                continue
            vistos.add(c[0])
            x0_, x1_, y0_, y1_, z0_, z1_ = caixa_de(c)
            u, v = tela((x0_ + x1_) / 2, (y0_ + y1_) / 2, (z0_ + z1_) / 2)
            s.append(f'<circle cx="{u:.1f}" cy="{v:.1f}" r="7.5" fill="#fff" stroke="#000" stroke-width="0.9"/>')
            texto(u, v + 3.6, str(c[0]), 9.5, "#000", "middle", "bold")
        # setas de orientação
        if parede == "x":
            texto(cx + (R_EXT + 2) * e, y0 - 25, "pote (−Y) →", 11, "#557", "end")

    # Vista C: corte de cima, abaixo do funil
    cx, cy, e = 1000, 520, 3.0
    texto(cx, 100, "C · Corte de cima, abaixo do funil (z &lt; −85)", 15, "#000", "middle", "bold")
    s.append(f'<circle cx="{cx}" cy="{cy}" r="{R_EXT * e}" fill="#f4f8fb" stroke="#7a9bb5" stroke-width="2"/>')
    s.append(f'<circle cx="{cx}" cy="{cy}" r="{R_INT * e}" fill="#fff" stroke="#7a9bb5" stroke-width="1.2"/>')

    def tela_c(x, y):
        return cx + x * e, cy - y * e

    for nome in ("extensao_funil", "suporte_placa"):
        pts = [tela_c(x, y) for x, y, z in pecas[nome] if z < -85]
        poli(envoltoria(pts), "#f2d7c4" if "funil" in nome else "#9cc79f", "#777", 1, 0.7)
    for c in sorted(COMPONENTES, key=lambda c: caixa_de(c)[5]):
        x0_, x1_, y0_, y1_, _, _ = caixa_de(c)
        a, b = tela_c(x0_, y1_), tela_c(x1_, y0_)
        if c[3] == "cilindro":
            r = c[4][0] * e
            s.append(f'<rect x="{a[0]:.1f}" y="{a[1]:.1f}" width="{b[0] - a[0]:.1f}" height="{b[1] - a[1]:.1f}" '
                     f'rx="{min(r, 4):.1f}" fill="{c[6]}" fill-opacity="0.9" stroke="#000" stroke-width="0.8"/>')
        else:
            poli([a, (b[0], a[1]), b, (a[0], b[1])], c[6], "#000", 0.8, 0.9)
    vistos = set()
    for c in COMPONENTES:
        if c[0] in vistos or c[0] in (5,):
            continue
        vistos.add(c[0])
        x0_, x1_, y0_, y1_, _, _ = caixa_de(c)
        u, v = tela_c((x0_ + x1_) / 2, (y0_ + y1_) / 2)
        s.append(f'<circle cx="{u:.1f}" cy="{v:.1f}" r="7.5" fill="#fff" stroke="#000" stroke-width="0.9"/>')
        texto(u, v + 3.6, str(c[0]), 9.5, "#000", "middle", "bold")
    texto(cx, cy - R_EXT * e - 42, "trás (+Y): LED, chave, buzzer, fonte", 11.5, "#557", "middle")
    texto(cx, cy + R_EXT * e + 112, "frente (−Y): saída da ração para o pote", 11.5, "#557", "middle")
    texto(cx + R_EXT * e + 8, cy + 4, "+X", 11.5, "#557", "start")
    texto(cx - R_EXT * e - 8, cy + 4, "−X", 11.5, "#557", "end")
    texto(cx, cy + R_EXT * e + 150, "Verde: suporte da placa (impresso). A placa encosta na aba lisa e leva 2 parafusos M3;", 11, "#444", "middle")
    texto(cx, cy + R_EXT * e + 166, "as porcas ficam dentro do U. Não solde nada atrás da placa na área do suporte.", 11, "#444", "middle")

    # Tabela de componentes
    tx, ty = 1180, 110
    texto(tx, ty, "Componentes (X × Y × Z, em mm)", 14, "#000", "start", "bold")
    vistos = set()
    linha = 0
    for c in COMPONENTES:
        if c[0] in vistos:
            continue
        vistos.add(c[0])
        num, ref, nome, forma, m, p, cor = c
        med = "×".join(f"{v:g}" for v in m) if forma == "caixa" else f"Ø{2 * m[0]:g}×{m[1]:g}"
        x0_, x1_, y0_, y1_, z0_, z1_ = caixa_de(c)
        yy = ty + 24 + linha * 34
        s.append(f'<rect x="{tx}" y="{yy - 11}" width="12" height="12" fill="{cor}" stroke="#000" stroke-width="0.6"/>')
        texto(tx + 18, yy, f"{num}. {'' if ref == '—' else ref + ' · '}{nome}", 11.5)
        texto(tx + 18, yy + 14, f"{med} · altura {z0_ - BASE_Z:.0f} a {z1_ - BASE_Z:.0f} mm da base", 10.5, "#555")
        linha += 1

    # Tabela de furos
    fy = 700
    texto(80, fy + 260, "", 1)
    s.append(f'<rect x="60" y="{fy + 255}" width="1080" height="150" fill="none" stroke="#000"/>')
    texto(75, fy + 278, "Furos no tubo (altura do centro medida a partir da base do tubo; nenhum abaixo de 45 mm, onde entra o cap)", 13, "#000", "start", "bold")
    for i, (f, d, onde, (_, _, z), para) in enumerate(FUROS):
        col, lin = divmod(i, 5)
        xx, yy = 75 + col * 540, fy + 302 + lin * 20
        texto(xx, yy, f"<tspan font-weight='bold' fill='#c00000'>{f}</tspan>  Ø {d} mm · {onde} · {z - BASE_Z:.0f} mm · {para}", 11)

    # carimbo simples
    texto(1180, 1060, "Lulu · posicionamento da eletrônica", 14, "#1d3bbd", "start", "bold")
    texto(1180, 1080, "Gerado por ferramentas/eletronica_3d.py a partir do IoT3D.FCStd", 10.5, "#555")
    texto(1180, 1096, "Medidas de catálogo: confira com paquímetro antes de furar.", 10.5, "#555")

    svg = (f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {L} {A}" width="{L}" height="{A}" '
           f'font-family="Arial, Helvetica, sans-serif">\n' + "\n".join(s) + "\n</svg>\n")
    saida = os.path.join("hardware", "3d", "imagens", "eletronica_posicionamento.svg")
    open(saida, "w", encoding="utf-8", newline="\n").write(svg)
    print(saida, len(svg))
    if png:
        chrome = next(filter(None, map(shutil.which, ("google-chrome", "chromium", "chromium-browser"))), None)
        if not chrome:
            sys.exit("Chrome/Chromium não encontrado: abra o SVG no navegador e exporte o PNG.")
        arq = os.path.abspath(saida[:-4] + ".png")
        subprocess.run([chrome, "--headless=new", "--disable-gpu", "--hide-scrollbars",
                        "--force-device-scale-factor=1.5", f"--window-size={L},{A + 200}",
                        f"--screenshot={arq}", "file://" + os.path.abspath(saida)],
                       check=True, capture_output=True)
        from PIL import Image
        Image.open(arq).crop((0, 0, int(L * 1.5), int(A * 1.5))).save(arq, optimize=True)
        print(arq)


try:
    import FreeCAD  # noqa: F401
    NO_FREECAD = True
except ImportError:
    NO_FREECAD = False

if NO_FREECAD:
    no_freecad()
elif __name__ == "__main__":
    desenhar("--png" in sys.argv)
