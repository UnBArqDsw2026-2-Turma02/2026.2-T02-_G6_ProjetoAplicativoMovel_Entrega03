#include "documento_plataforma_adapter.hpp"
using namespace std;

DocumentoPlataformaAdapter::DocumentoPlataformaAdapter(
    const PlataformaDocumentoMock& plataforma)
    : plataforma_(plataforma) {}

bool DocumentoPlataformaAdapter::copiarCodigo(
    const string& codigo) const {
    
        // Conversão
    
    return plataforma_.copiarParaClipboard(codigo);
}

bool DocumentoPlataformaAdapter::abrirPDF(
    const string& arquivo) const {
    
        // Conversão
    
    return plataforma_.abrirCom(arquivo);
}

bool DocumentoPlataformaAdapter::compartilhar(
    const string& arquivo) const {
    
        // Conversão
    
    return plataforma_.compartilharArquivo(arquivo);
}

bool DocumentoPlataformaAdapter::baixar(
    const string& arquivo) const {
    
        // Conversão
    
    return plataforma_.salvarOuBaixar(arquivo);
}

bool DocumentoPlataformaAdapter::imprimir(
    const string& arquivo) const {
    
        // Conversão
    
    return plataforma_.imprimirArquivo(arquivo);
}
