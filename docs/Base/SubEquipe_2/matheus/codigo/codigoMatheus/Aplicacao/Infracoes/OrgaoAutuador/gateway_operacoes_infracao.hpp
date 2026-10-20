#pragma once

#include "../../../Dominios/infracao.hpp"

#include <string>
using namespace std;

struct SolicitacaoDesconto {
    string idInfracao;
    string cpfUsuario;
    int percentual = 0;
    bool termoReconhecimentoAceito = false;
};

struct ResultadoOperacaoInfracao {
    bool sucesso = false;
    string mensagem;
};

struct ResultadoConsultaSolicitacaoBoleto {
    bool sucesso = false;
    bool existe = false;
    string mensagem;
};

// [Adapter: interface]
// Target: operações encaminhadas ao órgão autuador.
class GatewayOperacoesInfracao {
public:
    virtual ~GatewayOperacoesInfracao() = default;

    virtual ResultadoOperacaoInfracao enviarSolicitacaoDesconto(const SolicitacaoDesconto& solicitacao) const = 0;
    virtual ResultadoOperacaoInfracao solicitarBoleto(const string& idInfracao, const string& cpfUsuario) const = 0;
    virtual ResultadoConsultaSolicitacaoBoleto consultarSolicitacaoBoleto(const string& idInfracao, const string& cpfUsuario) const = 0;
};
