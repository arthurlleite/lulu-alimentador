# Referências técnicas e bibliográficas

[Voltar ao relatório](README.md)

Fontes on-line consultadas em **29 de outubro de 2026**. Fontes de fabricante descrevem recursos declarados; datasheets descrevem componentes; a desmontagem descreve uma unidade. Os artigos científicos fundamentam a discussão, sem comprovar desempenho do protótipo da equipe.

## Produto e plataforma embarcada

<a id="f1"></a>
**F1. PETLIBRO.** *Air Smart Feeder — PLAF108*. Página do fabricante, versão australiana. [Produto e especificações](https://au.petlibro.com/products/air-wifi-feeder).

<a id="f2"></a>
**F2. PETLIBRO.** *How the AIR WiFi Feeder Works (PLAF108)*. Central de instruções: instalação, programação, notificações, reset e obstrução. [Página oficial](https://petlibro.com/pages/how-the-air-wifi-feeder-works-plaf108).

<a id="f3"></a>
**F3. PINECONE DATA.** *Flashing ESPHome on a WiFi Pet Feeder*. 19 jun. 2026. Desmontagem e substituição do firmware de uma unidade PLAF108. [Relato técnico com fotografias e logs](https://www.pineconedata.com/2026-06-19-flashing-esphome-on-wifi-pet-feeder/).

<a id="f4"></a>
**F4. ESPRESSIF SYSTEMS.** *ESP32-C3-WROOM-02 / ESP32-C3-WROOM-02U Datasheet*. Versão 1.7 consultada. [Datasheet do módulo](https://www.espressif.com/sites/default/files/documentation/esp32-c3-wroom-02_datasheet_en.pdf).

<a id="f5"></a>
**F5. ESPRESSIF SYSTEMS.** *ESP32-C3 Series Datasheet*. Versão 2.4 consultada. [Datasheet do SoC](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf).

<a id="f6"></a>
**F6. ESPRESSIF SYSTEMS.** *Analog to Digital Converter (ADC) Oneshot Mode Driver — ESP32-C3*. ESP-IDF Programming Guide, v5.3. [Documentação oficial](https://docs.espressif.com/projects/esp-idf/en/v5.3/esp32c3/api-reference/peripherals/adc_oneshot.html).

<a id="f7"></a>
**F7. ESPRESSIF SYSTEMS.** *Security — ESP32-C3*. ESP-IDF Programming Guide, v5.3. [Secure Boot, Flash Encryption e identidade de dispositivo](https://docs.espressif.com/projects/esp-idf/en/v5.3/esp32c3/security/security.html).

<a id="f8"></a>
**F8. ESPRESSIF SYSTEMS.** *ESP32-S3 Series Datasheet*. Versão 2.2 consultada. [Datasheet do SoC](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf).

<a id="f9"></a>
**F9. ESPRESSIF SYSTEMS.** *FreeRTOS (IDF) — ESP32-S3*. ESP-IDF Programming Guide, v5.3. Implementação baseada em FreeRTOS 10.5.1 com modificações SMP. [Documentação oficial](https://docs.espressif.com/projects/esp-idf/en/v5.3/esp32s3/api-reference/system/freertos_idf.html).

<a id="f10"></a>
**F10. PETLIBRO.** *How to Resolve Connection Issues for the Air Smart Feeder (AF108, PLAF108)*. [Suporte do fabricante](https://designlibro.zendesk.com/hc/en-us/articles/43966875800857-How-to-Resolve-Connection-Issues-for-the-Air-Smart-Feeder-AF108-PLAF108).

<a id="f11"></a>
**F11. PETLIBRO.** *Air WiFi Automatic Pet Feeder PLAF108 — User Manual*. Manual disponibilizado na página oficial do produto. [Manual original em PDF](https://cdn.shopify.com/s/files/1/0252/5197/1119/files/PETLIBRO_AIR_WiFi_Feeder_PLAF108_USER_MANUAL.pdf?v=1697786658). Conferir a revisão indicada na unidade adquirida; não confundir PLAF108 com PLAF108S.

<a id="f12"></a>
**F12. ESPRESSIF SYSTEMS.** *ESP32-S3-DevKitC-1*. Guia da placa e variantes. [Documentação oficial](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/index.html). Confirmar marcação do módulo e variante física; não atribuir automaticamente PSRAM configurada ao build.

<a id="f13"></a>
**F13. FUNDAMENTOS DE SISTEMAS EMBARCADOS, FGA/UnB.** *Lista de Sensores e Atuadores Disponíveis*. Trabalho 2, 2026/2. [Lista oficial](https://gitlab.com/fse_fga/trabalhos-2026_2/trabalho-2-2026-2/-/blob/main/Lista_de_Sensores.md).

<a id="f14"></a>
**F14. WAVESHARE.** *Flame Sensor*. Módulo detector de chama com comparador LM393: faixa de 760 a 1100 nm, saídas digital e analógica e ajuste de sensibilidade. [Página do produto](https://www.waveshare.com/product/modules/sensors/flame-sensor.htm) e [wiki técnica](https://www.waveshare.com/wiki/Flame_Sensor).

<a id="f15"></a>
**F15. OASIS.** BANKS, Andrew; GUPTA, Rahul (ed.). *MQTT Version 3.1.1*. OASIS Standard, 29 out. 2014. [Especificação](https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html).

<a id="f16"></a>
**F16. HIVEMQ.** *Free MQTT Broker*. Limites do serviço público compartilhado. [Informações oficiais](https://www.hivemq.com/mqtt/public-mqtt-broker/).

<a id="f17"></a>
**F17. TEXAS INSTRUMENTS.** *LM393, LM293, LM2903 Dual Differential Comparators*. Datasheet do comparador usado nos módulos detectores de chama. [PDF](https://www.ti.com/lit/ds/symlink/lm393.pdf).

<a id="f18"></a>
**F18. TOWER PRO.** *MG996R — Metal Gear Servo*. Especificações do servo usado no dosador. [Página do fabricante](https://www.towerpro.com.tw/product/mg996r/).

## Artigos sobre tecnologias

<a id="a1"></a>
**A1 / T1.** WYTRĘBOWICZ, Jacek; CABAJ, Krzysztof; KRAWIEC, Jerzy. *Messaging Protocols for IoT Systems—A Pragmatic Comparison*. **Sensors**, v. 21, n. 20, art. 6904, 2021. DOI: [10.3390/s21206904](https://doi.org/10.3390/s21206904). [PDF](https://www.mdpi.com/1424-8220/21/20/6904/pdf?version=1634718880).

<a id="a2"></a>
**A2 / T2.** FAROOQ, Muhammad Omer; KUNZ, Thomas. *Operating Systems for Wireless Sensor Networks: A Survey*. **Sensors**, v. 11, n. 6, p. 5900–5930, 2011. DOI: [10.3390/s110605900](https://doi.org/10.3390/s110605900). [PDF](https://www.mdpi.com/1424-8220/11/6/5900/pdf?version=1403315286).

<a id="a3"></a>
**A3 / T3.** SILVA, Jonathan de Carvalho; RODRIGUES, Joel J. P. C.; AL-MUHTADI, Jalal; RABÊLO, Ricardo A. L.; FURTADO, Vasco. *Management Platforms and Protocols for Internet of Things: A Survey*. **Sensors**, v. 19, n. 3, art. 676, 2019. DOI: [10.3390/s19030676](https://doi.org/10.3390/s19030676). [PDF](https://www.mdpi.com/1424-8220/19/3/676/pdf).

<a id="a4"></a>
**A4 / T4.** MRABET, Hichem; BELGUITH, Sana; ALHOMOUD, Adeeb; JEMAI, Abderrazak. *A Survey of IoT Security Based on a Layered Architecture of Sensing and Data Analysis*. **Sensors**, v. 20, n. 13, art. 3625, 2020. DOI: [10.3390/s20133625](https://doi.org/10.3390/s20133625). [PDF](https://mdpi-res.com/d_attachment/sensors/sensors-20-03625/article_deploy/sensors-20-03625.pdf).

## Artigos sobre aplicação e uso

<a id="a5"></a>
**A5 / U1.** WITZEL-ROLLINS, Angela; MURPHY, Maryanne; SPRINGER, Cary M.; MOYERS, Tamberlyn D.; ALBRIGHT, Julia D. *Evaluation of a pet-separating automatic feeder and high-frequency meal feeding for weight loss in multi-cat households*. **Journal of Feline Medicine and Surgery**, v. 24, n. 8, p. e281–e288, 2022. DOI: [10.1177/1098612X221105046](https://doi.org/10.1177/1098612X221105046). [PDF](https://journals.sagepub.com/doi/pdf/10.1177/1098612X221105046).

<a id="a6"></a>
**A6 / U2.** MIGNY, Rémi; CONCORDET, Didier; REYNOLDS, Brice S. *Characterising individual feeding and elimination behaviours in healthy cats using a connected smart feeder and litter box: a preliminary study*. **Journal of Feline Medicine and Surgery**, v. 28, n. 2, art. 1098612X251414320, 2026. DOI: [10.1177/1098612X251414320](https://doi.org/10.1177/1098612X251414320). [PDF](https://journals.sagepub.com/doi/pdf/10.1177/1098612X251414320).

<a id="a7"></a>
**A7 / U3.** EYRE, Ryan; MARSHALL, Emily; GOYON, Annabelle; ELLERBY, Zack; CARVELL-MILLER, Laura; MCGRANE, Scott J. *Twenty-Four-Hour Feeding Patterns of In-Home Healthy Aging Cats Fed Wet, Dry, or a Combination of Wet and Dry Diets Ad Libitum*. **Animals**, v. 16, n. 1, art. 45, edição 2026; publicação on-line em 24 dez. 2025. DOI: [10.3390/ani16010045](https://doi.org/10.3390/ani16010045). [PDF](https://www.mdpi.com/2076-2615/16/1/45/pdf).

<a id="a8"></a>
**A8 / U4.** AMANS, Faida Bako; UMAR, Umar Ali. *Design and Fabrication of an Automatic Pet Food Dispenser*. **Journal of Mechatronics and Artificial Intelligence**, v. 2, n. 1, p. 11–20, 2025. DOI: [10.17509/jmai.v2i1.80389](https://doi.org/10.17509/jmai.v2i1.80389). [PDF](https://ejournal.upi.edu/index.php/JMAI/article/download/80389/31419).

## Artigos complementares sobre tecnologias

<a id="a9"></a>
**A9 / T5.** PLAUSKA, Ignas; LIUTKEVIČIUS, Agnius; JANAVIČIŪTĖ, Audronė. *Performance Evaluation of C/C++, MicroPython, Rust and TinyGo Programming Languages on ESP32 Microcontroller*. **Electronics**, v. 12, n. 1, art. 143, 2023. DOI: [10.3390/electronics12010143](https://doi.org/10.3390/electronics12010143). [PDF](https://www.mdpi.com/2079-9292/12/1/143/pdf).

<a id="a10"></a>
**A10 / T6.** AZHAR, M. Azhar; AZWAR, Muhamad; ASRONI, Ondi. *Performance and Security Analysis of MQTT over TLS on ESP32-Based IoT Systems*. **Journal of Applied Informatics and Computing**, v. 10, n. 4, p. 4144–4156, 2026. DOI: [10.30871/jaic.v10i4.13355](https://doi.org/10.30871/jaic.v10i4.13355). [PDF](https://jurnal.polibatam.ac.id/index.php/JAIC/article/download/13355/3921).

<a id="a11"></a>
**A11 / T7.** DEL-VALLE-SOTO, Carolina; ALVAREZ-GARCIA, Maria Fernanda; BRISEÑO, Ramon A.; RODRIGUEZ, Jafet; VISCONTI, Paolo. *Stability-Aware Security–Performance Trade-Off Analysis in Resource-Constrained IoT Systems*. **Digital**, v. 6, n. 2, art. 35, 2026. DOI: [10.3390/digital6020035](https://doi.org/10.3390/digital6020035). [PDF](https://www.mdpi.com/2673-6470/6/2/35/pdf).

## Artigos complementares sobre aplicação e uso

<a id="a12"></a>
**A12 / U5.** UNTIVEROS-PEÑALOZA, Helder; CASTRO-VARGAS, Cristian. *IoT-Based Smart Pet Feeding System with Dynamic Network Configuration Using ESP32, WiFiManager and Blynk 2.0*. **Ingénierie des Systèmes d'Information**, v. 30, n. 11, p. 2881–2889, 2025. DOI: [10.18280/isi.301106](https://doi.org/10.18280/isi.301106). [PDF](https://www.iieta.org/download/file/fid/190865).

<a id="a13"></a>
**A13 / U6.** KHUSNI, Mukhamad Iqbal; SUASANA, Iman Saufik; KURNIAWAN, Dendy. *Design of IoT-Based Fire Monitoring Using Flame and Smoke Sensors (Dlimas Village)*. **Jurnal Ilmiah Sistem Informasi**, v. 5, n. 2, p. 33–44, 2026. DOI: [10.51903/aznf0p81](https://doi.org/10.51903/aznf0p81). [PDF](https://ejurnal.provisi.ac.id/index.php/JUISI/article/download/1497/1031).

## Produtos do comparativo

<a id="c1"></a>
**C1. PETKIT.** *Fresh Element Solo Automatic Feeder — P570*. Especificações e perguntas frequentes. [Fabricante](https://www.petkit.com/products/fresh-element-solo).

<a id="c2"></a>
**C2. XIAOMI.** *Xiaomi Smart Pet Food Feeder — Specifications*. Família XWPF01MG. [Especificações oficiais](https://www.mi.com/global/product/xiaomi-smart-pet-food-feeder/specs/).

<a id="c3"></a>
**C3. XIAOMI.** *Xiaomi Smart Pet Food Feeder*. Construção, dosagem, alimentação de emergência e interface. [Produto](https://www.mi.com/global/product/xiaomi-smart-pet-food-feeder/).

<a id="c4"></a>
**C4. AQARA.** *Smart Pet Feeder C1*. Funções, capacidade, alimentação e dependência de hub. [Produto](https://www.aqara.com/en/product/smart-pet-feeder-c1/).

<a id="c5"></a>
**C5. AQARA.** *Smart Pet Feeder C1 — Specifications*. Modelo PETC1-M01. [Especificações](https://www.aqara.com/en/product/smart-pet-feeder-c1/specs/).

<a id="c6"></a>
**C6. PETSAFE.** *Smart Feed Automatic Dog and Cat Feeder, 2nd Generation — PFD00-16828*. [Produto](https://www.petsafe.com/p/smart-feed/PFD00-16828/).

<a id="c7"></a>
**C7. PETSAFE.** *Product Catalog — Pet Feeders*. Tabela do modelo PFD00-16828: programação, porções e energia. [Catálogo do fabricante em PDF](https://www.petsafe.net/media/downloads/415-403-19_Online.pdf).

<a id="c8"></a>
**C8. PETLIBRO.** *What are the differences between the PLAF101/102 Essential Wi-Fi Feeder, PLAF103 Granary Wi-Fi Feeder, and the PLAF203 Granary Camera Monitoring Feeder?* Atualização indicada: 16 ago. 2024. [Comparativo do suporte oficial](https://designlibro.zendesk.com/hc/en-us/articles/7123581366425-What-are-the-differences-between-the-PLAF101-102-Essential-Wi-Fi-Feeder-PLAF103-Granary-Wi-Fi-Feeder-and-the-PLAF203-Granary-Camera-Monitoring-Feeder).

## Imagens e autoria

<a id="i1"></a>
**I1. PETLIBRO.** Imagem promocional do Air Smart Feeder. [Arquivo original](https://au.petlibro.com/cdn/shop/files/AF108.png?v=1746612294&width=1200). Imagem incorporada pelo endereço da fonte: [petlibro-air-plaf108.png](https://au.petlibro.com/cdn/shop/files/AF108.png?v=1746612294&width=1200). Direitos da imagem pertencem ao titular original; a atribuição não constitui declaração de licença aberta.

<a id="i2"></a>
**I2. PINECONE DATA.** Fotografia da placa PLAF108, publicada no relato F3. [Arquivo original](https://www.pineconedata.com/assets/img/posts/2026-06-19-flashing-esphome-on-wifi-pet-feeder/IMG_20260605_163636_790.jpg). Imagem incorporada pelo endereço da fonte: [placa-plaf108.jpg](https://www.pineconedata.com/assets/img/posts/2026-06-19-flashing-esphome-on-wifi-pet-feeder/IMG_20260605_163636_790.jpg). Crédito e direitos do autor original; não é fotografia produzida pela equipe.

Os diagramas conceituais desta pasta foram elaborados para o relatório com base nas fontes indicadas. As renderizações e peças do Lulu permanecem em [hardware/3d](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/hardware/3d/README.md), com seu histórico de desenvolvimento preservado no repositório.

