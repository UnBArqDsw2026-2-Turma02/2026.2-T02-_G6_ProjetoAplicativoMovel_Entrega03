#include "infracao_facade.hpp"

#include "../../../Infraestrutura/Infracoes/Boleto/Mock/api_boleto_txt_mock.hpp"
#include "../../../Infraestrutura/Infracoes/Boleto/boleto_api_adapter.hpp"
#include "../../../Infraestrutura/Infracoes/Documento/Mock/plataforma_documento_mock.hpp"
#include "../../../Infraestrutura/Infracoes/Documento/documento_plataforma_adapter.hpp"
#include "../../../Infraestrutura/Infracoes/Infracoes/Mock/api_infracoes_txt_mock.hpp"
#include "../../../Infraestrutura/Infracoes/Infracoes/infracoes_api_adapter.hpp"
#include "../../../Infraestrutura/Infracoes/OrgaoAutuador/Mock/api_orgao_autuador_txt_mock.hpp"
#include "../../../Infraestrutura/Infracoes/OrgaoAutuador/orgao_autuador_api_adapter.hpp"

#include <algorithm>
#include <string>
using namespace std;

struct InfracaoFacade::Impl {
    ApiInfracoesTxtMock apiInfracoes;
    InfracoesApiAdapter adapterInfracoes;
    ApiOrgaoAutuadorTxtMock apiOrgaoAutuador;
    OrgaoAutuadorApiAdapter adapterOrgaoAutuador;
    ApiBoletoTxtMock apiBoleto;
    BoletoApiAdapter adapterBoleto;
    PlataformaDocumentoMock plataformaDocumento;
    DocumentoPlataformaAdapter adapterDocumento;

    Impl(
        const string& caminhoInfracoes,
        const string& caminhoBoletos,
        const string& caminhoSolicitacoesBoleto,
        const string& caminhoSolicitacoesDesconto)
        : apiInfracoes(caminhoInfracoes),
          adapterInfracoes(apiInfracoes),
          apiOrgaoAutuador(
              caminhoSolicitacoesBoleto, caminhoSolicitacoesDesconto),
          adapterOrgaoAutuador(apiOrgaoAutuador),
          apiBoleto(caminhoBoletos),
          adapterBoleto(apiBoleto),
          adapterDocumento(plataformaDocumento) {}
};

// Construtor de produção: instancia mocks e adapters usando os caminhos TXT
// recebidos; os gateways apontam para os adapters mantidos pelo Pimpl.
InfracaoFacade::InfracaoFacade(
    const string& caminhoInfracoes,
    const string& caminhoBoletos,
    const string& caminhoSolicitacoesBoleto,
    const string& caminhoSolicitacoesDesconto)
    : impl_(make_unique<Impl>(
          caminhoInfracoes, caminhoBoletos,
          caminhoSolicitacoesBoleto, caminhoSolicitacoesDesconto)),
      gatewayInfracoes_(&impl_->adapterInfracoes),
      gatewayOperacoes_(&impl_->adapterOrgaoAutuador),
      gatewayBoleto_(&impl_->adapterBoleto),
      gatewayDocumento_(&impl_->adapterDocumento) {}

InfracaoFacade::~InfracaoFacade() = default;

ResultadoListaInfracoes InfracaoFacade::consultarInfracoes(
    const string& cpfUsuario) const {
    return gatewayInfracoes_->listarInfracoes(cpfUsuario);
}

ResultadoDetalhesInfracao InfracaoFacade::obterDetalhes(
    const string& idInfracao) const {
    return gatewayInfracoes_->obterDetalhes(idInfracao);
}

vector<int> InfracaoFacade::obterOpcoesDesconto(
    const Infracao& infracao) {
    return beneficios_.obterOpcoes(infracao);
}

ResultadoOperacaoInfracao InfracaoFacade::solicitarDesconto(
    const Usuario& usuario,
    const Infracao& infracao,
    int percentual,
    bool termoReconhecimentoAceito) {
    if (infracao.status != StatusInfracao::A_VENCER) {
        return {false, "Desconto indisponível para infração vencida ou paga."};
    }

    const vector<int> opcoes = beneficios_.obterOpcoes(infracao);
    if (find(opcoes.begin(), opcoes.end(), percentual) == opcoes.end()) {
        return {false, "Esse percentual não está disponível para a infração."};
    }
    if (percentual == 40 && !usuario.aderiuSNE) {
        return {false, "O usuário precisa aderir ao SNE para usar 40%."};
    }
    if (percentual == 40 && !termoReconhecimentoAceito) {
        return {false, "O desconto de 40% exige aceite do termo."};
    }

    SolicitacaoDesconto solicitacao{
        infracao.id, usuario.cpf, percentual,
        percentual == 40 && termoReconhecimentoAceito};
    return gatewayOperacoes_->enviarSolicitacaoDesconto(solicitacao);
}

