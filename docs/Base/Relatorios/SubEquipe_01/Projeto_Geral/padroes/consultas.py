"""Factory Method - consultas veiculares (Edson Pereira)."""

from abc import ABC, abstractmethod

from nucleo.base_detran import BaseDetranMultiton
from nucleo.modelos import ResultadoConsulta, Veiculo


# Product
class Consulta(ABC):
    def __init__(self, base: BaseDetranMultiton) -> None:
        self._base = base

    def _veiculo(self, renavam: str) -> Veiculo | None:
        return self._base.buscar_por_renavam(renavam)

    @abstractmethod
    def executar(self, renavam: str) -> ResultadoConsulta: ...


# Concrete Products
class ConsultaDebitos(Consulta):
    def executar(self, renavam: str) -> ResultadoConsulta:
        veiculo = self._veiculo(renavam)
        return ResultadoConsulta("Débitos", list(veiculo.debitos) if veiculo else [])


class ConsultaRestricoes(Consulta):
    def executar(self, renavam: str) -> ResultadoConsulta:
        veiculo = self._veiculo(renavam)
        return ResultadoConsulta("Restrições", list(veiculo.restricoes) if veiculo else [])


class ConsultaFinanciamentos(Consulta):
    def executar(self, renavam: str) -> ResultadoConsulta:
        veiculo = self._veiculo(renavam)
        return ResultadoConsulta("Financiamentos", list(veiculo.financiamentos) if veiculo else [])


class ConsultaNadaConsta(Consulta):
    def executar(self, renavam: str) -> ResultadoConsulta:
        veiculo = self._veiculo(renavam)
        itens = (veiculo.debitos + veiculo.restricoes + veiculo.financiamentos) if veiculo else []
        return ResultadoConsulta("Nada Consta", list(itens))


# Creator
class ConsultaCreator(ABC):
    def __init__(self, base: BaseDetranMultiton) -> None:
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


# Permite à tela escolher o Creator sem if/elif
CRIADORES: dict[str, type[ConsultaCreator]] = {
    "debitos": DebitosCreator,
    "restricoes": RestricoesCreator,
    "financiamentos": FinanciamentosCreator,
    "nada_consta": NadaConstaCreator,
}
