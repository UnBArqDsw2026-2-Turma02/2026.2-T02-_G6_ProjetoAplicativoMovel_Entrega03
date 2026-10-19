"""Subject do padrão GoF Observer.

Papel no padrão: Subject (ConcreteSubject).
Guarda o status da habilitação e mantém a lista de observadores,
notificando todos quando o status muda.
"""
import logging

from observador import Observador

logger = logging.getLogger(__name__)

STATUS_VALIDOS = ("PPD Ativa", "Definitiva", "Cassada")


class AcompanhamentoHabilitacao:
    def __init__(self, condutor: str, status: str = "PPD Ativa"):
        if status not in STATUS_VALIDOS:
            raise ValueError(f"Status inválido: {status}")
        self.condutor = condutor
        self._status = status
        self._observadores: list[Observador] = []

    @property
    def status(self) -> str:
        return self._status

    # --- gerenciamento dos observadores (attach / detach) ---
    def adicionar(self, observador: Observador) -> None:
        if observador not in self._observadores:
            self._observadores.append(observador)

    def remover(self, observador: Observador) -> None:
        if observador in self._observadores:
            self._observadores.remove(observador)

    # --- notificação (notify) ---
    def notificar(self) -> None:
        for observador in list(self._observadores):
            try:
                observador.atualizar(self)
            except Exception:
                # um observador com erro não impede que os demais sejam avisados
                logger.exception("Falha ao notificar %s", type(observador).__name__)

    # --- mudança de estado que dispara a notificação ---
    def mudar_status(self, novo_status: str) -> None:
        if novo_status not in STATUS_VALIDOS:
            raise ValueError(f"Status inválido: {novo_status}")
        if novo_status == self._status:
            return  # sem mudança, ninguém é notificado
        self._status = novo_status
        self.notificar()
