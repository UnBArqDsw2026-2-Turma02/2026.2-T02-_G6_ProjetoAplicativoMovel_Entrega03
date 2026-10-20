"""Orquestração usada pelas rotas do servidor.

Cada função aciona os padrões pelos seus participantes (Creators, Multiton e
Client do Prototype) e devolve dicionários prontos para virar JSON.
"""

from nucleo.base_detran import BaseDetranMultiton
from padroes.consultas import CRIADORES
from padroes.documentos import PROTOTIPO_CRLVE
from padroes.servicos import AlteracaoEnderecoCreator, EmissaoCRLVeCreator

UFS_DISPONIVEIS = ("DF", "GO")
documentos_emitidos = []


class EntradaInvalida(ValueError):
    """Erro de entrada do usuário (vira resposta 400)."""


def _validar_uf(unidade_federativa: str) -> str:
    unidade_normalizada = (unidade_federativa or "").upper()
    if unidade_normalizada not in UFS_DISPONIVEIS:
        raise EntradaInvalida(f"UF inválida. Opções: {', '.join(UFS_DISPONIVEIS)}.")
    return unidade_normalizada


def listar_veiculos(unidade_federativa: str) -> dict:
    base_da_uf = BaseDetranMultiton.get_instance(_validar_uf(unidade_federativa))
    return {
        "uf": base_da_uf.uf_id,
        "id_base": id(base_da_uf),
        "veiculos": [veiculo.para_dict() for veiculo in base_da_uf.veiculos()],
    }


def consultar_tudo(unidade_federativa: str, renavam: str) -> dict:
    base_da_uf = BaseDetranMultiton.get_instance(_validar_uf(unidade_federativa))
    if not base_da_uf.buscar_por_renavam(renavam or ""):
        raise EntradaInvalida("Nenhum veículo com esse RENAVAM nesta UF.")
    resultados = {
        nome_consulta: classe_creator(base_da_uf).consultar(renavam).para_dict()
        for nome_consulta, classe_creator in CRIADORES.items()
    }
    return {"renavam": renavam, "id_base": id(base_da_uf), "resultados": resultados}


def alterar_endereco(unidade_federativa: str, placa: str, uf_destino: str,
                     cep_destino: str, caixa_postal: bool) -> dict:
    creator_endereco = AlteracaoEnderecoCreator(_validar_uf(unidade_federativa))
    requisicao = {
        "novo_endereco": {
            "uf": (uf_destino or "").upper(),
            "cep": (cep_destino or "").strip(),
            "is_caixa_postal": bool(caixa_postal),
        }
    }
    return creator_endereco.processar(placa or "", requisicao).para_dict()


def emitir_crlve(unidade_federativa: str, placa: str, exercicio: int) -> dict:
    creator_emissao = EmissaoCRLVeCreator(_validar_uf(unidade_federativa))
    try:
        exercicio_numerico = int(exercicio)
    except (TypeError, ValueError):
        raise EntradaInvalida("O exercício deve ser um ano, por exemplo 2026.") from None
    if not 2000 <= exercicio_numerico <= 2100:
        raise EntradaInvalida("O exercício deve estar entre 2000 e 2100.")
    resultado_emissao = creator_emissao.processar(placa or "", {"exercicio": exercicio_numerico})
    resposta = resultado_emissao.para_dict()
    if resultado_emissao.sucesso:
        documento_emitido = resultado_emissao.dados["documento"]
        documentos_emitidos.append(documento_emitido)
        resposta["dados"] = {"documento": documento_emitido.para_dict()}
    return resposta


def estado_prototipo() -> dict:
    identificadores = [documento.para_dict()["id"] for documento in documentos_emitidos]
    return {
        "prototipo": PROTOTIPO_CRLVE.para_dict(),
        "documentos": [documento.para_dict() for documento in documentos_emitidos],
        "todos_independentes": len(set(identificadores)) == len(identificadores)
        and id(PROTOTIPO_CRLVE) not in identificadores,
    }
