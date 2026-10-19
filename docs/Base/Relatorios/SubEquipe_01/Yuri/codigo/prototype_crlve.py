from abc import ABC, abstractmethod
from copy import deepcopy
from dataclasses import dataclass


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


# Execução demonstrativa
if __name__ == "__main__":

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
