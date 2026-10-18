# ConcreteHandler 2.
# Observado na Entrega 1: para quem tem B, o app ofereceu só ACC+B e AB.
# As linhas de A e ACC vêm do texto "Como funciona" do app, mas não foram testadas.
from resultado import RECUSADA
from validador import Validador

OPCOES = {
    "B": ["ACC+B", "AB"],
    "A": ["AB"],
    "ACC": ["A", "AB", "ACC+B"],
}


class ValidadorCompatibilidade(Validador):
    def tratar(self, solicitacao):
        atual = solicitacao.habilitacao.categoria
        permitidas = OPCOES.get(atual, [])
        if solicitacao.categoria_desejada not in permitidas:
            solicitacao.motivo = ("A categoria " + solicitacao.categoria_desejada
                                  + " não está disponível para quem tem " + atual + ".")
            return RECUSADA
        return None
