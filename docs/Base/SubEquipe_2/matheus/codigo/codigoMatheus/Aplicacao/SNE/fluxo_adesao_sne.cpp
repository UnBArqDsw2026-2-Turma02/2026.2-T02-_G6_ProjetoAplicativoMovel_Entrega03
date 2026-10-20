#include "fluxo_adesao_sne.hpp"

#include <iostream>
#include <string>
using namespace std;
namespace {
bool lerConfirmacao(istream& entrada, ostream& saida, const string& pergunta, bool& resposta) {
    while (true) {
        saida << pergunta << " (s/n): ";
        string texto;
        if (!getline(entrada, texto)) {
            return false;
        }
        if (texto == "s" || texto == "S") {
            resposta = true;
            return true;
        }
        if (texto == "n" || texto == "N") {
            resposta = false;
            return true;
        }
        saida << "Resposta inválida. Digite s ou n.\n";
    }
}

bool lerCampo(istream& entrada, ostream& saida, const string& pergunta, string& valor) {
    saida << pergunta;
    return static_cast<bool>(getline(entrada, valor));  // guarda a entrada em valor e retorna se a leitura falhou ou n
}
}  // namespace

bool oferecerAdesaoSNE(Usuario& usuario, SNEFacade& facade, istream& entrada, ostream& saida, ostream& erros) {
  
    if (usuario.aderiuSNE)
        return true;

    bool desejaAderir = false;
    if (!lerConfirmacao(entrada, saida, "Deseja aderir ao Sistema de Notificação Eletrônica (SNE) agora?", desejaAderir))
        return false;
    
    if (!desejaAderir) {
        saida << "Você poderá aderir ao SNE posteriormente.\n";
        return true;
    }

    bool termosAceitos = false;
    if (!lerConfirmacao(entrada, saida, "Aceita os termos de adesão ao SNE?", termosAceitos)) // so retorna false em caso de erro no getline
        return false;
    
    if (!termosAceitos) {
        saida << "Adesão cancelada: é necessário aceitar os termos.\n";
        return true;
    }

    saida << "Documento para validação (1 - CNH, 2 - CRLV): ";
    string escolha;
    if (!getline(entrada, escolha)) {
        return false;
    }
    if (escolha != "1" && escolha != "2") {
        erros << "Tipo de documento inválido. Adesão não realizada.\n";
        return true;
    }

    DadosAdesaoSNE dados;
    const TipoDocumentoSNE tipoDocumento = escolha == "1" ? TipoDocumentoSNE::CNH : TipoDocumentoSNE::CRLV;
    if (tipoDocumento == TipoDocumentoSNE::CNH) {
        if (!lerCampo(entrada, saida, "Número da CNH: ", dados.numeroCNH) || !lerCampo(entrada, saida, "Código de segurança da CNH: ", dados.codigoSegurancaCNH)) {
            return false;
        }
    } else {
        if (!lerCampo(entrada, saida, "Placa do veículo: ", dados.placa) || !lerCampo(entrada, saida, "RENAVAM: ", dados.renavam) || !lerCampo(entrada, saida, "Número de segurança do CRV: ", dados.numeroSegurancaCRV)) {
            return false;
        }
    }

    bool confirmarDados = false;
    if (!lerConfirmacao(entrada, saida, "Confirma os dados informados?",
                        confirmarDados)) {
        return false;
    }
    if (!confirmarDados) { // entrada = 'n'
        saida << "Adesão cancelada.\n";
        return true;
    }

    const ResultadoAdesaoFacadeSNE resultado = facade.aderir(usuario, tipoDocumento, dados, termosAceitos);
    (resultado.sucesso ? saida : erros) << resultado.mensagem << '\n';
    return true;
}
