#include "orgao_autuador_api_adapter.hpp"

#include <string>
using namespace std;

OrgaoAutuadorApiAdapter::OrgaoAutuadorApiAdapter(
    const ApiOrgaoAutuadorTxtMock& api)
    : api_(api) {}

ResultadoOperacaoInfracao OrgaoAutuadorApiAdapter::enviarSolicitacaoDesconto(
    const SolicitacaoDesconto& solicitacao) const {
    
        // Conversão para formato para API
    
        return api_.registrarSolicitacaoDesconto(solicitacao);
}

ResultadoOperacaoInfracao OrgaoAutuadorApiAdapter::solicitarBoleto(
    const string& idInfracao,
    const string& cpfUsuario) const {
    
        // Conversão para formato para API
    
    return api_.registrarSolicitacaoBoleto(idInfracao, cpfUsuario);
}

ResultadoConsultaSolicitacaoBoleto
OrgaoAutuadorApiAdapter::consultarSolicitacaoBoleto(
    const string& idInfracao,
    const string& cpfUsuario) const {
    
        // Conversão da resposta da API
    
    return api_.consultarSolicitacaoBoleto(idInfracao, cpfUsuario);
}
