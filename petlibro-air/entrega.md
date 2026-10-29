# Preparação da entrega 2

[Relatório principal](README.md) · [Enunciado](https://gitlab.com/fse_fga/trabalhos-2026_2/trabalho-2-2026-2)

Prazo do enunciado: **03/11/2026**. Branch de trabalho: **`atualizacoes-antonio`**. A pasta `petlibro-air/` reúne o conteúdo acadêmico, mantendo o desenvolvimento do Lulu nas pastas existentes.

## Cronograma de referência

Os marcos abaixo orientam a revisão do grupo. A documentação foi adiantada em 29/10/2026; as datas não representam execução futura automaticamente agendada.

| Marco | Entregável | Estado desta revisão |
| --- | --- | --- |
| 29–30/10 | Oito artigos e identificação | Quatro de tecnologias e quatro de aplicação selecionados/resumidos, com DOI e PDF; nomes e matrículas dos quatro integrantes registrados. |
| 30–31/10 | Características e comparativo | Doze grupos e seus subitens documentados; cinco produtos adicionais; lacunas técnicas explicitadas como NI. |
| 01/11 | Proposta, diagramas e referências | Mapeamento para ESP32-S3, componentes, diagramas e limitações redigidos; ensaios descritos como futuros. |
| 02/11 | Revisão final e estrutura | Pasta do produto com README e referências; revisão do grupo e decisão sobre o procedimento de submissão ainda necessárias. |
| 03/11 | Submissão | Executar o procedimento aceito pelo professor e conferir a versão enviada. |

## Conferência com o enunciado

| Exigência do enunciado | Onde está | Situação |
| --- | --- | --- |
| Produto comercial embarcado com sensores e atuadores | [README](README.md): PETLIBRO Air PLAF108 | Atendido |
| 3.1 Funções, público, contexto, segmento e aplicações | [README](README.md#31-descrição-do-produto-selecionado) | Atendido |
| 3.1 Fotos do produto e da placa interna | [README](README.md): Figuras 1 e 2, com fonte | Atendido (imagens por link da fonte original) |
| 3.1 Tecnologias de comunicação e controle | [README](README.md) e [características](caracteristicas.md) | Atendido |
| 3.2 Doze grupos de características, cada item com fonte | [caracteristicas.md](caracteristicas.md) | Atendido; itens não encontrados marcados como NI |
| 3.2 Tecnologias críticas destacadas | [caracteristicas.md](caracteristicas.md) | Atendido |
| 3.3 Implementação com ESP32 / ESP-IDF | [proposta.md](proposta.md) | Atendido, com firmware real |
| 3.3 Mapeamento 3.2 → ESP32 e Lista de Sensores | [proposta.md](proposta.md#mapeamento-das-características) | Atendido; peças já adquiridas |
| 3.3 Diagrama conceitual ou esquemático | [proposta.md](proposta.md#esquemático-e-montagem): blocos, esquemático e posicionamento | Atendido |
| 3.3 Limitações e desafios | [proposta.md](proposta.md#limitações-e-desafios) | Atendido |
| 3.4 Quatro artigos de periódico sobre tecnologias | [artigos.md](artigos.md): T1–T4 (+ T5–T7) | Atendido; tipo e DOI conferidos na Crossref |
| 3.4 Quatro artigos de periódico sobre aplicação | [artigos.md](artigos.md): U1–U4 (+ U5–U6) | Atendido; tipo e DOI conferidos na Crossref |
| 3.4 Resumo de cada artigo e link do PDF original | [artigos.md](artigos.md) | Atendido; nenhum PDF no repositório |
| 3.5 Cinco produtos e tabela pelos itens da listagem | [comparativo.md](comparativo.md) | Atendido |
| Referências completas ao final | [referencias.md](referencias.md) | Atendido |
| Nome completo e matrícula dos integrantes | [README](README.md#equipe) | Atendido |
| Pasta com nome do produto, sem acento/espaço, `README.md` principal | `petlibro-air/` | Atendido |
| Markdown; imagens em formato renderizável | `.md` e `imagens/*.png` | Atendido |
| Fork do repositório da disciplina + MR | — | **Pendente**: ver abaixo |

## Conferência do conteúdo

- [x] Produto comercial identificado como PETLIBRO Air PLAF108.
- [x] Funções, público-alvo, segmento e contexto de uso.
- [x] Imagem promocional do produto e fotografia de uma placa, com fontes e atribuição.
- [x] Doze grupos de características com seus subitens e fontes.
- [x] Tecnologias críticas destacadas.
- [x] Recursos disponíveis no chip separados do uso comprovado no produto.
- [x] Informações não identificadas declaradas, sem pressupor ausência.
- [x] Reprodução parcial com ESP32 e correspondência com a lista de sensores.
- [x] Diagramas conceituais, limitações e plano de validação.
- [x] Quatro artigos de periódicos sobre tecnologias.
- [x] Quatro artigos de periódicos sobre aplicações/uso.
- [x] Cada artigo com resumo, aplicação ao projeto, limite da inferência e link de PDF.
- [x] Cinco outros modelos comerciais no comparativo.
- [x] Referências bibliográficas e técnicas reunidas.
- [x] Nenhuma cópia de PDF acadêmico adicionada à pasta.
- [x] README principal em pasta sem espaços/acentos, nomeada pelo produto.
- [x] Identificação dos quatro integrantes completa; matrícula de Antônio Lucas De Araújo Lima: 241025597.
- [ ] Os quatro integrantes revisarem a interpretação das fontes e os resumos.
- [ ] Abrir os PDFs no navegador e conferir acesso na rede utilizada para avaliação; alguns servidores restringem robôs ou exigem verificação de navegador.
- [ ] Confirmar com o professor o envio por link ou cumprir o procedimento de fork/MR descrito no enunciado.
- [ ] Conferir a branch/commit que será efetivamente enviado.

## Revisão técnica do protótipo

O firmware não foi modificado para resolver os pontos abaixo nesta revisão documental. Eles precisam de implementação e ensaio antes de se afirmar operação autônoma confiável:

| Pendência | Local do código | Resultado esperado após correção |
| --- | --- | --- |
| Evitar repetição de comandos e execução de comandos MQTT retidos | [comandos.c](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/firmware/src/comandos.c), [nuvem_mqtt.c](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/firmware/src/nuvem_mqtt.c) | Um pedido não provocar dispensações adicionais por reentrega. |
| Tratar fila cheia e reinício durante o minuto de disparo | [agenda.c](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/firmware/src/agenda.c) | Política explícita para pedidos recusados, perdidos ou repetidos. |
| Validar números antes da conversão para tipos menores | [comandos.c](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/firmware/src/comandos.c), [dosador.c](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/firmware/src/dosador.c) | Rejeitar valores inválidos sem truncamento inesperado. |
| Propagar falha de persistência | [estado.c](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/firmware/src/estado.c) | Não afirmar que salvou quando a NVS falhar. |
| Restringir reconfiguração local e autorizar acesso remoto | [servidor_web.c](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/firmware/src/servidor_web.c), [nuvem_mqtt.c](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/firmware/src/nuvem_mqtt.c) | Somente clientes autorizados alterarem configuração ou acionarem o motor. |
| Testar detecção óptica e quantidade efetivamente dispensada | [dosador.c](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/firmware/src/dosador.c) | Diferenciar passagem detectada de confirmação de todas as porções. |
| Integrar sensores à estrutura e verificar movimento/fixação | [CAD](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/blob/main/hardware/3d/README.md) | Montagem mensurável, desmontável e sem interferências no curso previsto. |

## Como revisar esta branch

1. Abrir `petlibro-air/README.md` na branch `atualizacoes-antonio`.
2. Seguir a navegação para características, artigos, proposta e comparativo.
3. Conferir a fonte de cada afirmação, especialmente os itens NI.
4. Conferir os nomes e matrículas dos quatro integrantes.
5. Manter a distinção entre funcionalidades no código, proposta conceitual e testes realmente executados.

## Estrutura para a submissão

A pasta `petlibro-air/` é autossuficiente: as figuras do Lulu estão em `imagens/` e os links para firmware, hardware e documentação apontam para a `main` do [projeto no GitLab](https://gitlab.com/Devv-Antonio1/lulu-alimentador). Esses links funcionam depois que esta branch for incorporada à `main`.

Passos para entregar:

1. Incorporar esta branch à `main` do projeto (para os links absolutos funcionarem).
2. Fazer **fork** de [fse_fga/trabalhos-2026_2/trabalho-2-2026-2](https://gitlab.com/fse_fga/trabalhos-2026_2/trabalho-2-2026-2).
3. Copiar a pasta `petlibro-air/` inteira (com `imagens/`) para a raiz do fork, numa branch própria.
4. Abrir o **MR** do fork para o repositório da disciplina até **03/11/2026** e conferir no MR se as imagens e os links abrem.

## Colaboração

Contas identificadas: `Devv-Antonio1`, `Amandaaaaabreu`, `Depaiiva` e `arthurlleite`. A lista acadêmica não concede acesso ao projeto. Conferir os convites em [Manage → Members](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/project_members), usando Developer para trabalho por branches/merge requests. [Documentação do GitLab](https://docs.gitlab.com/user/project/members/).

