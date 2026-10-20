"""Prototype - emissão do CRLV-e (Yuri Andrade)."""

from abc import ABC, abstractmethod
from copy import deepcopy

from nucleo.modelos import DadosVeiculo


# Prototype
class DocumentoVeicular(ABC):

    @abstractmethod
    def clonar(self):
        """Cria uma cópia do documento."""
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

    def para_dict(self) -> dict:
        return {
            "tipo": self.tipo,
            "orgao_emissor": self.orgao_emissor,
            "versao_layout": self.versao_layout,
            "placa": self.placa,
            "renavam": self.renavam,
            "exercicio": self.exercicio,
            "id": id(self),
        }

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


# Objeto-base único, criado uma vez e reutilizado em todas as emissões
PROTOTIPO_CRLVE = CRLVe()
