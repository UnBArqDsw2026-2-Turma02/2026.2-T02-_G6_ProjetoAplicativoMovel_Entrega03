#pragma once

#include <string>
using namespace std;

struct ResultadoConsultaAdesaoSNE {
    bool sucesso = false;
    bool aderiu = false;
    string mensagem;
};

// Repositório mock: cada linha do arquivo contém o CPF de um usuário aderente.
class RepositorioAdesaoSNETxt {
public:
    explicit RepositorioAdesaoSNETxt(string caminhoArquivo);

    ResultadoConsultaAdesaoSNE consultarPorCPF(const string& cpf) const;
    ResultadoConsultaAdesaoSNE registrarAdesao(const string& cpf) const;

private:
    string caminhoArquivo_;
};
