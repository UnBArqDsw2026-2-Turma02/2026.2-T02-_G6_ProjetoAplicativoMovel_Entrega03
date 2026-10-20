#pragma once

#include <string>

using namespace std;

struct Usuario {
    Usuario() = default;

    Usuario(string cpfUsuario, string nomeUsuario, bool aderiuSNE = false)
        : cpf(cpfUsuario),
          nome(nomeUsuario),
          aderiuSNE(aderiuSNE) {}

    string cpf;
    string nome;
    bool aderiuSNE = false;
    string numeroCNH;
    string codigoSegurancaCNH;
    string placa;
    string renavam;
    string numeroSegurancaCRV;
};
