#pragma once

#include "../../../Aplicacao/Infracoes/Documento/gateway_documento.hpp"
#include "Mock/plataforma_documento_mock.hpp"
using namespace std;

// [Adapter]
class DocumentoPlataformaAdapter final : public GatewayDocumento {
public:
    explicit DocumentoPlataformaAdapter(
        const PlataformaDocumentoMock& plataforma);

    bool copiarCodigo(const string& codigo) const override;
    bool abrirPDF(const string& arquivo) const override;
    bool compartilhar(const string& arquivo) const override;
    bool baixar(const string& arquivo) const override;
    bool imprimir(const string& arquivo) const override;

private:
    const PlataformaDocumentoMock& plataforma_;
};
