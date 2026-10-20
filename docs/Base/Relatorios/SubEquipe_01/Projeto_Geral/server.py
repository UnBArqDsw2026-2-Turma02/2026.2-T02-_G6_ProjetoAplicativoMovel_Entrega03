"""Servidor do MVP: telas em web/ e API JSON sobre os padrões.

Uso:
    python server.py          # http://localhost:8000
    python server.py 8001     # outra porta
"""

import json
import sys
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import parse_qs, urlparse

import aplicacao

PASTA_WEB = Path(__file__).resolve().parent / "web"


def _rota_veiculos(parametros: dict, corpo: dict) -> dict:
    return aplicacao.listar_veiculos(parametros.get("uf", ""))


def _rota_consultas(parametros: dict, corpo: dict) -> dict:
    return aplicacao.consultar_tudo(parametros.get("uf", ""), parametros.get("renavam", ""))


def _rota_prototipo(parametros: dict, corpo: dict) -> dict:
    return aplicacao.estado_prototipo()


def _rota_endereco(parametros: dict, corpo: dict) -> dict:
    return aplicacao.alterar_endereco(corpo.get("uf"), corpo.get("placa"), corpo.get("uf_destino"),
                                      corpo.get("cep_destino"), corpo.get("caixa_postal", False))


def _rota_crlve(parametros: dict, corpo: dict) -> dict:
    return aplicacao.emitir_crlve(corpo.get("uf"), corpo.get("placa"), corpo.get("exercicio", 2026))


ROTAS_GET = {"/api/veiculos": _rota_veiculos, "/api/consultas": _rota_consultas,
             "/api/prototipo": _rota_prototipo}
ROTAS_POST = {"/api/endereco": _rota_endereco, "/api/crlve": _rota_crlve}


class ManipuladorDetran(SimpleHTTPRequestHandler):
    def __init__(self, *argumentos, **opcoes):
        super().__init__(*argumentos, directory=str(PASTA_WEB), **opcoes)

    def do_GET(self):
        endereco_requisicao = urlparse(self.path)
        if endereco_requisicao.path.startswith("/api/"):
            parametros = {chave: valores[0] for chave, valores in parse_qs(endereco_requisicao.query).items()}
            self._responder_api(ROTAS_GET, endereco_requisicao.path, parametros, {})
        else:
            super().do_GET()

    def do_POST(self):
        tamanho_corpo = int(self.headers.get("Content-Length") or 0)
        try:
            corpo = json.loads(self.rfile.read(tamanho_corpo) or b"{}")
        except json.JSONDecodeError:
            self._enviar_json(400, {"erro": "JSON inválido."})
            return
        self._responder_api(ROTAS_POST, urlparse(self.path).path, {}, corpo)

    def _responder_api(self, rotas: dict, caminho: str, parametros: dict, corpo: dict):
        funcao_rota = rotas.get(caminho)
        if not funcao_rota:
            self._enviar_json(404, {"erro": "Rota não encontrada."})
            return
        try:
            self._enviar_json(200, funcao_rota(parametros, corpo))
        except (aplicacao.EntradaInvalida, ValueError, TypeError) as erro:
            self._enviar_json(400, {"erro": str(erro)})

    def _enviar_json(self, status: int, conteudo: dict):
        resposta = json.dumps(conteudo, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(resposta)))
        self.end_headers()
        self.wfile.write(resposta)


if __name__ == "__main__":
    porta = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    print(f"MVP SubEquipe_01 em http://localhost:{porta}")
    ThreadingHTTPServer(("", porta), ManipuladorDetran).serve_forever()
