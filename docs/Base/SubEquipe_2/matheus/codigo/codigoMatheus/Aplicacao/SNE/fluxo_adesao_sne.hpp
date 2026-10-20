#pragma once

#include "sne_facade.hpp"

#include <iosfwd>
using namespace std;

// Oferece a adesão e coleta os dados necessários para a Facade do SNE.
bool oferecerAdesaoSNE(Usuario& usuario, SNEFacade& facade, istream& entrada, ostream& saida, ostream& erros);
