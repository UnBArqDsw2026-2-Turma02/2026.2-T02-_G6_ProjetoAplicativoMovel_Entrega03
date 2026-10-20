#include "infracoes_api_adapter.hpp"
using namespace std;

InfracoesApiAdapter::InfracoesApiAdapter(const ApiInfracoesTxtMock& api)
    : api_(api) {}

ResultadoListaInfracoes InfracoesApiAdapter::listarInfracoes(const string& cpfUsuario) const {
    
        // Conversão
    
        return api_.consultarInfracoes(cpfUsuario);
}

ResultadoDetalhesInfracao InfracoesApiAdapter::obterDetalhes(const string& idInfracao) const {
    
        // Conversão
    
    return api_.consultarDetalhes(idInfracao);
}
