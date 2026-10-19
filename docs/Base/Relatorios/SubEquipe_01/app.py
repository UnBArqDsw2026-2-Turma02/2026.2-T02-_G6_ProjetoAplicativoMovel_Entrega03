"""Código integrado da SubEquipe_01 - GoFs Criacionais.

Reúne, em um único arquivo, os padrões implementados por cada integrante.
As versões individuais estão em <Integrante>/codigo/.

Execução:
    python app.py              # executa as três demonstrações
    python app.py prototype    # executa apenas uma (factory, multiton ou prototype)
"""

import sys
from abc import ABC, abstractmethod
from copy import deepcopy
from dataclasses import dataclass, field
from typing import Dict


# ==========================================================================
# EDSON PEREIRA - Factory Method: consulta veicular
# Página: Edson/1.1.1.Entrega_Edson.md
# ==========================================================================

class BaseDetran:
    """Simulação das fontes de dados que o app consulta (inferido, não observado)."""

    def __init__(self) -> None:
        self.debitos = {"98765432100": ["IPVA 2026: R$ 1.250,00", "Multa: R$ 195,23"]}
        self.restricoes = {"98765432100": ["Restrição judicial (RENAJUD)"]}
        self.financiamentos = {"55555555555": ["Alienação fiduciária - Banco X"]}

@dataclass
class ResultadoConsulta:
    tipo: str
    itens: list[str] = field(default_factory=list)

    @property
    def limpo(self) -> bool:
        return not self.itens

    def __str__(self) -> str:
        if self.limpo:
            return f"[{self.tipo}] nada consta"
        return f"[{self.tipo}] " + "; ".join(self.itens)


# Product
class Consulta(ABC):
    def __init__(self, base: BaseDetran) -> None:
        self._base = base

    @abstractmethod
    def executar(self, renavam: str) -> ResultadoConsulta: ...


# Concrete Products
class ConsultaDebitos(Consulta):
    def executar(self, renavam: str) -> ResultadoConsulta:
        return ResultadoConsulta("Débitos", self._base.debitos.get(renavam, []))


class ConsultaRestricoes(Consulta):
    def executar(self, renavam: str) -> ResultadoConsulta:
        return ResultadoConsulta("Restrições", self._base.restricoes.get(renavam, []))


class ConsultaFinanciamentos(Consulta):
    def executar(self, renavam: str) -> ResultadoConsulta:
        return ResultadoConsulta("Financiamentos", self._base.financiamentos.get(renavam, []))


class ConsultaNadaConsta(Consulta):
    def executar(self, renavam: str) -> ResultadoConsulta:
        itens = (
            self._base.debitos.get(renavam, [])
            + self._base.restricoes.get(renavam, [])
            + self._base.financiamentos.get(renavam, [])
        )
        return ResultadoConsulta("Nada Consta", itens)

# Creator
class ConsultaCreator(ABC):
    def __init__(self, base: BaseDetran) -> None:
        self._base = base

    @abstractmethod
    def criar_consulta(self) -> Consulta:
        """Factory Method: subclasses decidem qual Consulta instanciar."""

    def consultar(self, renavam: str) -> ResultadoConsulta:
        consulta = self.criar_consulta()
        return consulta.executar(renavam)


# Concrete Creators
class DebitosCreator(ConsultaCreator):
    def criar_consulta(self) -> Consulta:
        return ConsultaDebitos(self._base)


class RestricoesCreator(ConsultaCreator):
    def criar_consulta(self) -> Consulta:
        return ConsultaRestricoes(self._base)


class FinanciamentosCreator(ConsultaCreator):
    def criar_consulta(self) -> Consulta:
        return ConsultaFinanciamentos(self._base)


class NadaConstaCreator(ConsultaCreator):
    def criar_consulta(self) -> Consulta:
        return ConsultaNadaConsta(self._base)


# ==========================================================================
# RICARDO HENRIQUE - Factory Method + Multiton: alteração de endereço (UC03)
# Página: Ricardo/1.1.1.Entrega_Ricardo.md
# ==========================================================================

