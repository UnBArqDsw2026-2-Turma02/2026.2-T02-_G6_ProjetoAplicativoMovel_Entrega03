#pragma once

#include "estrategia_documento_sne.hpp"
#include "../../Dominios/usuario.hpp"
#include "gateway_adesao_sne.hpp"

#include <optional>
#include <memory>
#include <string>
using namespace std;

struct ResultadoConsultaFacadeSNE {
    bool sucesso = false;
    bool aderiu = false;
    string mensagem;
};

struct ResultadoAdesaoFacadeSNE {
    bool sucesso = false;
    string mensagem;
};

// [Facade]
class SNEFacade {
public:
    explicit SNEFacade(const string& caminhoAdesoes);
    explicit SNEFacade(const GatewayAdesaoSNE& gateway);
    ~SNEFacade();

    ResultadoConsultaFacadeSNE consultaAdesao(Usuario& usuario) const;
    ResultadoAdesaoFacadeSNE aderir(Usuario& usuario, TipoDocumentoSNE tipoDocumento, const DadosAdesaoSNE& dados, bool termosAceitos);

    void definirEstrategiaDocumento(unique_ptr<EstrategiaDocumentoSNE> estrategia);
    optional<RequisicaoAdesaoSNE> validarDocumento(const Usuario& usuario, const DadosAdesaoSNE& dados) const;

private:
    struct Impl;
    unique_ptr<Impl> impl_;
    const GatewayAdesaoSNE* gateway_;
    ContextoValidacaoDocumentoSNE contextoValidacao_;
};
