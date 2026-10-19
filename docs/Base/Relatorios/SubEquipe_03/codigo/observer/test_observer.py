import unittest
from acompanhamento_habilitacao import AcompanhamentoHabilitacao
from observador import Observador
from observadores import NotificacaoPush, NotificacaoEmail, HistoricoAuditoria


class ObservadorFalso(Observador):
    def __init__(self):
        self.chamadas = 0

    def atualizar(self, acompanhamento):
        self.chamadas += 1


class ObservadorComErro(Observador):
    def atualizar(self, acompanhamento):
        raise RuntimeError("falha simulada")


class TestObserver(unittest.TestCase):
    def setUp(self):
        self.sujeito = AcompanhamentoHabilitacao("Maria")

    def test_1_status_inicial_e_ppd_ativa(self):
        self.assertEqual(self.sujeito.status, "PPD Ativa")

    def test_2_observador_e_notificado_na_mudanca(self):
        obs = ObservadorFalso()
        self.sujeito.adicionar(obs)
        self.sujeito.mudar_status("Definitiva")
        self.assertEqual(obs.chamadas, 1)

    def test_3_todos_os_observadores_sao_notificados(self):
        a, b = ObservadorFalso(), ObservadorFalso()
        self.sujeito.adicionar(a)
        self.sujeito.adicionar(b)
        self.sujeito.mudar_status("Cassada")
        self.assertEqual((a.chamadas, b.chamadas), (1, 1))

    def test_4_observador_removido_nao_e_notificado(self):
        a, b = ObservadorFalso(), ObservadorFalso()
        self.sujeito.adicionar(a)
        self.sujeito.adicionar(b)
        self.sujeito.remover(a)
        self.sujeito.mudar_status("Definitiva")
        self.assertEqual((a.chamadas, b.chamadas), (0, 1))

    def test_5_mesmo_status_nao_notifica(self):
        obs = ObservadorFalso()
        self.sujeito.adicionar(obs)
        self.sujeito.mudar_status("PPD Ativa")
        self.assertEqual(obs.chamadas, 0)

    def test_6_status_invalido_levanta_erro_e_nao_notifica(self):
        obs = ObservadorFalso()
        self.sujeito.adicionar(obs)
        with self.assertRaises(ValueError):
            self.sujeito.mudar_status("Suspensa")
        self.assertEqual(obs.chamadas, 0)
        self.assertEqual(self.sujeito.status, "PPD Ativa")

    def test_7_adicionar_duas_vezes_notifica_uma_vez(self):
        obs = ObservadorFalso()
        self.sujeito.adicionar(obs)
        self.sujeito.adicionar(obs)
        self.sujeito.mudar_status("Definitiva")
        self.assertEqual(obs.chamadas, 1)

    def test_8_observadores_concretos_reagem_ao_novo_status(self):
        push, email, aud = NotificacaoPush(), NotificacaoEmail(), HistoricoAuditoria()
        for o in (push, email, aud):
            self.sujeito.adicionar(o)
        self.sujeito.mudar_status("Cassada")
        self.assertIn("cassada", push.enviadas[0])
        self.assertIn("Cassada", email.enviadas[0])
        self.assertEqual(aud.registros, ["Maria -> Cassada"])

    def test_9_erro_em_um_observador_nao_impede_os_demais(self):
        ruim, bom = ObservadorComErro(), ObservadorFalso()
        self.sujeito.adicionar(ruim)
        self.sujeito.adicionar(bom)
        with self.assertLogs("acompanhamento_habilitacao", level="ERROR"):
            self.sujeito.mudar_status("Cassada")
        self.assertEqual(bom.chamadas, 1)
        self.assertEqual(self.sujeito.status, "Cassada")


if __name__ == "__main__":
    unittest.main(verbosity=2)
