from abc import ABC, abstractmethod
from typing import Dict, List
from dataclasses import dataclass

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

# ==========================================
# TESTE DO PADRÃO EM EXECUÇÃO
# ==========================================
if __name__ == "__main__":
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