# ==========================================
# 1. MULTITON: Gerenciador de Bases do Detran
# ==========================================
class BaseDetranMultiton:
    """
    Multiton que permite a criação de instâncias limitadas da base de dados,
    gerenciadas por uma chave (ex: por Unidade Federativa).
    """
    _instances: Dict[str, 'BaseDetranMultiton'] = {}

    def __new__(cls, uf_id: str):
        if uf_id not in cls._instances:
            instance = super().__new__(cls)
            instance.uf_id = uf_id
            # Simulação do banco de dados de veículos (Placa -> Dados)
            instance.veiculos = {
                "JKA1234": {
                    "proprietario": "Ricardo",
                    "debitos_vencidos": False,
                    "restricoes": False,
                    "endereco_atual": {"uf": "DF", "cep": "70000-000"}
                }
            }
            cls._instances[uf_id] = instance
        return cls._instances[uf_id]

    @classmethod
    def get_instance(cls, uf_id: str) -> 'BaseDetranMultiton':
        return cls(uf_id)

@dataclass
class ResultadoOperacao:
    sucesso: bool
    mensagem: str

# ==========================================
# 2. FACTORY METHOD: Products
# ==========================================
class ServicoDetran(ABC):
    """
    Product: define a interface para os serviços que serão criados pelo factory method.
    """
    def __init__(self, base: BaseDetranMultiton) -> None:
        self._base = base

    @abstractmethod
    def executar(self, placa: str, dados_requisicao: dict) -> ResultadoOperacao:
        pass

class ServicoAlteracaoEndereco(ServicoDetran):
    """
    Concrete Product: Implementa as regras do fluxo UC03.
    """
    def executar(self, placa: str, dados_requisicao: dict) -> ResultadoOperacao:
        novo_endereco = dados_requisicao.get("novo_endereco", {})
        veiculo = self._base.veiculos.get(placa)

        if not veiculo:
            return ResultadoOperacao(False, "Veículo não encontrado.")

        # Validações do UC03
        if novo_endereco.get("uf") != "DF":
            return ResultadoOperacao(False, "Endereço deve estar no Distrito Federal. (Requer outro fluxo)")

        cep = novo_endereco.get("cep")
        if not cep or novo_endereco.get("is_caixa_postal"):
            return ResultadoOperacao(False, "CEP obrigatório e não pode ser caixa postal.")

        if veiculo.get("debitos_vencidos"):
            return ResultadoOperacao(False, "Débitos vencidos devem estar quitados.")

        if veiculo.get("restricoes"):
            return ResultadoOperacao(False, "Não pode existir restrição administrativa ou judicial.")

        # Satisfeitos os requisitos, atualiza
        veiculo["endereco_atual"] = novo_endereco
        return ResultadoOperacao(True, "Endereço cadastrado do veículo atualizado, sem cobrança.")

# ==========================================
# 3. FACTORY METHOD: Creators
# ==========================================
class ServicoCreator(ABC):
    """
    Creator: declara o factory method que retorna um objeto do tipo ServicoDetran.
    """
    def __init__(self, uf_id: str) -> None:
        # Integração com o Multiton para injetar a base de dados correta
        self._base = BaseDetranMultiton.get_instance(uf_id)

    @abstractmethod
    def criar_servico(self) -> ServicoDetran:
        """Factory Method"""
        pass

    def processar(self, placa: str, dados_requisicao: dict) -> ResultadoOperacao:
        # Delega a instanciação à subclasse e executa a operação principal
        servico = self.criar_servico()
        return servico.executar(placa, dados_requisicao)

class AlteracaoEnderecoCreator(ServicoCreator):
    """
    Concrete Creator: sobrescreve o factory method para instanciar o serviço correto.
    """
    def criar_servico(self) -> ServicoDetran:
        return ServicoAlteracaoEndereco(self._base)


# ==========================================================================
# YURI ANDRADE - Prototype: emissão do CRLV-e (UC02 / FV-02)
# Página: Yuri/1.1.1.Entrega_Yuri.md
# ==========================================================================

