from nucleo.modelos import Endereco, Veiculo


def _dados_iniciais(uf_id: str) -> dict[str, Veiculo]:
    """Dados de exemplo por UF (simulação; não são dados reais do DETRAN)."""
    veiculos_por_uf = {
        "DF": [
            Veiculo("JKA1234", "11111111111", "Ricardo", Endereco("DF", "70000-000")),
            Veiculo("ABC1D23", "98765432100", "Edson", Endereco("DF", "70100-000"),
                    debitos=["IPVA 2026: R$ 1.250,00", "Multa: R$ 195,23"],
                    restricoes=["Restrição judicial (RENAJUD)"]),
            Veiculo("XYZ9A87", "55555555555", "Yuri", Endereco("DF", "70200-000"),
                    financiamentos=["Alienação fiduciária - Banco X"]),
        ],
        "GO": [
            Veiculo("GOA1B23", "22222222222", "Proprietário GO", Endereco("GO", "74000-000")),
        ],
    }
    return {veiculo.placa: veiculo for veiculo in veiculos_por_uf.get(uf_id, [])}


# Multiton (Ricardo): no máximo uma base por UF, compartilhada por todo o sistema
class BaseDetranMultiton:
    _instances: dict[str, "BaseDetranMultiton"] = {}

    def __new__(cls, uf_id: str):
        uf_id = uf_id.upper()
        if uf_id not in cls._instances:
            instance = super().__new__(cls)
            instance.uf_id = uf_id
            instance._veiculos = _dados_iniciais(uf_id)
            cls._instances[uf_id] = instance
        return cls._instances[uf_id]

    @classmethod
    def get_instance(cls, uf_id: str) -> "BaseDetranMultiton":
        return cls(uf_id)

    def veiculos(self) -> list[Veiculo]:
        return list(self._veiculos.values())

    def buscar_por_placa(self, placa: str) -> Veiculo | None:
        return self._veiculos.get(placa.upper())

    def buscar_por_renavam(self, renavam: str) -> Veiculo | None:
        return next((veiculo for veiculo in self._veiculos.values() if veiculo.renavam == renavam), None)
