# ConcreteHandler 3.
# Decisão de projeto nossa (não foi observada como regra): na Entrega 1 só vimos
# a tela dizendo "Não existem bloqueios ativos". Usamos possui_bloqueio_ativo(),
# que já existe no Diagrama de Classes da Entrega 2.
from resultado import RECUSADA
from validador import Validador


class ValidadorBloqueios(Validador):
    def tratar(self, solicitacao):
        if solicitacao.habilitacao.possui_bloqueio_ativo():
            motivos = ", ".join(solicitacao.habilitacao.bloqueios_ativos)
            solicitacao.motivo = "Habilitação com bloqueio ativo: " + motivos + "."
            return RECUSADA
        return None
