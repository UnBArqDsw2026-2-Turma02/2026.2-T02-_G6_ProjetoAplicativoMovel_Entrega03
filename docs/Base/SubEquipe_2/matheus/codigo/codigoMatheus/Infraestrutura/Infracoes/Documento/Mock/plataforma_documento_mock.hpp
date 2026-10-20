#pragma once

#include <string>
using namespace std;

// [Adapter: adaptee] Representa ações locais sobre documentos.
class PlataformaDocumentoMock {
public:
    bool copiarParaClipboard(const string& codigo) const;
    bool abrirCom(const string& arquivo) const;
    bool compartilharArquivo(const string& arquivo) const;
    bool salvarOuBaixar(const string& arquivo) const;
    bool imprimirArquivo(const string& arquivo) const;
};
