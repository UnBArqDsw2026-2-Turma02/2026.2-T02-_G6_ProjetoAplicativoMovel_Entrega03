#include "sne_facade.hpp"

#include "../../Infraestrutura/SNE/Mock/api_sne_mock.hpp"
#include "../../Infraestrutura/SNE/adesao_sne_api_adapter.hpp"
#include "../../Infraestrutura/SNE/repositorio_adesao_sne_txt.hpp"

using namespace std;

struct SNEFacade::Impl {

    // facade cria o repositório TXT, o mock da API e o adapter que implementa o gateway
    RepositorioAdesaoSNETxt repositorio;
    ApiSNEMock api;
    AdesaoSNEApiAdapter adapter;

    explicit Impl(const string& caminhoAdesoes)
        : repositorio(caminhoAdesoes),
          api(repositorio),
          adapter(api) {}
};

// instancia as dependências concretas a partir do caminho do arquivo
// gateway aponta para o adapter do Pimpl.
SNEFacade::SNEFacade(const string& caminhoAdesoes)
    : impl_(make_unique<Impl>(caminhoAdesoes)),
      gateway_(&impl_->adapter) {}

SNEFacade::~SNEFacade() = default;

// -----------------------------------------------------------------

ResultadoConsultaFacadeSNE SNEFacade::consultaAdesao(Usuario& usuario) const {
    const ResultadoGatewayAdesaoSNE resultado = gateway_->consultarAdesao(usuario.cpf);
    
    if (!resultado.sucesso) {
        return {false, usuario.aderiuSNE, resultado.mensagem};
    }

    usuario.aderiuSNE = resultado.aderiu;
    return {true, resultado.aderiu, {}};
}

// -----------------------------------------------------------------

ResultadoAdesaoFacadeSNE SNEFacade::aderir( Usuario& usuario, TipoDocumentoSNE tipoDocumento, const DadosAdesaoSNE& dados, bool termosAceitos) {
  
    if (!termosAceitos) {
        return {false, "É necessário aceitar os termos de adesão ao SNE."};
    }

    if (usuario.aderiuSNE) {
        return {true, "O usuário já possui adesão ao SNE."};
    }

    if (tipoDocumento == TipoDocumentoSNE::CNH) {
        definirEstrategiaDocumento(make_unique<EstrategiaDocumentoCNH>());
    } 
    
    else {
        definirEstrategiaDocumento(make_unique<EstrategiaDocumentoCRLV>());
    }

    const optional<RequisicaoAdesaoSNE> requisicao = validarDocumento(usuario, dados);
    
    if (!requisicao) {
        return {false, "Os dados estão incompletos ou não correspondem aos documentos cadastrados para o usuário."};
    }

    const ResultadoGatewayAdesaoSNE resultado =
        gateway_->solicitarAdesao(*requisicao);
    if (!resultado.sucesso) {
        return {false, resultado.mensagem};
    }
    
    if (!resultado.aderiu) {
        return {false, "A API SNE não confirmou a adesão."};
    }

    usuario.aderiuSNE = true;
    if (tipoDocumento == TipoDocumentoSNE::CNH) {
        usuario.numeroCNH = dados.numeroCNH;
        usuario.codigoSegurancaCNH = dados.codigoSegurancaCNH;
    } 
    
    else {
        usuario.placa = dados.placa;
        usuario.renavam = dados.renavam;
        usuario.numeroSegurancaCRV = dados.numeroSegurancaCRV;
    }
    
    return {true, "Adesão ao SNE concluída."};
}

// -----------------------------------------------------------------

void SNEFacade::definirEstrategiaDocumento(unique_ptr<EstrategiaDocumentoSNE> estrategia) {
    contextoValidacao_.definirEstrategia(estrategia);
}

// -----------------------------------------------------------------

optional<RequisicaoAdesaoSNE> SNEFacade::validarDocumento(const Usuario& usuario, const DadosAdesaoSNE& dados) const {
    return contextoValidacao_.validarEPreparar(usuario, dados);
}
