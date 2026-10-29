# 3.4 — Pesquisa bibliográfica: oito artigos de periódicos

[Voltar ao relatório](README.md) · [Referências completas](referencias.md)

A seleção contém **quatro artigos sobre tecnologias** e **quatro sobre aplicação/uso de alimentadores**. São publicações de periódicos, não anais de conferências, trabalhos de conclusão, tutoriais ou manuais. A busca utilizou páginas dos editores e registros PubMed/PMC, com termos relacionados a protocolos IoT, sistemas operacionais embarcados, gerenciamento de dispositivos e alimentadores automáticos. Consulta: **29/10/2026**. A seleção é dirigida ao projeto, sem pretensão de revisão sistemática exaustiva.

Cada ficha apresenta objetivo/método, contribuição e uma aplicação **proposta pela equipe**. Essa aplicação não deve ser atribuída aos autores. Os títulos completos e dados bibliográficos estão nas referências. **Nenhum PDF deve ser adicionado ao repositório.** Alguns servidores impõem verificações de navegador ao acesso automático; mantêm-se o DOI e o texto HTML como caminhos alternativos.

## A. Quatro artigos sobre tecnologias

<a id="t1"></a>

### T1 — Protocolos de mensagens para IoT

**Wytrębowicz, Cabaj e Krawiec (2021), Sensors, 21(20), 6904.** [Referência A1](referencias.md#a1) · [DOI](https://doi.org/10.3390/s21206904) · [PDF original](https://www.mdpi.com/1424-8220/21/20/6904/pdf?version=1634718880) · [HTML](https://pmc.ncbi.nlm.nih.gov/articles/PMC8540579/)

**Resumo:** compara protocolos de mensagens a partir de suas especificações e dos requisitos de diferentes dispositivos IoT. Analisa funcionalidades, complexidade e adequação a aquisição de dados, notificações e envio de comandos. A contribuição é um método de escolha orientado ao cenário, em vez de um protocolo universalmente superior.

**Relação com o Lulu:** fundamenta separar a API HTTP local da comunicação MQTT remota e definir tópicos de estado, evento e comando. **Limite:** o estudo não demonstra segurança ou desempenho do nosso firmware, nem identifica o protocolo original do PETLIBRO.

<a id="t2"></a>

### T2 — Sistemas operacionais para nós sensores

**Farooq e Kunz (2011), Sensors, 11(6), 5900–5930.** [Referência A2](referencias.md#a2) · [DOI](https://doi.org/10.3390/s110605900) · [PDF original](https://www.mdpi.com/1424-8220/11/6/5900/pdf?version=1403315286) · [HTML](https://pmc.ncbi.nlm.nih.gov/articles/PMC3231431/)

**Resumo:** revisa sistemas operacionais para redes de sensores com recursos limitados. Compara arquitetura, programação, escalonamento, memória, comunicação e compartilhamento de recursos. Mostra que requisitos temporais e concorrência precisam orientar a organização do software, juntamente com suas restrições de memória e energia.

**Relação com o Lulu:** fornece critérios para discutir tarefas, prioridades, fila do dosador e proteção do estado. **Limite:** é uma referência conceitual anterior ao ESP32; não mede FreeRTOS no ESP32-S3. A implementação adotada é documentada separadamente pela [Espressif, F9](referencias.md#f9).

<a id="t3"></a>

### T3 — Gerenciamento de dispositivos e plataformas IoT

**Silva et al. (2019), Sensors, 19(3), 676.** [Referência A3](referencias.md#a3) · [DOI](https://doi.org/10.3390/s19030676) · [PDF original](https://www.mdpi.com/1424-8220/19/3/676/pdf) · [HTML](https://pmc.ncbi.nlm.nih.gov/articles/PMC6387169/)

**Resumo:** revisa plataformas e protocolos para gerenciamento de redes e dispositivos IoT. Discute heterogeneidade, interoperabilidade, configuração, monitoramento e segurança, comparando soluções e requisitos de implantação. Sua contribuição é organizar critérios de avaliação para integrar dispositivos e serviços com limitações diferentes.

**Relação com o Lulu:** fundamenta o fluxo de configuração inicial, a telemetria de estado e a distinção entre interface, broker e dispositivo. **Limite:** não identifica a plataforma interna da PETLIBRO nem demonstra interoperabilidade do nosso firmware com o serviço comercial.

<a id="t4"></a>

### T4 — Segurança em camadas na IoT

**Mrabet, Belguith, Alhomoud e Jemai (2020), Sensors, 20(13), 3625.** [Referência A4](referencias.md#a4) · [DOI](https://doi.org/10.3390/s20133625) · [PDF original](https://mdpi-res.com/d_attachment/sensors/sensors-20-03625/article_deploy/sensors-20-03625.pdf)

**Resumo:** organiza ameaças e medidas de proteção em camadas de percepção, rede/protocolo, transporte, aplicação e dados/nuvem. A revisão mostra que proteger apenas um componente deixa outras superfícies expostas e discute autenticação, autorização, integridade e confidencialidade.

**Relação com o Lulu:** orienta separar TLS, identidade do cliente, permissão nos tópicos e validação dos comandos. **Limite:** o artigo não audita o Lulu. Usar TLS com um broker público não comprova autorização; um identificador no tópico não substitui controle de acesso.

## B. Quatro artigos sobre aplicação e uso do produto

<a id="u1"></a>

### U1 — Alimentador com acesso individual em casas com vários gatos

**Witzel-Rollins et al. (2022), Journal of Feline Medicine and Surgery, 24(8), e281–e288.** [Referência A5](referencias.md#a5) · [DOI](https://doi.org/10.1177/1098612X221105046) · [PDF original](https://journals.sagepub.com/doi/pdf/10.1177/1098612X221105046) · [HTML](https://pmc.ncbi.nlm.nih.gov/articles/PMC10812286/)

**Resumo:** compara, em 23 gatos, programas de redução de peso com pote convencional ou alimentador de acesso individual e duas ou seis refeições diárias. O alimentador facilitou o programa segundo os tutores. Aumentar de duas para seis refeições não acelerou a taxa de perda de peso.

**Relação com o Lulu:** justifica horários configuráveis e registros, mas evidencia a limitação de não identificar qual animal acessa o pote. **Limite:** o Lulu não reproduz o controle individual do equipamento estudado; o estudo não determina uma programação alimentar universal.

<a id="u2"></a>

### U2 — Monitoramento com alimentador conectado

**Migny, Concordet e Reynolds (2026), Journal of Feline Medicine and Surgery, 28(2), 1098612X251414320.** [Referência A6](referencias.md#a6) · [DOI](https://doi.org/10.1177/1098612X251414320) · [PDF original](https://journals.sagepub.com/doi/pdf/10.1177/1098612X251414320) · [HTML](https://pmc.ncbi.nlm.nih.gov/articles/PMC12905059/)

**Resumo:** estudo preliminar que usa alimentador Catspad e caixa sanitária conectada para registrar comportamentos e modelar padrões temporais. Os dados são processados com análise de frequências para descrever rotinas individuais de animais saudáveis.

**Relação com o Lulu:** motiva registrar data, hora e resultado de cada dispensação para análise posterior. **Limite:** o histórico atual do Lulu registra comandos/dispensação e permanece em RAM; não mede ingestão nem identifica o animal. Não permite reproduzir as conclusões de monitoramento do artigo.

<a id="u3"></a>

### U3 — Padrões de alimentação medidos por alimentadores

**Eyre et al., Animals, 16(1), 45 (edição de 2026; publicação on-line em 24/12/2025).** [Referência A7](referencias.md#a7) · [DOI](https://doi.org/10.3390/ani16010045) · [PDF original](https://www.mdpi.com/2076-2615/16/1/45/pdf) · [HTML](https://pmc.ncbi.nlm.nih.gov/articles/PMC12784832/)

**Resumo:** acompanha 134 gatos com sete anos ou mais em domicílio, usando alimentadores com pesagem. Compara acesso livre a dietas seca, úmida e mista e observa diferenças na frequência das refeições e ingestão energética conforme a apresentação da dieta.

**Relação com o Lulu:** reforça a utilidade de horários e quantidades configuráveis, além de separar ração liberada de ração consumida. **Limite:** o Lulu não possui pesagem, não trabalha com alimento úmido e não reproduz o protocolo experimental; os resultados não validam sua precisão.

<a id="u4"></a>

### U4 — Construção e avaliação de um dispensador automático

**Amans e Umar (2025), Journal of Mechatronics and Artificial Intelligence, 2(1), 11–20.** [Referência A8](referencias.md#a8) · [DOI](https://doi.org/10.17509/jmai.v2i1.80389) · [PDF original](https://ejournal.upi.edu/index.php/JMAI/article/download/80389/31419)

**Resumo:** apresenta um dispensador com motor DC, rosca, célula de carga, RTC, teclado e LCD. O controle interrompe a dosagem conforme o peso. Ensaios com dois tipos de ração registram erros de até ±10% nas menores massas testadas.

**Relação com o Lulu:** oferece um exemplo de ensaio quantitativo e de realimentação por pesagem. **Limite:** a tampa comandada por tempo e o sensor IR do Lulu não equivalem ao controle por massa; a precisão daquele protótipo não pode ser atribuída ao nosso.

## Síntese para o projeto

| Decisão de projeto | Fundamentação | Evidência ainda necessária no Lulu |
| --- | --- | --- |
| HTTP local e MQTT remoto | T1 | Latência, reconexão e operação sem nuvem |
| Tarefas, fila e mutex | T2 e F9 | Memória, concorrência e comportamento sob fila cheia |
| Configuração e monitoramento | T3 | Persistência da configuração e diagnóstico de falhas |
| Proteção de comandos | T4 | Autenticação, autorização e resistência à repetição |
| Horários e histórico | U1, U2 e U3 | Execução dos horários e integridade dos registros |
| Calibração de porções | U4 | Massa e variabilidade da ração efetivamente dispensada |

Os artigos de aplicação tratam de alimentadores e de sua utilização, não necessariamente da marca PETLIBRO. Os resultados sobre gatos não são apresentados como validação clínica do Lulu nem extrapolados automaticamente a cães.

