# Demonstração do Chain of Responsibility (dados fictícios).
from habilitacao import Habilitacao
from orgao_de_transito import OrgaoDeTransito
from solicitacao_alteracao_categoria import SolicitacaoAlteracaoCategoria
from validador_bloqueios import ValidadorBloqueios
from validador_compatibilidade import ValidadorCompatibilidade
from validador_detran import ValidadorDetran


def montar_cadeia(orgao):
    primeiro = ValidadorDetran(orgao)
    primeiro.definir_proximo(ValidadorCompatibilidade()).definir_proximo(ValidadorBloqueios())
    return primeiro


def testar(titulo, habilitacao, desejada, cadeia):
    solicitacao = SolicitacaoAlteracaoCategoria(habilitacao, desejada)
    resultado = solicitacao.validar_elegibilidade(cadeia)
    print("\n[" + titulo + "]")
    print("  atual:", habilitacao.categoria, "| desejada:", desejada)
    print("  resultado:", resultado)
    if solicitacao.motivo:
        print("  motivo:", solicitacao.motivo)


if __name__ == "__main__":
    cadeia = montar_cadeia(OrgaoDeTransito("GO"))

    testar("1. caminho feliz", Habilitacao("B"), "AB", cadeia)
    testar("2. categoria presencial", Habilitacao("B"), "C", cadeia)
    testar("3. categoria incompatível", Habilitacao("B"), "A", cadeia)
    testar("4. bloqueio ativo", Habilitacao("B", ["suspensão do direito de dirigir"]), "AB", cadeia)
