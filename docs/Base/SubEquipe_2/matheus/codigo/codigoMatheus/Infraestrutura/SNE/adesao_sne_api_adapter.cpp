#include "adesao_sne_api_adapter.hpp"
using namespace std;

AdesaoSNEApiAdapter::AdesaoSNEApiAdapter(const ApiSNEMock& api)
    : api_(api) {}

ResultadoGatewayAdesaoSNE AdesaoSNEApiAdapter::consultarAdesao(
    const string& cpfUsuario) const {

    // Converte o resultado próprio do mock para o resultado definido pelo Gateway.
    const ResultadoConsultaAdesaoSNE resultado = api_.consultarAdesao(cpfUsuario);
    
    return {resultado.sucesso, resultado.aderiu, resultado.mensagem};
}

ResultadoGatewayAdesaoSNE AdesaoSNEApiAdapter::solicitarAdesao(
    const RequisicaoAdesaoSNE& requisicao) const {
    
    // Ajuste aqui a conversão da requisição se o formato da API SNE mudar.
    const ResultadoConsultaAdesaoSNE resultado = api_.efetivarAdesao(requisicao);
    
    // Traduz a resposta da API para o tipo esperado pela camada de aplicação.
    return {resultado.sucesso, resultado.aderiu, resultado.mensagem};
}
