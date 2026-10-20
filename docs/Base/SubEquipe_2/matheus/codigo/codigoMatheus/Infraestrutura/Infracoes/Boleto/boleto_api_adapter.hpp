#pragma once

#include "../../../Aplicacao/Infracoes/Boleto/gateway_boleto.hpp"
#include "Mock/api_boleto_txt_mock.hpp"
using namespace std;

// [Adapter]
class BoletoApiAdapter final : public GatewayBoleto {
public:
    explicit BoletoApiAdapter(const ApiBoletoTxtMock& api);

    ResultadoConsultaBoleto consultarBoleto(const string& idInfracao) const override;
    ResultadoConsultaBoleto atualizarDesconto(const string& idInfracao,int percentualDesconto) const override;

private:
    const ApiBoletoTxtMock& api_;
};
