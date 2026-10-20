import unittest

from habilitacao import Habilitacao
from main import montar_cadeia
from orgao_de_transito import OrgaoDeTransito
from resultado import ACEITA, RECUSADA, ENCAMINHADA_AO_DETRAN
from solicitacao_alteracao_categoria import SolicitacaoAlteracaoCategoria
from validador import Validador
from validador_bloqueios import ValidadorBloqueios
from validador_detran import ValidadorDetran


def validar(atual, desejada, bloqueios=None):
    cadeia = montar_cadeia(OrgaoDeTransito("GO"))
    solicitacao = SolicitacaoAlteracaoCategoria(Habilitacao(atual, bloqueios), desejada)
    return solicitacao, solicitacao.validar_elegibilidade(cadeia)


class TestCadeia(unittest.TestCase):
    def test_caminho_feliz(self):
        solicitacao, resultado = validar("B", "AB")
        self.assertEqual(resultado, ACEITA)
        self.assertEqual(solicitacao.status, ACEITA)

    def test_categoria_presencial_vai_para_o_detran(self):
        for categoria in ["C", "D", "E"]:
            solicitacao, resultado = validar("B", categoria)
            self.assertEqual(resultado, ENCAMINHADA_AO_DETRAN)
            self.assertIn("Detran-GO", solicitacao.motivo)

    def test_categoria_incompativel_e_recusada(self):
        solicitacao, resultado = validar("B", "A")
        self.assertEqual(resultado, RECUSADA)
        self.assertIn("não está disponível", solicitacao.motivo)

    def test_bloqueio_ativo_recusa(self):
        solicitacao, resultado = validar("B", "AB", ["suspensão"])
        self.assertEqual(resultado, RECUSADA)
        self.assertIn("bloqueio ativo", solicitacao.motivo)

    def test_cadeia_para_quando_um_validador_resolve(self):
        class Espiao(Validador):
            foi_chamado = False

            def tratar(self, solicitacao):
                Espiao.foi_chamado = True
                return None

        primeiro = ValidadorDetran(OrgaoDeTransito("GO"))
        primeiro.definir_proximo(Espiao())
        solicitacao = SolicitacaoAlteracaoCategoria(Habilitacao("B"), "C")
        solicitacao.validar_elegibilidade(primeiro)
        self.assertFalse(Espiao.foi_chamado)

    def test_a_ordem_da_cadeia_muda_o_resultado(self):
        # Com bloqueio e categoria C ao mesmo tempo, quem vem primeiro decide a resposta.
        bloqueios_primeiro = ValidadorBloqueios()
        bloqueios_primeiro.definir_proximo(ValidadorDetran(OrgaoDeTransito("GO")))
        solicitacao = SolicitacaoAlteracaoCategoria(Habilitacao("B", ["suspensão"]), "C")
        self.assertEqual(solicitacao.validar_elegibilidade(bloqueios_primeiro), RECUSADA)

        _, resultado = validar("B", "C", ["suspensão"])  # cadeia padrão: Detran primeiro
        self.assertEqual(resultado, ENCAMINHADA_AO_DETRAN)


if __name__ == "__main__":
    unittest.main()
