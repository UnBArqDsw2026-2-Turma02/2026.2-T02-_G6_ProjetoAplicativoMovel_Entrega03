#pragma once

#include "infracao_facade.hpp"
#include "../../SNE/sne_facade.hpp"

#include <iosfwd>
using namespace std;

bool executarFluxoInfracoes(
    Usuario& usuario,
    InfracaoFacade& facade,
    SNEFacade& sneFacade,
    istream& entrada,
    ostream& saida,
    ostream& erros);
