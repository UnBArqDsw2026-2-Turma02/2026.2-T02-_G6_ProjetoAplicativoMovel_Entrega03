# Projeto_Geral — MVP da SubEquipe_01

MVP que une, num único sistema, os três GoFs criacionais da SubEquipe_01, sem descaracterizar nenhum deles:

| Integrante | Padrão | Onde está |
|--|--|--|
| Edson Pereira | Factory Method (consultas veiculares) | `padroes/consultas.py` |
| Ricardo Henrique | Factory Method + Multiton (serviços do DETRAN e base por UF) | `padroes/servicos.py` e `nucleo/base_detran.py` |
| Yuri Andrade | Prototype (emissão do CRLV-e) | `padroes/documentos.py` |

Os dados são simulados e o projeto é uma proposta acadêmica, sem relação com a arquitetura real do DETRAN-DF Digital.

## Como executar

Requer Python 3.10 ou superior. Não há dependências externas.

```bash
python server.py        # abre em http://localhost:8000
python server.py 8001   # outra porta
```

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
