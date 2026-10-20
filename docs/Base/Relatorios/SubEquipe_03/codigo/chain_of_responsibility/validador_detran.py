# ConcreteHandler 1.
# Observado na Entrega 1: para C, D e E o app manda procurar o Detran.
from resultado import ENCAMINHADA_AO_DETRAN
from validador import Validador


class ValidadorDetran(Validador):
    def __init__(self, orgao):
        super().__init__()
        self.orgao = orgao

    def tratar(self, solicitacao):
        if solicitacao.categoria_desejada in ["C", "D", "E"]:
            solicitacao.motivo = self.orgao.encaminhar_atendimento_presencial()
            return ENCAMINHADA_AO_DETRAN
        return None
