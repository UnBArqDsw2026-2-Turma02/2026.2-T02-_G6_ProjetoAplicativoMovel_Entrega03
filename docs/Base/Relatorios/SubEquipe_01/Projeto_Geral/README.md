# Projeto_Geral — MVP da SubEquipe_01

MVP que une, num único sistema, os três GoFs criacionais da SubEquipe_01, sem descaracterizar nenhum deles:

| Integrante | Padrão | Onde está |
|--|--|--|
| Edson Pereira | Factory Method (consultas veiculares) | `padroes/consultas.py` |
| Ricardo Henrique | Factory Method + Multiton (serviços do DETRAN e base por UF) | `padroes/servicos.py` e `nucleo/base_detran.py` |
| Yuri Andrade | Prototype (emissão do CRLV-e) | `padroes/documentos.py` |

Os dados são simulados e o projeto é uma proposta acadêmica, sem relação com a arquitetura real do DETRAN-DF Digital.

**Pasta do projeto no GitHub:** [`docs/Base/Relatorios/SubEquipe_01/Projeto_Geral`](https://github.com/UnBArqDsw2026-2-Turma02/2026.2-T02-_G6_ProjetoAplicativoMovel_Entrega03/tree/feature/subequipe-1/docs/Base/Relatorios/SubEquipe_01/Projeto_Geral)

## Como executar

**Requisito:** Python 3.10 ou superior (`python --version`). Não há bibliotecas para instalar.

1. Clone o repositório e entre na branch da SubEquipe_01:

   ```bash
   git clone https://github.com/UnBArqDsw2026-2-Turma02/2026.2-T02-_G6_ProjetoAplicativoMovel_Entrega03.git
   cd 2026.2-T02-_G6_ProjetoAplicativoMovel_Entrega03
   git checkout feature/subequipe-1
   ```

   Sem Git, também dá para baixar o ZIP da branch `feature/subequipe-1` no GitHub (botão **Code → Download ZIP**) e extrair.

2. Entre na pasta do MVP e inicie o servidor:

   ```bash
   cd docs/Base/Relatorios/SubEquipe_01/Projeto_Geral
   python server.py
   ```

   No Windows, se `python` não for reconhecido, use `py server.py`.

3. Abra **http://localhost:8000** no navegador.

Se a porta 8000 estiver ocupada, use outra: `python server.py 8001` e abra `http://localhost:8001`. Para encerrar, pressione `Ctrl+C` no terminal.

**Roteiro sugerido:**
1. Na tela **Início**, escolha a UF **DF** e o veículo **ABC1D23**.
2. Em **Consultas**, veja as pendências.
3. Em **CRLV-e**, a emissão é bloqueada.
4. Volte, escolha **JKA1234**, altere o endereço em **Endereço** e emita o **CRLV-e** duas vezes.
5. O painel "Prova do Prototype" mostra documentos com `id` diferentes e o protótipo vazio.

## Como os padrões se unem

1. **Multiton (Ricardo):** `BaseDetranMultiton.get_instance(uf)` mantém **uma base por UF**, e todas as telas usam a mesma instância. Ela é a fonte única de veículos, débitos, restrições, financiamentos e endereços.
2. **Factory Method (Edson):** os Creators (`DebitosCreator`, `RestricoesCreator`, `FinanciamentosCreator` e `NadaConstaCreator`) criam as consultas, que leem dessa base.
3. **Factory Method (Ricardo):** `ServicoCreator` cria os serviços do DETRAN.
   - `AlteracaoEnderecoCreator` aplica as regras do UC03 e grava na base.
   - `EmissaoCRLVeCreator` é o novo par produto-creator que integra os três padrões.
4. **Prototype (Yuri):** o `ServicoEmissaoCRLVe` primeiro consulta o **Nada Consta** (Edson). Só sem pendências o `EmissorCRLVe` clona o `CRLVe` base e preenche placa, RENAVAM e exercício. Com pendências, nenhum clone é criado.

## Telas

| Tela | O que faz | Padrão |
|--|--|--|
| `index.html` | Escolha da UF e do veículo e situação do veículo | Multiton |
| `consultas.html` | Débitos, restrições, financiamentos e nada consta | Factory Method (Edson) |
| `endereco.html` | Alteração de endereço com as regras do UC03 | Factory Method + Multiton (Ricardo) |
| `crlve.html` | Emissão do CRLV-e e prova de independência dos clones | Prototype (Yuri) |
| `padroes.html` | Participantes de cada padrão e fluxo integrado | — |

Cada ação gera uma notificação (sucesso, aviso ou erro). As telas seguem o fluxo **escolher veículo → consultar → atualizar endereço → emitir CRLV-e**. A seleção do veículo fica salva entre as telas.

## API

| Método | Rota | Função em `aplicacao.py` |
|--|--|--|
| GET | `/api/veiculos?uf=DF` | `listar_veiculos` |
| GET | `/api/consultas?uf=DF&renavam=…` | `consultar_tudo` |
| POST | `/api/endereco` | `alterar_endereco` |
| POST | `/api/crlve` | `emitir_crlve` |
| GET | `/api/prototipo` | `estado_prototipo` |

## Dados de exemplo

| UF | Placa | RENAVAM | Situação |
|--|--|--|--|
| DF | JKA1234 | 11111111111 | Regular: altera endereço e emite CRLV-e |
| DF | ABC1D23 | 98765432100 | Débitos e restrição: tudo bloqueado |
| DF | XYZ9A87 | 55555555555 | Financiamento: altera endereço, mas não emite CRLV-e |
| GO | GOA1B23 | 22222222222 | Regular, na base de GO (outra instância do Multiton) |

Os dados ficam em memória e voltam ao estado inicial quando o servidor reinicia.
