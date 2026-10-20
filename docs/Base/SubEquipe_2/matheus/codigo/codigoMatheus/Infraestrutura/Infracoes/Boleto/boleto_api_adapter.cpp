#include "boleto_api_adapter.hpp"
using namespace std;

BoletoApiAdapter::BoletoApiAdapter(const ApiBoletoTxtMock& api)
    : api_(api) {}

ResultadoConsultaBoleto BoletoApiAdapter::consultarBoleto(const string& idInfracao) const {
    
    // convert

    return api_.obterBoleto(idInfracao);
}

ResultadoConsultaBoleto BoletoApiAdapter::atualizarDesconto(const string& idInfracao, int percentualDesconto) const {

    // convert

    return api_.atualizarDesconto(idInfracao, percentualDesconto);
}
