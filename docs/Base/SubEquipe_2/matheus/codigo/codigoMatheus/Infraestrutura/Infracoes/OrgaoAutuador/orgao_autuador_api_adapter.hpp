#pragma once

#include <Aplicacao/Infracoes/OrgaoAutuador/gateway_operacoes_infracao.hpp>
#include "Mock/api_orgao_autuador_txt_mock.hpp"
using namespace std;

// [Adapter]
class OrgaoAutuadorApiAdapter final : public GatewayOperacoesInfracao {
public:
    explicit OrgaoAutuadorApiAdapter(const ApiOrgaoAutuadorTxtMock& api);

    ResultadoOperacaoInfracao enviarSolicitacaoDesconto(const SolicitacaoDesconto& solicitacao) const override;
    ResultadoOperacaoInfracao solicitarBoleto(const string& idInfracao,const string& cpfUsuario) const override;
    ResultadoConsultaSolicitacaoBoleto consultarSolicitacaoBoleto(const string& idInfracao,const string& cpfUsuario) const override;

private:
    const ApiOrgaoAutuadorTxtMock& api_;
};
