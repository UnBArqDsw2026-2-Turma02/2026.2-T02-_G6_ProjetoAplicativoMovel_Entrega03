from dataclasses import asdict, dataclass, field


@dataclass
class Endereco:
    uf: str
    cep: str
    caixa_postal: bool = False


@dataclass
class Veiculo:
    """Veículo da base única, compartilhado pelos três padrões."""

    placa: str
    renavam: str
    proprietario: str
    endereco: Endereco
    debitos: list[str] = field(default_factory=list)
    restricoes: list[str] = field(default_factory=list)
    financiamentos: list[str] = field(default_factory=list)

    # Semântica usada nas regras do UC03 (Ricardo)
    @property
    def debitos_vencidos(self) -> bool:
        return bool(self.debitos)

    @property
    def possui_restricoes(self) -> bool:
        return bool(self.restricoes)

    def para_dict(self) -> dict:
        return asdict(self)


# Edson - retorno padronizado das consultas
@dataclass
class ResultadoConsulta:
    tipo: str
    itens: list[str] = field(default_factory=list)

    @property
    def limpo(self) -> bool:
        return not self.itens

    def __str__(self) -> str:
        if self.limpo:
            return f"[{self.tipo}] nada consta"
        return f"[{self.tipo}] " + "; ".join(self.itens)

    def para_dict(self) -> dict:
        return {"tipo": self.tipo, "itens": self.itens, "limpo": self.limpo}


# Ricardo - retorno dos serviços do DETRAN
@dataclass
class ResultadoOperacao:
    sucesso: bool
    mensagem: str
    dados: dict | None = None

    def para_dict(self) -> dict:
        return asdict(self)


# Yuri - dados variáveis de cada emissão
@dataclass
class DadosVeiculo:
    placa: str
    renavam: str
    exercicio: int
