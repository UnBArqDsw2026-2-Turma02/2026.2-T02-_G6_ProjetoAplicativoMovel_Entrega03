#pragma once

#include <string>
using namespace std;

enum class StatusInfracao {
    VENCIDA,
    A_VENCER,
    PAGA
};

struct OrgaoAutuador {
    string id;
    string nome;
    bool aderiuSNE = false;
    bool ofereceDesconto20CDT = false;
};

struct Infracao {
    string id;
    StatusInfracao status = StatusInfracao::A_VENCER;
    string motivo;
    string data;
    string horario;
    double valorOriginal = 0.0;
    string placa;
    string renainf;
    string numeroAuto;
    string codigoInfracao;
    string local;
    string dataHoraCometimento;
    string dataNotificacaoAutuacao;
    string prazoDefesa;
    string dataNotificacaoPenalidade;
    string prazoRecurso;
    string vencimentoDesconto;
    int diasAteVencimento = 0;
    OrgaoAutuador orgao;
};
