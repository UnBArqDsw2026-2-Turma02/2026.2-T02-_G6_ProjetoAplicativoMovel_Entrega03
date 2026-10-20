import os
import sys
import unittest
from datetime import date

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from habilitacao_state import (Cassada, Definitiva, Gravidade, Habilitacao,
                               Infracao, PPDAtiva, TransicaoInvalida)

FIM_PRAZO = date(2026, 10, 1)


def nova_hab():
    return Habilitacao("1", "Teste", date(2025, 10, 1))


class TestHabilitacaoState(unittest.TestCase):
    def test_estado_inicial_e_ppd_ativa(self):
        self.assertIsInstance(nova_hab()._estado, PPDAtiva)

    def test_sem_infracao_vira_definitiva(self):
        h = nova_hab(); h.concluir_prazo(FIM_PRAZO)
        self.assertIsInstance(h._estado, Definitiva)

    def test_infracao_leve_vira_definitiva(self):
        h = nova_hab(); h.registrar_infracao(Infracao("a", Gravidade.LEVE, date(2026, 1, 1)))
        h.concluir_prazo(FIM_PRAZO)
        self.assertEqual(h.status, "Definitiva")

    def test_infracao_grave_cassa(self):
        h = nova_hab(); h.registrar_infracao(Infracao("a", Gravidade.GRAVE, date(2026, 1, 1)))
        h.concluir_prazo(FIM_PRAZO)
        self.assertIsInstance(h._estado, Cassada)
        self.assertFalse(h.pode_dirigir())

    def test_reincidencia_media_cassa(self):
        h = nova_hab()
        for d in (date(2026, 1, 1), date(2026, 2, 1)):
            h.registrar_infracao(Infracao("m", Gravidade.MEDIA, d))
        h.concluir_prazo(FIM_PRAZO)
        self.assertEqual(h.status, "Cassada")

    def test_prazo_nao_concluido(self):
        with self.assertRaises(TransicaoInvalida):
            nova_hab().concluir_prazo(date(2026, 5, 1))

    def test_cassada_e_estado_final(self):
        h = nova_hab(); h.registrar_infracao(Infracao("a", Gravidade.GRAVISSIMA, date(2026, 1, 1)))
        h.concluir_prazo(FIM_PRAZO)
        with self.assertRaises(TransicaoInvalida):
            h.registrar_infracao(Infracao("b", Gravidade.LEVE, date(2026, 11, 1)))
        with self.assertRaises(TransicaoInvalida):
            h.concluir_prazo(FIM_PRAZO)


if __name__ == "__main__":
    unittest.main(verbosity=2)