ResultadoOperacaoInfracao InfracaoFacade::solicitarBoleto(
    const Usuario& usuario,
    const Infracao& infracao) const {
    if (infracao.status == StatusInfracao::PAGA) {
        return {false, "Não é possível solicitar boleto para infração paga."};
    }

    const ResultadoConsultaBoleto consulta =
        consultarSituacaoBoleto(usuario, infracao.id);
    if (!consulta.sucesso) {
        return {false, "Não foi possível verificar se já existe um boleto: " +
                           consulta.mensagem};
    }
    if (consulta.boleto) {
        return {true, "Já existe um boleto para esta infração. "
                      "Nenhum novo pedido foi enviado."};
    }
    if (consulta.emAnalise) {
        return {true, "O pedido de boleto já está em análise. "
                      "Ele não foi reenviado."};
    }

    return gatewayOperacoes_->solicitarBoleto(infracao.id, usuario.cpf);
}

ResultadoOperacaoInfracao InfracaoFacade::atualizarBoletoComDesconto(
    const Usuario& usuario,
    const Infracao& infracao,
    int percentualDesconto,
    bool termoReconhecimentoAceito) {
    if (infracao.status != StatusInfracao::A_VENCER) {
        return {false,
                "Não é possível alterar o desconto de uma infração vencida ou paga."};
    }

    const vector<int> opcoes = beneficios_.obterOpcoes(infracao);
    if (find(opcoes.begin(), opcoes.end(), percentualDesconto) ==
        opcoes.end()) {
        return {false, "Esse percentual não está disponível para a infração."};
    }
    if (percentualDesconto == 40 && !usuario.aderiuSNE) {
        return {false, "O usuário precisa aderir ao SNE para usar 40%."};
    }
    if (percentualDesconto == 40 && !termoReconhecimentoAceito) {
        return {false, "O desconto de 40% exige aceite do termo."};
    }

    const ResultadoConsultaBoleto existente =
        gatewayBoleto_->consultarBoleto(infracao.id);
    if (!existente.sucesso) {
        return {false, "Não foi possível consultar o boleto existente: " +
                           existente.mensagem};
    }
    if (!existente.boleto) {
        return {false, "Não existe boleto para atualizar nesta infração."};
    }
    if (existente.boleto->percentualDesconto == percentualDesconto) {
        return {false, "O boleto já está emitido com esse percentual de "
                       "desconto. Escolha um valor diferente."};
    }

    const ResultadoConsultaBoleto atualizado =
        gatewayBoleto_->atualizarDesconto(infracao.id, percentualDesconto);
    if (!atualizado.sucesso) {
        return {false, atualizado.mensagem};
    }

    return {true, "Boleto existente atualizado para desconto de " +
                      to_string(percentualDesconto) + "%."};
}

ResultadoConsultaBoleto InfracaoFacade::consultarBoleto(
    const string& idInfracao) const {
    return gatewayBoleto_->consultarBoleto(idInfracao);
}

ResultadoConsultaBoleto InfracaoFacade::consultarSituacaoBoleto(
    const Usuario& usuario,
    const string& idInfracao) const {
    ResultadoConsultaBoleto consulta =
        gatewayBoleto_->consultarBoleto(idInfracao);
    if (!consulta.sucesso || consulta.boleto) {
        return consulta;
    }

    const ResultadoConsultaSolicitacaoBoleto pedido =
        gatewayOperacoes_->consultarSolicitacaoBoleto(idInfracao, usuario.cpf);
    if (!pedido.sucesso) {
        return {false, nullopt, pedido.mensagem};
    }
    if (pedido.existe) {
        consulta.emAnalise = true;
        consulta.mensagem = "O pedido foi registrado e está em análise.";
    }
    return consulta;
}

bool InfracaoFacade::copiarCodigoPagamento(const Boleto& boleto) const {
    return boleto.status == StatusBoleto::PRONTO &&
           !boleto.codigoPagamento.empty() &&
           gatewayDocumento_->copiarCodigo(boleto.codigoPagamento);
}

bool InfracaoFacade::exibirPDFBoleto(const Boleto& boleto) const {
    return boleto.status == StatusBoleto::PRONTO &&
           !boleto.arquivoPDF.empty() &&
           gatewayDocumento_->abrirPDF(boleto.arquivoPDF);
}

bool InfracaoFacade::exibirPDFInfracao(const Infracao& infracao) const {
    return !infracao.numeroAuto.empty() &&
           gatewayDocumento_->abrirPDF(infracao.numeroAuto + ".pdf");
}

bool InfracaoFacade::compartilharDocumento(
    const string& arquivo) const {
    return gatewayDocumento_->compartilhar(arquivo);
}

bool InfracaoFacade::baixarDocumento(const string& arquivo) const {
    return gatewayDocumento_->baixar(arquivo);
}

bool InfracaoFacade::imprimirDocumento(const string& arquivo) const {
    return gatewayDocumento_->imprimir(arquivo);
}
