"""ConcreteObservers do padrão GoF Observer.

Cada observador reage à mudança de status do seu jeito,
sem que o Subject precise conhecer os detalhes de cada um.
"""
from observador import Observador

MENSAGENS = {
    "PPD Ativa": "Sua Permissão Para Dirigir está ativa. Mantenha-se sem infrações por 12 meses.",
    "Definitiva": "Parabéns! Sua CNH agora é definitiva.",
    "Cassada": "Sua Permissão Para Dirigir foi cassada. Procure o Detran.",
}


class NotificacaoPush(Observador):
    def __init__(self):
        self.enviadas: list[str] = []

    def atualizar(self, acompanhamento) -> None:
        msg = f"[PUSH] {acompanhamento.condutor}: {MENSAGENS[acompanhamento.status]}"
        self.enviadas.append(msg)
        print(msg)


class NotificacaoEmail(Observador):
    def __init__(self):
        self.enviadas: list[str] = []

    def atualizar(self, acompanhamento) -> None:
        msg = (f"[E-MAIL] Para: {acompanhamento.condutor} | "
               f"Assunto: Habilitação {acompanhamento.status}")
        self.enviadas.append(msg)
        print(msg)


class HistoricoAuditoria(Observador):
    def __init__(self):
        self.registros: list[str] = []

    def atualizar(self, acompanhamento) -> None:
        reg = f"{acompanhamento.condutor} -> {acompanhamento.status}"
        self.registros.append(reg)
        print(f"[AUDITORIA] {reg}")