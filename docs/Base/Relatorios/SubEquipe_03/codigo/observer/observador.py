"""Observer (interface) do padrão GoF Observer.

Papel no padrão: Observer.
Declara a operação que o Subject chama quando o seu estado muda.
"""
from abc import ABC, abstractmethod


class Observador(ABC):
    @abstractmethod
    def atualizar(self, acompanhamento) -> None:
        """Chamado pelo Subject a cada mudança de status."""