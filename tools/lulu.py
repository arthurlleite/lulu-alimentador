"""Gera a arte do Lulu da Pomerânia usada no app (ícone e ilustração).

Uso:
    python tools/lulu.py web/icon.svg icon   # com fundo arredondado (ícone)
    python tools/lulu.py lulu.svg            # sem fundo (vai embutido no HTML como <symbol id="lulu">)

Os PNGs (icon-192/512) são capturas do icon.svg em 192 e 512 px.
"""
import math, sys
out = sys.argv[1]
icon = len(sys.argv) > 2 and sys.argv[2] == "icon"

def fluff(cx, cy, rx, ry, spike, n, phase=0.0, sweep=1):
    """Silhueta com tufos de pelo pontudos e levemente curvados."""
    step = 2*math.pi/n
    def pt(a, k):
        return (cx + (rx+k)*math.cos(a), cy + (ry+k)*math.sin(a))
    a0 = phase
    x, y = pt(a0, 0)
    d = [f"M{x:.1f} {y:.1f}"]
    for i in range(n):
        a = a0 + i*step
        tip = a + step*0.5 + sweep*step*0.15
        c1 = pt(a + step*0.25, spike*0.85)
        t = pt(tip, spike)
        c2 = pt(a + step*0.8, spike*0.35)
        e = pt(a + step, 0)
        d.append(f"Q{c1[0]:.1f} {c1[1]:.1f} {t[0]:.1f} {t[1]:.1f}Q{c2[0]:.1f} {c2[1]:.1f} {e[0]:.1f} {e[1]:.1f}")
    return "".join(d) + "Z"

O, OD, OL, CR, INK = "#F59E45", "#D9701C", "#FFC987", "#FFF0DA", "#2B1B12"
bg = '<rect width="256" height="256" rx="58" fill="url(#bg)"/>' if icon else ""
svg = f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 256 256" role="img" aria-label="Lulu da Pomerânia">
<defs>
<radialGradient id="bg" cx="50%" cy="38%" r="70%"><stop offset="0" stop-color="#FFF6EA"/><stop offset="1" stop-color="#FFD9AE"/></radialGradient>
<radialGradient id="fur" gradientUnits="userSpaceOnUse" cx="112" cy="96" r="150"><stop offset="0" stop-color="{OL}"/><stop offset=".55" stop-color="{O}"/><stop offset="1" stop-color="{OD}"/></radialGradient>
<radialGradient id="tail" gradientUnits="userSpaceOnUse" cx="176" cy="100" r="60"><stop offset="0" stop-color="#FFE2B8"/><stop offset="1" stop-color="{O}"/></radialGradient>
</defs>
{bg}
<ellipse cx="128" cy="228" rx="66" ry="9" fill="#C98A52" opacity=".22"/>
<path d="{fluff(182,120,30,32,12,11,0.3)}" fill="url(#tail)"/>
<path d="{fluff(130,178,50,42,11,16,0.1)}" fill="url(#fur)"/>
<g fill="{CR}"><rect x="98" y="196" width="22" height="31" rx="11"/><rect x="134" y="196" width="22" height="31" rx="11"/></g>
<g fill="#F3CFA6"><path d="M104 221v5M109 221v5M114 221v5M140 221v5M145 221v5M150 221v5" stroke="#E9B985" stroke-width="2" stroke-linecap="round"/></g>
<path d="M74 86 L82 30 Q86 22 94 30 L118 66 Z" fill="{OD}"/>
<path d="M166 86 L158 30 Q154 22 146 30 L122 66 Z" fill="{OD}"/>
<path d="M84 74 L87 42 L104 66 Z" fill="#F6B3A0"/>
<path d="M156 74 L153 42 L136 66 Z" fill="#F6B3A0"/>
<path d="{fluff(120,116,52,50,13,20,0.05)}" fill="url(#fur)"/>
<path d="{fluff(122,160,20,15,8,9,0.4,-1)}" fill="{CR}"/>
<ellipse cx="120" cy="124" rx="40" ry="34" fill="#FFCF94"/>
<ellipse cx="120" cy="140" rx="22" ry="16" fill="{CR}"/>
<g fill="{INK}"><ellipse cx="101" cy="118" rx="8.5" ry="9.5"/><ellipse cx="139" cy="118" rx="8.5" ry="9.5"/></g>
<g fill="#fff"><circle cx="104" cy="114" r="3.2"/><circle cx="142" cy="114" r="3.2"/><circle cx="98.5" cy="122" r="1.5"/><circle cx="136.5" cy="122" r="1.5"/></g>
<path d="M90 106 Q100 100 110 105 M130 105 Q140 100 150 106" stroke="#E59447" stroke-width="2.5" fill="none" stroke-linecap="round"/>
<g fill="#F38C7C" opacity=".5"><ellipse cx="88" cy="137" rx="8" ry="5"/><ellipse cx="152" cy="137" rx="8" ry="5"/></g>
<path d="M112 131 Q120 126 128 131 Q128 138 120 140 Q112 138 112 131 Z" fill="{INK}"/>
<ellipse cx="117" cy="131.5" rx="2.4" ry="1.3" fill="#fff" opacity=".75"/>
<path d="M120 140v4.5M120 144.5q-6 5-11 1M120 144.5q6 5 11 1" stroke="{INK}" stroke-width="2.4" fill="none" stroke-linecap="round"/>
<path d="M114.5 147.5Q120 161 125.5 147.5Z" fill="#EF6F6F"/>
<path d="M120 149v6" stroke="#D85858" stroke-width="1.2" stroke-linecap="round"/>
</svg>
'''
open(out, "w", encoding="utf-8").write(svg)
print(len(svg))
