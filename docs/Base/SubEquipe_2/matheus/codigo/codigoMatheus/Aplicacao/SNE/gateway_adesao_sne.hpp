#pragma once

#include "estrategia_documento_sne.hpp"

#include <string>
using namespace std;

struct ResultadoGatewayAdesaoSNE {
    bool sucesso = false;
    bool aderiu = false;
    string mensagem;
};

// [Adapter: interface]
// Target: contrato  para consultar a adesão  SNE
class GatewayAdesaoSNE {
public:
    virtual ~GatewayAdesaoSNE() = default;

    virtual ResultadoGatewayAdesaoSNE consultarAdesao(const string& cpfUsuario) const = 0;
    virtual ResultadoGatewayAdesaoSNE solicitarAdesao(const RequisicaoAdesaoSNE& requisicao) const = 0;
};
