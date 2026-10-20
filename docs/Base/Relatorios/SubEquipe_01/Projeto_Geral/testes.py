"""Testes do MVP integrado (python testes.py)."""

import unittest

import aplicacao
from nucleo.base_detran import BaseDetranMultiton
from padroes.consultas import DebitosCreator, NadaConstaCreator
from padroes.documentos import PROTOTIPO_CRLVE, CRLVe


class TesteMultiton(unittest.TestCase):
    def test_mesma_instancia_por_uf(self):
        self.assertIs(BaseDetranMultiton.get_instance("DF"), BaseDetranMultiton("df"))

    def test_ufs_diferentes_tem_bases_e_dados_proprios(self):
        base_distrito_federal = BaseDetranMultiton.get_instance("DF")
        base_goias = BaseDetranMultiton.get_instance("GO")
        self.assertIsNot(base_distrito_federal, base_goias)
        self.assertIsNone(base_goias.buscar_por_placa("JKA1234"))


class TesteFactoryMethodConsultas(unittest.TestCase):
    def setUp(self):
        self.base_distrito_federal = BaseDetranMultiton.get_instance("DF")

    def test_veiculo_com_pendencias(self):
        resultado_debitos = DebitosCreator(self.base_distrito_federal).consultar("98765432100")
        self.assertEqual(len(resultado_debitos.itens), 2)
        self.assertFalse(NadaConstaCreator(self.base_distrito_federal).consultar("98765432100").limpo)

    def test_veiculo_regular(self):
        resposta_consultas = aplicacao.consultar_tudo("DF", "11111111111")["resultados"]
        self.assertTrue(all(resultado["limpo"] for resultado in resposta_consultas.values()))


class TesteAlteracaoEndereco(unittest.TestCase):
    def test_regras_do_uc03(self):
        casos = [
            ("JKA1234", "GO", "74000-000", False, "Distrito Federal"),
            ("JKA1234", "DF", "", False, "CEP obrigatório"),
            ("JKA1234", "DF", "70000-001", True, "caixa postal"),
            ("ABC1D23", "DF", "70000-001", False, "Débitos vencidos"),
            ("ZZZ0000", "DF", "70000-001", False, "não encontrado"),
        ]
        for placa, uf_destino, cep_destino, caixa_postal, trecho_esperado in casos:
            resposta = aplicacao.alterar_endereco("DF", placa, uf_destino, cep_destino, caixa_postal)
            self.assertFalse(resposta["sucesso"], placa)
            self.assertIn(trecho_esperado, resposta["mensagem"])

    def test_alteracao_grava_na_base_unica(self):
        resposta = aplicacao.alterar_endereco("DF", "JKA1234", "DF", "71000-000", False)
        self.assertTrue(resposta["sucesso"])
        veiculo_atualizado = BaseDetranMultiton.get_instance("DF").buscar_por_placa("JKA1234")
        self.assertEqual(veiculo_atualizado.endereco.cep, "71000-000")


class TesteEmissaoCRLVe(unittest.TestCase):
    def test_bloqueio_nao_cria_documento(self):
        quantidade_antes = len(aplicacao.documentos_emitidos)
        resposta = aplicacao.emitir_crlve("DF", "ABC1D23", 2026)
        self.assertFalse(resposta["sucesso"])
        self.assertTrue(resposta["dados"]["pendencias"])
        self.assertEqual(len(aplicacao.documentos_emitidos), quantidade_antes)

    def test_emissoes_independentes_e_prototipo_intacto(self):
        primeira_emissao = aplicacao.emitir_crlve("DF", "JKA1234", 2026)
        segunda_emissao = aplicacao.emitir_crlve("GO", "GOA1B23", 2026)
        self.assertTrue(primeira_emissao["sucesso"] and segunda_emissao["sucesso"])
        self.assertNotEqual(primeira_emissao["dados"]["documento"]["id"],
                            segunda_emissao["dados"]["documento"]["id"])
        self.assertIsNone(PROTOTIPO_CRLVE.placa)
        self.assertIsInstance(aplicacao.documentos_emitidos[-1], CRLVe)
        self.assertTrue(aplicacao.estado_prototipo()["todos_independentes"])


if __name__ == "__main__":
    unittest.main(verbosity=2)
