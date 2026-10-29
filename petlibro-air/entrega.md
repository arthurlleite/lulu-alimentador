# Preparação da entrega 2

[Relatório principal](README.md) · [Enunciado](https://gitlab.com/fse_fga/trabalhos-2026_2/trabalho-2-2026-2)

Prazo do enunciado: **03/11/2026**. Branch de trabalho: **`atualizacoes-antonio`**. A pasta `petlibro-air/` reúne o conteúdo acadêmico, mantendo o desenvolvimento do Lulu nas pastas existentes.

## Cronograma de referência

Os marcos abaixo orientam a revisão do grupo. A documentação foi adiantada em 29/10/2026; as datas não representam execução futura automaticamente agendada.

| Marco | Entregável | Estado desta revisão |
| --- | --- | --- |
| 29–30/10 | Oito artigos e identificação | Quatro de tecnologias e quatro de aplicação selecionados/resumidos, com DOI e PDF; quatro nomes registrados; falta a matrícula de Antônio. |
| 30–31/10 | Características e comparativo | Doze grupos e seus subitens documentados; cinco produtos adicionais; lacunas técnicas explicitadas como NI. |
| 01/11 | Proposta, diagramas e referências | Mapeamento para ESP32-S3, componentes, diagramas e limitações redigidos; ensaios descritos como futuros. |
| 02/11 | Revisão final e estrutura | Pasta do produto com README e referências; revisão do grupo e decisão sobre o procedimento de submissão ainda necessárias. |
| 03/11 | Submissão | Executar o procedimento aceito pelo professor e conferir a versão enviada. |

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
- [ ] Informar a matrícula de Antônio Lucas De Araújo Lima.
- [ ] Os quatro integrantes revisarem a interpretação das fontes e os resumos.
- [ ] Abrir os PDFs no navegador e conferir acesso na rede utilizada para avaliação; alguns servidores restringem robôs ou exigem verificação de navegador.
- [ ] Confirmar com o professor o envio por link ou cumprir o procedimento de fork/MR descrito no enunciado.
- [ ] Conferir a branch/commit que será efetivamente enviado.

## Revisão técnica do protótipo

O firmware não foi modificado para resolver os pontos abaixo nesta revisão documental. Eles precisam de implementação e ensaio antes de se afirmar operação autônoma confiável:

| Pendência | Local do código | Resultado esperado após correção |
| --- | --- | --- |
| Evitar repetição de comandos e execução de comandos MQTT retidos | [comandos.c](../firmware/src/comandos.c), [nuvem_mqtt.c](../firmware/src/nuvem_mqtt.c) | Um pedido não provocar dispensações adicionais por reentrega. |
| Tratar fila cheia e reinício durante o minuto de disparo | [agenda.c](../firmware/src/agenda.c) | Política explícita para pedidos recusados, perdidos ou repetidos. |
| Validar números antes da conversão para tipos menores | [comandos.c](../firmware/src/comandos.c), [dosador.c](../firmware/src/dosador.c) | Rejeitar valores inválidos sem truncamento inesperado. |
| Propagar falha de persistência | [estado.c](../firmware/src/estado.c) | Não afirmar que salvou quando a NVS falhar. |
| Restringir reconfiguração local e autorizar acesso remoto | [servidor_web.c](../firmware/src/servidor_web.c), [nuvem_mqtt.c](../firmware/src/nuvem_mqtt.c) | Somente clientes autorizados alterarem configuração ou acionarem o motor. |
| Testar detecção óptica e quantidade efetivamente dispensada | [dosador.c](../firmware/src/dosador.c) | Diferenciar passagem detectada de confirmação de todas as porções. |
| Integrar sensores à estrutura e verificar movimento/fixação | [CAD](../hardware/3d/README.md) | Montagem mensurável, desmontável e sem interferências no curso previsto. |

## Como revisar esta branch

1. Abrir `petlibro-air/README.md` na branch `atualizacoes-antonio`.
2. Seguir a navegação para características, artigos, proposta e comparativo.
3. Conferir a fonte de cada afirmação, especialmente os itens NI.
4. Revisar os dados da equipe e registrar a matrícula pendente.
5. Manter a distinção entre funcionalidades no código, proposta conceitual e testes realmente executados.

## Estrutura para a submissão

O grupo continua desenvolvendo neste repositório. O enunciado escrito pede fork do repositório da disciplina e merge request contendo uma pasta do produto. A pasta acadêmica criada aqui já tem esse formato e pode ser levada posteriormente ao fork, ajustando os links relativos que apontam para firmware/hardware fora dela.

Se a pasta for copiada isoladamente, também deverão ser incluídos os materiais do Lulu referenciados ou substituídos esses links por URLs permanentes do commit deste projeto. Não copiar somente o README e deixar os arquivos auxiliares de fora.

Caso o professor autorize receber apenas o link deste repositório, usar a URL da **branch ou de um commit específico que contenha o relatório**, para evitar apontar à `main` antes da incorporação das mudanças. O procedimento de fork/MR não foi executado nesta etapa; a revisão documental não equivale à submissão.

## Colaboração

Contas identificadas: `Devv-Antonio1`, `Amandaaaaabreu`, `Depaiiva` e `arthurlleite`. A lista acadêmica não concede acesso ao projeto. Conferir os convites em [Manage → Members](https://gitlab.com/Devv-Antonio1/lulu-alimentador/-/project_members), usando Developer para trabalho por branches/merge requests. [Documentação do GitLab](https://docs.gitlab.com/user/project/members/).

