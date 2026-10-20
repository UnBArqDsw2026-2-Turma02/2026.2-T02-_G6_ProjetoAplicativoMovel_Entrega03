# Chain of Responsibility: alteração de categoria da CNH

Padrão GoF comportamental aplicado à validação de uma solicitação de alteração de categoria.
SubEquipe_03 · Entrega 3 · Autor: Artur Fernandes

## Como rodar

    python main.py
    python -m unittest -v

Sem dependências externas.

## Papéis do padrão

| Papel GoF | Classe | Arquivo | Origem |
|---|---|---|---|
| Handler | `Validador` | `validador.py` | Decisão de projeto |
| ConcreteHandler 1 | `ValidadorDetran` | `validador_detran.py` | Observado na Entrega 1 (C, D e E vão ao Detran); gateway do BPMN; `alt` do Diagrama de Sequência (Entrega 2) |
| ConcreteHandler 2 | `ValidadorCompatibilidade` | `validador_compatibilidade.py` | Observado na Entrega 1 (quem tem B recebeu só ACC+B e AB); A e ACC vêm do texto "Como funciona" |
| ConcreteHandler 3 | `ValidadorBloqueios` | `validador_bloqueios.py` | Decisão de projeto; usa `possui_bloqueio_ativo()` do Diagrama de Classes da Entrega 2 |
| Client | `SolicitacaoAlteracaoCategoria` | `solicitacao_alteracao_categoria.py` | Diagrama de Classes da Entrega 2 (`validarElegibilidade()`) |
| apoio | `OrgaoDeTransito`, `Habilitacao` | `orgao_de_transito.py`, `habilitacao.py` | Diagrama de Classes da Entrega 2 |

A regra de bloqueio e a ordem da cadeia são decisões de projeto, não comportamento observado.
