#pragma once

#include "../../../Dominios/infracao.hpp"

#include <optional>
#include <string>
#include <vector>
using namespace std;

struct ResultadoListaInfracoes {
    bool sucesso = false;
    vector<Infracao> infracoes;
    string mensagem;
};

struct ResultadoDetalhesInfracao {
    bool sucesso = false;
    optional<Infracao> infracao;
    string mensagem;
};

// [Adapter: interface]
// Target: contrato usado pelo fluxo para consultar infrações.
class GatewayInfracoes {
public:
    virtual ~GatewayInfracoes() = default;

    virtual ResultadoListaInfracoes listarInfracoes(const string& cpfUsuario) const = 0;
    virtual ResultadoDetalhesInfracao obterDetalhes(const string& idInfracao) const = 0;
};
