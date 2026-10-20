# Handler do padrão: é a base de todos os validadores da cadeia.
from resultado import ACEITA


class Validador:
    def __init__(self):
        self.proximo = None

    def definir_proximo(self, proximo):
        self.proximo = proximo
        return proximo  # devolve o próximo para dar para encadear numa linha só

    def validar(self, solicitacao):
        # Cada validador tenta resolver. Se não resolver (None), passa para o próximo.
        resultado = self.tratar(solicitacao)
        if resultado is not None:
            return resultado
        if self.proximo is not None:
            return self.proximo.validar(solicitacao)
        return ACEITA  # chegou ao fim da cadeia sem ninguém barrar

    def tratar(self, solicitacao):
        raise NotImplementedError("cada validador implementa o seu tratar()")
