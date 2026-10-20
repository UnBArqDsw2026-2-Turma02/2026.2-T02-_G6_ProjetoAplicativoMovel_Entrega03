#pragma once

#include "Mock/api_sne_mock.hpp"
#include "../../Aplicacao/SNE/gateway_adesao_sne.hpp"
using namespace std;

// [Adapter] Converte a resposta da API SNE mockada no Target.
class AdesaoSNEApiAdapter final : public GatewayAdesaoSNE {
public:
    explicit AdesaoSNEApiAdapter(const ApiSNEMock& api);

    ResultadoGatewayAdesaoSNE consultarAdesao(const string& cpfUsuario) const override;
    ResultadoGatewayAdesaoSNE solicitarAdesao(const RequisicaoAdesaoSNE& requisicao) const override;

private:
    const ApiSNEMock& api_;
};
