#include "plataforma_documento_mock.hpp"
using namespace std;

bool PlataformaDocumentoMock::copiarParaClipboard(const string& codigo) const {
    return !codigo.empty();
}

bool PlataformaDocumentoMock::abrirCom(const string& arquivo) const {
    return !arquivo.empty();
}

bool PlataformaDocumentoMock::compartilharArquivo(const string& arquivo) const {
    return !arquivo.empty();
}

bool PlataformaDocumentoMock::salvarOuBaixar(const string& arquivo) const {
    return !arquivo.empty();
}

bool PlataformaDocumentoMock::imprimirArquivo(const string& arquivo) const {
    return !arquivo.empty();
}
