#pragma once

#include "../../../../Aplicacao/Infracoes/Boleto/gateway_boleto.hpp"

#include <string>
using namespace std;

// [Adapter: adaptee] Consulta registros de boletos no TXT.
class ApiBoletoTxtMock {
public:
    explicit ApiBoletoTxtMock(string caminhoArquivo);

    ResultadoConsultaBoleto obterBoleto(const string& idInfracao) const;
    ResultadoConsultaBoleto atualizarDesconto(const string& idInfracao, int percentualDesconto) const;

private:
    string caminhoArquivo_;
};