@dataclass
class DadosVeiculo:
    placa: str
    renavam: str
    exercicio: int


# Prototype
class DocumentoVeicular(ABC):

    @abstractmethod
    def clonar(self) -> "DocumentoVeicular":
        """Cria uma cópia do documento."""
        ...

    @abstractmethod
    def preencher(self, dados: DadosVeiculo) -> None:
        """Preenche os dados específicos de uma emissão."""
        ...


# ConcretePrototype
class CRLVe(DocumentoVeicular):

    def __init__(self) -> None:
        # Dados comuns ao modelo do documento
        self.tipo = "CRLV-e"
        self.orgao_emissor = "Órgão de trânsito"
        self.versao_layout = "2026.1"

        # Dados específicos de cada emissão
        self.placa = None
        self.renavam = None
        self.exercicio = None

    def clonar(self):
        return deepcopy(self)

    def preencher(self, dados: DadosVeiculo) -> None:
        self.placa = dados.placa
        self.renavam = dados.renavam
        self.exercicio = dados.exercicio

    def __str__(self) -> str:
        return (
            f"{self.tipo} | "
            f"Placa: {self.placa} | "
            f"RENAVAM: {self.renavam} | "
            f"Exercício: {self.exercicio}"
        )


# Client
class EmissorCRLVe:

    def __init__(self, prototipo: DocumentoVeicular) -> None:
        self.prototipo = prototipo

    def emitir(self, dados: DadosVeiculo) -> DocumentoVeicular:
        documento = self.prototipo.clonar()
        documento.preencher(dados)
        return documento


# ==========================================================================
# Demonstrações
# ==========================================================================

def demonstrar_factory_method_consulta() -> None:
    base = BaseDetran()
    renavam = "98765432100"  # tem debitos e restrição

    print("\n== FACTORY METHOD ==")
    for creator in (DebitosCreator(base), RestricoesCreator(base),
                    FinanciamentosCreator(base), NadaConstaCreator(base)):
        print(creator.consultar(renavam))


def demonstrar_factory_method_multiton() -> None:
    # Cliente depende apenas das abstrações para disparar o serviço
    creator = AlteracaoEnderecoCreator(uf_id="DF")

    requisicao = {
        "novo_endereco": {
            "uf": "DF",
            "cep": "71000-000",
            "is_caixa_postal": False
        }
    }

    resultado = creator.processar(placa="JKA1234", dados_requisicao=requisicao)
    print(f"Status: {'[SUCESSO]' if resultado.sucesso else '[FALHA]'} {resultado.mensagem}")


def demonstrar_prototype_crlve() -> None:
    print("\n== PROTOTYPE ==")

    prototipo = CRLVe()
    emissor = EmissorCRLVe(prototipo)

    documento_1 = emissor.emitir(
        DadosVeiculo(
            placa="ABC1D23",
            renavam="12345678900",
            exercicio=2026
        )
    )

    documento_2 = emissor.emitir(
        DadosVeiculo(
            placa="XYZ9A87",
            renavam="98765432100",
            exercicio=2026
        )
    )

    print(documento_1)
    print(documento_2)

    print("\nObjetos independentes:")
    print(documento_1 is not documento_2)

    print("\nPrototype permaneceu sem dados específicos:")
    print(f"Placa: {prototipo.placa}")
    print(f"RENAVAM: {prototipo.renavam}")
    print(f"Exercício: {prototipo.exercicio}")


DEMONSTRACOES = {
    "factory": demonstrar_factory_method_consulta,
    "multiton": demonstrar_factory_method_multiton,
    "prototype": demonstrar_prototype_crlve,
}


if __name__ == "__main__":
    escolhidas = sys.argv[1:] or list(DEMONSTRACOES)
    for nome in escolhidas:
        if nome not in DEMONSTRACOES:
            sys.exit(f"Demonstração desconhecida: {nome}. Opções: {', '.join(DEMONSTRACOES)}")
        DEMONSTRACOES[nome]()
