"""Factory Method + Multiton - serviços do DETRAN (Ricardo Henrique).

A emissão do CRLV-e entra como mais um serviço criado pelo Factory Method e
integra os três padrões: valida com o Nada Consta (Edson) e cria o documento
por clonagem (Yuri).
"""

from abc import ABC, abstractmethod

from nucleo.base_detran import BaseDetranMultiton
from nucleo.modelos import DadosVeiculo, Endereco, ResultadoOperacao
from padroes.consultas import NadaConstaCreator
from padroes.documentos import PROTOTIPO_CRLVE, EmissorCRLVe


# Product
class ServicoDetran(ABC):
    def __init__(self, base: BaseDetranMultiton) -> None:
        self._base = base

    @abstractmethod
    def executar(self, placa: str, dados_requisicao: dict) -> ResultadoOperacao:
        pass


# Concrete Product - regras do UC03
class ServicoAlteracaoEndereco(ServicoDetran):
    def executar(self, placa: str, dados_requisicao: dict) -> ResultadoOperacao:
        novo_endereco = dados_requisicao.get("novo_endereco", {})
        veiculo = self._base.buscar_por_placa(placa)

        if not veiculo:
            return ResultadoOperacao(False, "Veículo não encontrado.")

        if novo_endereco.get("uf") != "DF":
            return ResultadoOperacao(False, "Endereço deve estar no Distrito Federal. (Requer outro fluxo)")

        cep = novo_endereco.get("cep")
        if not cep or novo_endereco.get("is_caixa_postal"):
            return ResultadoOperacao(False, "CEP obrigatório e não pode ser caixa postal.")

        if veiculo.debitos_vencidos:
            return ResultadoOperacao(False, "Débitos vencidos devem estar quitados.")

        if veiculo.possui_restricoes:
            return ResultadoOperacao(False, "Não pode existir restrição administrativa ou judicial.")

        veiculo.endereco = Endereco(novo_endereco["uf"], cep, False)
        return ResultadoOperacao(True, "Endereço cadastrado do veículo atualizado, sem cobrança.",
                                 {"veiculo": veiculo.para_dict()})


# Concrete Product - integração dos três padrões
class ServicoEmissaoCRLVe(ServicoDetran):
    def executar(self, placa: str, dados_requisicao: dict) -> ResultadoOperacao:
        veiculo = self._base.buscar_por_placa(placa)
        if not veiculo:
            return ResultadoOperacao(False, "Veículo não encontrado.")

        # Factory Method (Edson): a emissão só segue com "nada consta"
        nada_consta = NadaConstaCreator(self._base).consultar(veiculo.renavam)
        if not nada_consta.limpo:
            return ResultadoOperacao(False, "Emissão bloqueada: existem pendências no veículo.",
                                     {"pendencias": nada_consta.itens})

        # Prototype (Yuri): o documento é criado por clonagem do modelo-base
        exercicio = dados_requisicao.get("exercicio", 2026)
        documento = EmissorCRLVe(PROTOTIPO_CRLVE).emitir(
            DadosVeiculo(veiculo.placa, veiculo.renavam, exercicio))
        return ResultadoOperacao(True, "CRLV-e emitido.", {"documento": documento})


# Creator
class ServicoCreator(ABC):
    def __init__(self, uf_id: str) -> None:
        # Integração com o Multiton para injetar a base de dados correta
        self._base = BaseDetranMultiton.get_instance(uf_id)

    @abstractmethod
    def criar_servico(self) -> ServicoDetran:
        """Factory Method"""
        pass

    def processar(self, placa: str, dados_requisicao: dict) -> ResultadoOperacao:
        servico = self.criar_servico()
        return servico.executar(placa, dados_requisicao)


# Concrete Creators
class AlteracaoEnderecoCreator(ServicoCreator):
    def criar_servico(self) -> ServicoDetran:
        return ServicoAlteracaoEndereco(self._base)


class EmissaoCRLVeCreator(ServicoCreator):
    def criar_servico(self) -> ServicoDetran:
        return ServicoEmissaoCRLVe(self._base)
