#pragma once

#include <string>
using namespace std;
enum class StatusBoleto {
    NAO_SOLICITADO,
    PROCESSANDO,
    PRONTO,
    ERRO
};

struct Boleto {
    string id;
    string idInfracao;
    StatusBoleto status = StatusBoleto::NAO_SOLICITADO;
    string numeroAuto;
    string dataVencimento;
    double valorOriginal = 0.0;
    double valorDesconto = 0.0;
    double valorPagar = 0.0;
    int percentualDesconto = 0;
    string codigoPagamento;
    string arquivoPDF;
};
