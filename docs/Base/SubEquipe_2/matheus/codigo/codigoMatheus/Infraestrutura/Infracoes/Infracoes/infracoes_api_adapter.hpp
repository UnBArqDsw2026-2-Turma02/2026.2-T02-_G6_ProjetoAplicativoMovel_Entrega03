#pragma once

#include "../../../Aplicacao/Infracoes/Infracoes/gateway_infracoes.hpp"
#include "Mock/api_infracoes_txt_mock.hpp"
using namespace std;

// [Adapter] Adapta as chamadas do Target para a API TXT.
class InfracoesApiAdapter final : public GatewayInfracoes {
public:
    explicit InfracoesApiAdapter(const ApiInfracoesTxtMock& api);

    ResultadoListaInfracoes listarInfracoes(const string& cpfUsuario) const override;
    ResultadoDetalhesInfracao obterDetalhes(const string& idInfracao) const override;

private:
    const ApiInfracoesTxtMock& api_;
};
