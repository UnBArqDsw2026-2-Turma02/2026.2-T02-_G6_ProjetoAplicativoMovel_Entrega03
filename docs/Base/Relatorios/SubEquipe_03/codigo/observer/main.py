"""Demonstração do Observer: mudança de status da habilitação notifica os observadores."""
from acompanhamento_habilitacao import AcompanhamentoHabilitacao
from observadores import NotificacaoPush, NotificacaoEmail, HistoricoAuditoria


def main():
    acompanhamento = AcompanhamentoHabilitacao("Condutor Exemplo")
    push, email, auditoria = NotificacaoPush(), NotificacaoEmail(), HistoricoAuditoria()

    print("=== 1. Registrando os 3 observadores ===")
    for obs in (push, email, auditoria):
        acompanhamento.adicionar(obs)

    print("\n=== 2. PPD concluída sem infração: Definitiva ===")
    acompanhamento.mudar_status("Definitiva")

    print("\n=== 3. Removendo o e-mail; nova mudança notifica só os demais ===")
    acompanhamento.remover(email)
    acompanhamento.mudar_status("Cassada")

    print("\n=== 4. Mesmo status de novo: ninguém é notificado ===")
    acompanhamento.mudar_status("Cassada")

    print(f"\nPush: {len(push.enviadas)} | E-mail: {len(email.enviadas)} | "
          f"Auditoria: {len(auditoria.registros)}")


if __name__ == "__main__":
    main()