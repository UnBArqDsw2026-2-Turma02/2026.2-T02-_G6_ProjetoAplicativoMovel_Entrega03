#include "api_orgao_autuador_txt_mock.hpp"

#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

namespace {
// limpeza
string aparar(const string& texto) {
    const string espacos = " \t\r\n";
    const size_t inicio = texto.find_first_not_of(espacos);
    if (inicio == string::npos) {
        return {};
    }

    const size_t fim = texto.find_last_not_of(espacos);
    return texto.substr(inicio, fim - inicio + 1);
}

// separador '|'
vector<string> separarCampos(const string& linha) {
    vector<string> campos;
    stringstream entrada(linha);
    string campo;
    while (getline(entrada, campo, '|')) {
        campos.push_back(aparar(campo));
    }
    if (!linha.empty() && linha.back() == '|') {
        campos.emplace_back();
    }
    return campos;
}

bool campoSeguro(const string& valor) {
    return valor.find_first_of("|\r\n") == string::npos;
}
}  // namespace

ApiOrgaoAutuadorTxtMock::ApiOrgaoAutuadorTxtMock(
    string caminhoBoletos,
    string caminhoDescontos)
    : caminhoBoletos_(caminhoBoletos),
      caminhoDescontos_(caminhoDescontos) {}

ResultadoOperacaoInfracao ApiOrgaoAutuadorTxtMock::registrarLinha(const string& caminhoArquivo, const string& linha) const {
    
    ofstream arquivo(caminhoArquivo, ios::app);
    if (!arquivo.is_open()) {
        return {false,
                "Não foi possível abrir o arquivo de operações do órgão: " +
                    caminhoArquivo};
    }
    
    // escreve no arquivo de boleto ou desconto
    arquivo << linha << '\n';

    if (!arquivo.good()) {
        return {false, "Falha ao gravar no arquivo de operações do órgão."};
    }
    
    return {true, "Solicitação registrada no mock TXT do órgão autuador."};
}

// ------------------------------------
// DESCONTO
ResultadoOperacaoInfracao ApiOrgaoAutuadorTxtMock::registrarSolicitacaoDesconto(const SolicitacaoDesconto& solicitacao) const {
 
    // verificação da entrada para desconto
    if (!campoSeguro(solicitacao.cpfUsuario) || !campoSeguro(solicitacao.idInfracao) || (solicitacao.percentual != 20 && solicitacao.percentual != 40)) {
        return {false, "Dados inválidos para registrar pedido de desconto."};
    }

    // escreve no arquivo de desconto
    return registrarLinha(
        caminhoDescontos_,
        "DESCONTO|" + solicitacao.cpfUsuario + "|" + solicitacao.idInfracao +
        "|" + to_string(solicitacao.percentual) + "|" +
        (solicitacao.termoReconhecimentoAceito ? "true" : "false"));
}

// -----------------------------------------
// Boleto
ResultadoOperacaoInfracao ApiOrgaoAutuadorTxtMock::registrarSolicitacaoBoleto(const string& idInfracao, const string& cpfUsuario) const {
    
    // ver. entrada
    if (!campoSeguro(idInfracao) || !campoSeguro(cpfUsuario)) {
        return {false, "Dados inválidos para registrar pedido de boleto."};
    }

    // procura se o boelto ja existe
    const ResultadoConsultaSolicitacaoBoleto consulta = consultarSolicitacaoBoleto(idInfracao, cpfUsuario); // retorna [sucesso, existe, mensagem]
    
    if (!consulta.sucesso) {
        return {false, consulta.mensagem};
    }
    
    if (consulta.existe) {
        return {true, "O pedido de boleto já foi registrado e está em análise. "
                      "Ele não foi reenviado."};
    }

    // escreve no arq. de boleto
    return registrarLinha(caminhoBoletos_,"BOLETO|" + cpfUsuario + "|" + idInfracao);
}


ResultadoConsultaSolicitacaoBoleto ApiOrgaoAutuadorTxtMock::consultarSolicitacaoBoleto(const string& idInfracao, const string& cpfUsuario) const {
    
    //ver.
    if (!campoSeguro(idInfracao) || !campoSeguro(cpfUsuario)) {
        return {false, false, "Dados inválidos para consultar pedido de boleto."};
    }

    ifstream operacoes(caminhoBoletos_);

    if (!operacoes.is_open()) {
        return {false, false, "Não foi possível abrir o arquivo de operações do órgão: " + caminhoBoletos_};
    }

    const vector<string> camposPedido = separarCampos("BOLETO|" + cpfUsuario + "|" + idInfracao);

    string linha;
    size_t numeroLinha = 0;
    
    while (getline(operacoes, linha)) {
        ++numeroLinha;
        const string linhaAparada = aparar(linha);
        if (linhaAparada.empty() || linhaAparada.front() == '#') {
            continue;
        }

        const vector<string> campos = separarCampos(linha);
        if (campos.empty() || campos.front() != "BOLETO") {
            continue;
        }
        if (campos.size() != 3 || campos[1].empty() || campos[2].empty()) {
            return {false, false, "Registro de boleto inválido no arquivo de operações do órgão, linha " + to_string(numeroLinha) + "."};
        }
        if (campos == camposPedido) {
            return {true, true, {}}; // concluído: existe
        }
    }

    if (operacoes.bad()) {
        return {false, false, "Erro ao ler o arquivo de operações do órgão: " + caminhoBoletos_};
    }
    return {true, false, {}}; // concluido: não existe
}
