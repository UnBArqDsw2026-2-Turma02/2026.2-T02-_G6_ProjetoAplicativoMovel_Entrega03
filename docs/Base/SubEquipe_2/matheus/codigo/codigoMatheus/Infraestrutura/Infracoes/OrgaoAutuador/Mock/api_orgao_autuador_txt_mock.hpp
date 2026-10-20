#pragma once

#include <Aplicacao/Infracoes/OrgaoAutuador/gateway_operacoes_infracao.hpp>

#include <string>
using namespace std;

// [Adapter: adaptee] Registra solicitações encaminhadas ao órgão em TXT.
class ApiOrgaoAutuadorTxtMock {
public:
    ApiOrgaoAutuadorTxtMock(string caminhoBoletos,string caminhoDescontos);

    ResultadoOperacaoInfracao registrarSolicitacaoDesconto(const SolicitacaoDesconto& solicitacao) const;
    ResultadoOperacaoInfracao registrarSolicitacaoBoleto(const string& idInfracao,const string& cpfUsuario) const;
    ResultadoConsultaSolicitacaoBoleto consultarSolicitacaoBoleto(const string& idInfracao,const string& cpfUsuario) const;

private:
    ResultadoOperacaoInfracao registrarLinha(const string& caminhoArquivo,const string& linha) const;

    string caminhoBoletos_;
    string caminhoDescontos_;
};
