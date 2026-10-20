# Classe do Diagrama de Classes da Entrega 2.
# Na Entrega 1 vimos que o app manda procurar o Detran para as categorias C, D e E.
class OrgaoDeTransito:
    def __init__(self, uf):
        self.uf = uf

    def encaminhar_atendimento_presencial(self):
        return "Procure o Detran-" + self.uf + " para obter essa categoria."
