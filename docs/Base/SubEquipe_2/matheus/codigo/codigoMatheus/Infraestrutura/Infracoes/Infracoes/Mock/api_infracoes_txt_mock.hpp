#pragma once

#include "../../../../Aplicacao/Infracoes/Infracoes/gateway_infracoes.hpp"

#include <string>
using namespace std;

// [Adapter: adaptee] API mockada que pesquisa no TXT.
class ApiInfracoesTxtMock {
public:
    explicit ApiInfracoesTxtMock(string caminhoArquivo);

    ResultadoListaInfracoes consultarInfracoes(const string& cpfUsuario) const;
    ResultadoDetalhesInfracao consultarDetalhes(const string& idInfracao) const;

private:
    string caminhoArquivo_;
};
