#pragma once

#include <string>
using namespace std;

// [Adapter: interface]
// Target: operações de documento oferecidas pela plataforma/lib ao sistema
class GatewayDocumento {
public:
    virtual ~GatewayDocumento() = default;

    virtual bool copiarCodigo(const string& codigo) const = 0;
    virtual bool abrirPDF(const string& arquivo) const = 0;
    virtual bool compartilhar(const string& arquivo) const = 0;
    virtual bool baixar(const string& arquivo) const = 0;
    virtual bool imprimir(const string& arquivo) const = 0;
};
