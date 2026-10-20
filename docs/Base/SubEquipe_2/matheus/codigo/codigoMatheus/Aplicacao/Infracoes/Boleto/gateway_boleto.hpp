#pragma once

#include "../../../Dominios/boleto.hpp"

#include <optional>
#include <string>
using namespace std;

struct ResultadoConsultaBoleto {
    bool sucesso = false;
    optional<Boleto> boleto;
    string mensagem;
    bool emAnalise = false;
};

// [Adapter: interface]
// Target: contrato do subsistema para consultar boletos.
class GatewayBoleto {
public:
    virtual ~GatewayBoleto() = default;

    virtual ResultadoConsultaBoleto consultarBoleto(const string& idInfracao) const = 0;
    virtual ResultadoConsultaBoleto atualizarDesconto(const string& idInfracao, int percentualDesconto) const = 0;
};
