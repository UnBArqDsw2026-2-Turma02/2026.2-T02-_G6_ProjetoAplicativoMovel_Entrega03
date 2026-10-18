from abc import ABC, abstractmethod
from dataclasses import dataclass, field

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


# Testando

if __name__ == "__main__":
    base = BaseDetran()
    renavam = "98765432100"  # tem debitos e restrição

    print("\n== FACTORY METHOD ==")
    for creator in (DebitosCreator(base), RestricoesCreator(base),
                    FinanciamentosCreator(base), NadaConstaCreator(base)):
        print(creator.consultar(renavam))