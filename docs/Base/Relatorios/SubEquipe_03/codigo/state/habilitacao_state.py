from __future__ import annotations

from abc import ABC, abstractmethod
from dataclasses import dataclass
from datetime import date, timedelta
from enum import Enum

class Gravidade(Enum):
    LEVE = "leve"
    MEDIA = "media"
    GRAVE = "grave"
    GRAVISSIMA = "gravissima"


@dataclass(frozen=True)
class Infracao:
    codigo: str
    gravidade: Gravidade
    data: date


class TransicaoInvalida(Exception):
    """Operacao que nao faz sentido no estado atual da habilitacao."""

class EstadoHabilitacao(ABC):
    """Declara as operacoes cujo comportamento depende do estado."""

    nome: str = "?"

    def ao_entrar(self, habilitacao: "Habilitacao") -> None:
        """Acao de entrada (equivale ao `entry /` da Maquina de Estados)."""

    @abstractmethod
    def registrar_infracao(self, habilitacao: "Habilitacao", infracao: Infracao) -> None:
        ...

    @abstractmethod
    def concluir_prazo(self, habilitacao: "Habilitacao", hoje: date) -> None:
        ...

    @abstractmethod
    def pode_dirigir(self) -> bool:
        ...

class PPDAtiva(EstadoHabilitacao):
    """Estado inicial: Permissao Para Dirigir valida por 12 meses.

    Atividade continua `monitorarHistoricoInfracoes()` (Maquina de Estados - Entrega 2):
    cada infracao e apenas registrada; a decisao ocorre ao fim do prazo.
    """

    nome = "PPD Ativa"

    def registrar_infracao(self, habilitacao, infracao):
        habilitacao.historico_infracoes.append(infracao)
        print(f"   [PPD Ativa] infracao {infracao.gravidade.value} registrada no historico.")

    def concluir_prazo(self, habilitacao, hoje):
        if hoje < habilitacao.data_limite_ppd:
            raise TransicaoInvalida(
                f"Prazo de 12 meses ainda nao terminou (limite: {habilitacao.data_limite_ppd})."
            )
        if self._tem_impedimento(habilitacao):
            habilitacao.mudar_estado(Cassada())
        else:
            habilitacao.mudar_estado(Definitiva())

    def pode_dirigir(self):
        return True

    @staticmethod
    def _tem_impedimento(habilitacao) -> bool:
        """Regra do CTB, art. 148, par. 3: nao pode haver infracao grave ou
        gravissima, nem reincidencia em infracao media."""
        historico = habilitacao.historico_infracoes
        graves = [i for i in historico if i.gravidade in (Gravidade.GRAVE, Gravidade.GRAVISSIMA)]
        medias = [i for i in historico if i.gravidade is Gravidade.MEDIA]
        return bool(graves) or len(medias) > 1

class Definitiva(EstadoHabilitacao):
    """CNH definitiva: infracoes passam a ser apenas registradas (pontuacao fora do escopo)."""

    nome = "Definitiva"

    def ao_entrar(self, habilitacao):
        habilitacao.notificar_condutor("Parabens! Sua CNH agora e DEFINITIVA.")

    def registrar_infracao(self, habilitacao, infracao):
        habilitacao.historico_infracoes.append(infracao)
        print("   [Definitiva] infracao registrada; o estado nao muda.")

    def concluir_prazo(self, habilitacao, hoje):
        raise TransicaoInvalida("A habilitacao ja e definitiva: nao ha prazo de PPD a concluir.")

    def pode_dirigir(self):
        return True

class Cassada(EstadoHabilitacao):
    """Estado final. A PPD foi cassada: o condutor deve reiniciar a 1a habilitacao,
    o que gera um NOVO objeto Habilitacao (por isso nao ha volta para PPD Ativa)."""

    nome = "Cassada"

    def ao_entrar(self, habilitacao):
        habilitacao.notificar_condutor(
            "Sua PPD foi CASSADA. E necessario reiniciar o processo de 1a habilitacao."
        )

    def registrar_infracao(self, habilitacao, infracao):
        raise TransicaoInvalida("Habilitacao cassada: nao aceita novos registros.")

    def concluir_prazo(self, habilitacao, hoje):
        raise TransicaoInvalida("Habilitacao cassada: estado final.")

    def pode_dirigir(self):
        return False

class Habilitacao:
    """Context do padrao. Os atributos `status`, `data_emissao_ppd` e
    `data_limite_ppd` vem do Diagrama de Classes da Entrega 2."""

    def __init__(self, numero_registro: str, nome_condutor: str, data_emissao_ppd: date):
        self.numero_registro = numero_registro
        self.nome_condutor = nome_condutor
        self.data_emissao_ppd = data_emissao_ppd
        self.data_limite_ppd = data_emissao_ppd + timedelta(days=365)
        self.historico_infracoes: list[Infracao] = []
        self._estado: EstadoHabilitacao = PPDAtiva()

    @property
    def status(self) -> str:
        return self._estado.nome

    def mudar_estado(self, novo_estado: EstadoHabilitacao) -> None:
        print(f"   >> transicao: {self._estado.nome} -> {novo_estado.nome}")
        self._estado = novo_estado
        novo_estado.ao_entrar(self)

    def registrar_infracao(self, infracao: Infracao) -> None:
        self._estado.registrar_infracao(self, infracao)

    def concluir_prazo(self, hoje: date) -> None:
        self._estado.concluir_prazo(self, hoje)

    def pode_dirigir(self) -> bool:
        return self._estado.pode_dirigir()

    def notificar_condutor(self, mensagem: str) -> None:
        print(f"   [notificacao para {self.nome_condutor}] {mensagem}")

def _cenario(titulo: str, infracoes: list[Infracao]) -> Habilitacao:
    print("\n" + "=" * 70)
    print(titulo)
    print("=" * 70)
    hab = Habilitacao("00000000001", "Condutor Ficticio", date(2025, 10, 1))
    print(f"   estado inicial: {hab.status} | pode dirigir? {hab.pode_dirigir()}")
    for infracao in infracoes:
        hab.registrar_infracao(infracao)
    hab.concluir_prazo(hoje=date(2026, 10, 1))
    print(f"   estado final:   {hab.status} | pode dirigir? {hab.pode_dirigir()}")
    return hab

def main() -> None:
    _cenario("Cenario 1 - PPD sem infracao", [])

    _cenario("Cenario 2 - PPD com infracao GRAVE",
             [Infracao("I-01", Gravidade.GRAVE, date(2026, 3, 10))])

    _cenario("Cenario 3 - PPD com REINCIDENCIA em infracao media",
             [Infracao("I-02", Gravidade.MEDIA, date(2026, 1, 5)),
              Infracao("I-03", Gravidade.MEDIA, date(2026, 6, 20))])

    hab = _cenario("Cenario 4 - Operacao invalida em estado final (Cassada)",
                   [Infracao("I-04", Gravidade.GRAVISSIMA, date(2026, 2, 1))])
    try:
        hab.registrar_infracao(Infracao("I-05", Gravidade.LEVE, date(2026, 10, 2)))
    except TransicaoInvalida as erro:
        print(f"   operacao recusada: {erro}")


if __name__ == "__main__":
    main()
