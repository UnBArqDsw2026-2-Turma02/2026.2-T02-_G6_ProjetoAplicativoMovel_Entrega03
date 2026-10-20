#include <filesystem>
#include <iostream>
#include <locale>
#include <memory>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "Aplicacao/LoginGov/gateway_autenticacao.hpp"
#include "Aplicacao/Infracoes/Infracoes/fluxo_infracoes.hpp"
#include "Aplicacao/SNE/fluxo_adesao_sne.hpp"
#include "Aplicacao/Infracoes/Infracoes/infracao_facade.hpp"
#include "Aplicacao/SNE/sne_facade.hpp"
#include "Infraestrutura/LoginGov/Mock/api_usuarios_arquivo_mock.hpp"
#include "Infraestrutura/LoginGov/autenticacao_login_adapter.hpp"

using namespace std;

// Para os prints de custo nao usar '.' para decimal e sim ','
namespace {
class PontuacaoPtBr final : public numpunct<char> {
protected:
    char do_decimal_point() const override {
        return ',';
    }
};

// Pra poder printar UTF-8 (ç, áàãâ...)
#ifdef _WIN32
bool configurarConsoleUtf8(ostream& erros) {
    DWORD modo = 0;
    const HANDLE entrada = GetStdHandle(STD_INPUT_HANDLE);
    const HANDLE saida = GetStdHandle(STD_OUTPUT_HANDLE);

    if (entrada != INVALID_HANDLE_VALUE && entrada != nullptr &&
        GetConsoleMode(entrada, &modo) && !SetConsoleCP(CP_UTF8)) {
        erros << "Não foi possível configurar a entrada do terminal para UTF-8.\n";
        return false;
    }
    if (saida != INVALID_HANDLE_VALUE && saida != nullptr &&
        GetConsoleMode(saida, &modo) && !SetConsoleOutputCP(CP_UTF8)) {
        erros << "Não foi possível configurar a saída do terminal para UTF-8.\n";
        return false;
    }
    return true;
}
#endif
}  // namespace

// _______________________________________________________________________________
// _______________________________________________________________________________

int main(int argc, char* argv[]) {

//---------------------------------
// Formatação
#ifdef _WIN32
    if (!configurarConsoleUtf8(cerr)) {
        return 6;
    }
#endif
    locale::global( locale(locale::classic(), new PontuacaoPtBr));
    cout.imbue(locale());

//---------------------------------

    const string caminhoArquivo =
        argc > 1 ? argv[1] : "Infraestrutura/LoginGov/Mock/usuarios.txt";

    ApiUsuariosArquivoMock apiUsuarios(caminhoArquivo);

    // smart pointer pra nao precisar dar delete
    unique_ptr<GatewayAutenticacao> autenticacao = make_unique<AutenticacaoLoginAdapter>(apiUsuarios);

    string cpf;
    string senha;
    ResultadoAutenticacao resultado;

//---------------------------------
// LOGIN
    while (true) {
        cout << "CPF: ";
        if (!getline(cin, cpf)) {
            cerr << "Não foi possível ler o CPF.\n";
            return 1;
        }

        cout << "Senha: ";
        if (!getline(cin, senha)) {
            cerr << "Não foi possível ler a senha.\n";
            return 1;
        }

        resultado = autenticacao->autenticar(cpf, senha);

        if (resultado.status == StatusAutenticacao::ERRO) {
            cerr << "Erro de autenticação: " << resultado.mensagem << '\n';
            return 2;
        }

        if (resultado.autenticado()) {
            break;
        }

        cout << resultado.mensagem << " Tente novamente.\n";
    }

    cout << "Login realizado com sucesso.\n"
              << "Bem-vindo(a), " << resultado.usuario->nome << "!\n"
              << "CPF: " << resultado.usuario->cpf << '\n';

//----------------------------------------------------------------------------
// Adesão à SNE

    const string caminhoAdesoes =
        argc > 2 ? argv[2] : "Infraestrutura/SNE/Mock/adesoes_sne.txt";
    SNEFacade sneFacade(caminhoAdesoes);

    if (!resultado.usuario->aderiuSNE) {    // Se ainda nao aderiu (no objeto)
        const ResultadoConsultaFacadeSNE consulta = sneFacade.consultaAdesao(*resultado.usuario); //pesquisa
        if (!consulta.sucesso) {
            cerr << "Erro ao consultar adesão SNE: " << consulta.mensagem << '\n';
            return 4;
        }

        if (consulta.aderiu) {
            cout << "A consulta confirmou que o usuário já aderiu ao SNE.\n";
        } else {
            cout << "A consulta confirmou que o usuário ainda não aderiu ao SNE.\n";
            if (!oferecerAdesaoSNE(*resultado.usuario, sneFacade, cin, cout, cerr)) {
                return 1;
            }
        }
    } else {
        cout << "O usuário já possui adesão ao SNE.\n";
    }

//----------------------------------------------------------------------------
// INFRAçÕES
    const string caminhoInfracoes = argc > 3 ? argv[3] : "Infraestrutura/Infracoes/Infracoes/Mock/infracoes.txt";
    filesystem::path pastaOrgao = argc > 4 ? filesystem::path(argv[4]) : filesystem::path("Infraestrutura/Infracoes/OrgaoAutuador/Mock");
    
    if (pastaOrgao.has_extension()) {
        pastaOrgao = pastaOrgao.parent_path(); // sai do arquivo e volta para a pasta
    }
    
    const string caminhoBoletos = argc > 5 ? argv[5] : "Infraestrutura/Infracoes/Boleto/Mock/boletos.txt";
    const string caminhoSolicitacoesBoleto =
        (pastaOrgao / "orgaoboleto.txt").string();
    const string caminhoSolicitacoesDesconto =
        (pastaOrgao / "orgaodesconto.txt").string();
    InfracaoFacade infracaoFacade(
        caminhoInfracoes, caminhoBoletos,
        caminhoSolicitacoesBoleto, caminhoSolicitacoesDesconto);

    if (!executarFluxoInfracoes(*resultado.usuario, infracaoFacade, sneFacade, cin, cout, cerr)) {
        return 5;
    }

    return 0;
}
