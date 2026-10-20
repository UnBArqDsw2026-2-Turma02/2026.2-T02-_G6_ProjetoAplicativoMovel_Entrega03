class Habilitacao:
    def __init__(self, categoria, bloqueios_ativos=None):
        self.categoria = categoria
        self.bloqueios_ativos = bloqueios_ativos or []

    def possui_bloqueio_ativo(self):
        return len(self.bloqueios_ativos) > 0
