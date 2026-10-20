#pragma once

#include "beneficio_service.hpp"
#include "../Boleto/gateway_boleto.hpp"
#include "../Documento/gateway_documento.hpp"
#include "gateway_infracoes.hpp"
#include "../OrgaoAutuador/gateway_operacoes_infracao.hpp"
#include "../../../Dominios/usuario.hpp"

#include <memory>
#include <string>
#include <vector>
using namespace std;

// [Facade] Reúne as operações do fluxo de infrações
class InfracaoFacade {
public:
    InfracaoFacade(
        const string& caminhoInfracoes,
        const string& caminhoBoletos,
        const string& caminhoSolicitacoesBoleto,
        const string& caminhoSolicitacoesDesconto);
    ~InfracaoFacade();

    ResultadoListaInfracoes consultarInfracoes(const string& cpfUsuario) const;
    ResultadoDetalhesInfracao obterDetalhes(const string& idInfracao) const;
    vector<int> obterOpcoesDesconto(const Infracao& infracao);
    ResultadoOperacaoInfracao solicitarDesconto(const Usuario& usuario, const Infracao& infracao, int percentual, bool termoReconhecimentoAceito);
    ResultadoOperacaoInfracao solicitarBoleto(const Usuario& usuario, const Infracao& infracao) const;
    ResultadoOperacaoInfracao atualizarBoletoComDesconto(const Usuario& usuario, const Infracao& infracao, int percentualDesconto, bool termoReconhecimentoAceito);
    ResultadoConsultaBoleto consultarSituacaoBoleto(const Usuario& usuario, const string& idInfracao) const;
    ResultadoConsultaBoleto consultarBoleto(const string& idInfracao) const;

    bool copiarCodigoPagamento(const Boleto& boleto) const;
    bool exibirPDFBoleto(const Boleto& boleto) const;
    bool exibirPDFInfracao(const Infracao& infracao) const;
    bool compartilharDocumento(const string& arquivo) const;
    bool baixarDocumento(const string& arquivo) const;
    bool imprimirDocumento(const string& arquivo) const;

private:
    struct Impl;
    unique_ptr<Impl> impl_;
    const GatewayInfracoes* gatewayInfracoes_;
    const GatewayOperacoesInfracao* gatewayOperacoes_;
    const GatewayBoleto* gatewayBoleto_;
    const GatewayDocumento* gatewayDocumento_;
    BeneficioService beneficios_;
};
