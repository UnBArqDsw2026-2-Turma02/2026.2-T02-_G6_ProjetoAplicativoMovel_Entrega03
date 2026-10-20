# Client do padrão: é ela que manda a solicitação para a cadeia.
class SolicitacaoAlteracaoCategoria:
    def __init__(self, habilitacao, categoria_desejada):
        self.habilitacao = habilitacao
        self.categoria_desejada = categoria_desejada
        self.status = "Em análise"
        self.motivo = ""

    def validar_elegibilidade(self, primeiro_validador):
        resultado = primeiro_validador.validar(self)
        self.status = resultado
        return resultado
