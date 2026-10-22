"""Exporta as peças imprimíveis do IoT3D.FCStd para hardware/3d/stl.

Uso (na raiz do repositório):
    freecadcmd ferramentas/exportar_3d.py
"""
import os

import FreeCAD
import MeshPart

RAIZ = os.getcwd()
MODELO = os.path.join(RAIZ, "hardware", "3d", "IoT3D.FCStd")
SAIDA = os.path.join(RAIZ, "hardware", "3d", "stl")

# objeto no FreeCAD (nome interno ou rótulo) -> nome do arquivo
PECAS = {
    "Body003": "rampa_inclinada",
    "Body004": "tampa_atuador",
    "Body005": "tampa_tubo",
    "Funil_interno": "funil_interno",
    "extensao_funil": "extensao_funil",
    "Body008": "suporte_rampa",
    "Body009": "suporte_placa",
    "Body011": "suporte_motor",
}

doc = FreeCAD.openDocument(MODELO)
os.makedirs(SAIDA, exist_ok=True)
for nome, arquivo in PECAS.items():
    obj = doc.getObject(nome) or doc.getObjectsByLabel(nome)[0]
    malha = MeshPart.meshFromShape(Shape=obj.Shape.copy(), LinearDeflection=0.05,
                                   AngularDeflection=0.2, Relative=False)
    malha.write(os.path.join(SAIDA, arquivo + ".stl"))
    print("ok:", arquivo)
