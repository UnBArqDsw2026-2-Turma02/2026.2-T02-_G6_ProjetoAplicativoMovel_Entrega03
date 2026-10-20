#include "api_infracoes_txt_mock.hpp"

#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>
using namespace std;

namespace {

string aparar(const string& texto) {
    const string espacos = " \t\r\n";
    const size_t inicio = texto.find_first_not_of(espacos);
    if (inicio == string::npos) {
        return {};
    }

    const size_t fim = texto.find_last_not_of(espacos);
    return texto.substr(inicio, fim - inicio + 1);
}

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

bool converterBooleano(const string& valor, bool& resultado) {
    if (valor == "true") {
        resultado = true;
        return true;
    }
    if (valor == "false") {
        resultado = false;
        return true;
    }
    return false;
}

bool converterInfracao(
    const vector<string>& campos,
    Infracao& infracao,
    string& erro) {
    if (campos.size() != 23) {
        erro = "esperados 23 campos separados por '|'";
        return false;
    }

    if (campos[2] == "VENCIDA") {
        infracao.status = StatusInfracao::VENCIDA;
    } else if (campos[2] == "A_VENCER") {
        infracao.status = StatusInfracao::A_VENCER;
    } else if (campos[2] == "PAGA") {
        infracao.status = StatusInfracao::PAGA;
    } else {
        erro = "status deve ser VENCIDA, A_VENCER ou PAGA";
        return false;
    }

    bool aderiuSNE = false;
    bool ofereceDesconto20CDT = false;
    if (!converterBooleano(campos[21], aderiuSNE) ||
        !converterBooleano(campos[22], ofereceDesconto20CDT)) {
        erro = "os campos de política do órgão devem ser true ou false";
        return false;
    }

    try {
        size_t caracteresConvertidos = 0;
        infracao.valorOriginal = stod(campos[6], &caracteresConvertidos);
        if (caracteresConvertidos != campos[6].size()) {
            erro = "valorOriginal não é um número válido";
            return false;
        }
        caracteresConvertidos = 0;
        infracao.diasAteVencimento =
            stoi(campos[18], &caracteresConvertidos);
        if (caracteresConvertidos != campos[18].size()) {
            erro = "diasAteVencimento não é um número inteiro válido";
            return false;
        }
    } catch (const invalid_argument&) {
        erro = "valor numérico inválido";
        return false;
    } catch (const out_of_range&) {
        erro = "valor numérico fora do limite";
        return false;
    }

    infracao.id = campos[1];
    infracao.motivo = campos[3];
    infracao.data = campos[4];
    infracao.horario = campos[5];
    infracao.placa = campos[7];
    infracao.renainf = campos[8];
    infracao.numeroAuto = campos[9];
    infracao.codigoInfracao = campos[10];
    infracao.local = campos[11];
    infracao.dataHoraCometimento = campos[12];
    infracao.dataNotificacaoAutuacao = campos[13];
    infracao.prazoDefesa = campos[14];
    infracao.dataNotificacaoPenalidade = campos[15];
    infracao.prazoRecurso = campos[16];
    infracao.vencimentoDesconto = campos[17];
    infracao.orgao.id = campos[19];
    infracao.orgao.nome = campos[20];
    infracao.orgao.aderiuSNE = aderiuSNE;
    infracao.orgao.ofereceDesconto20CDT = ofereceDesconto20CDT;

    if (campos[0].empty() || infracao.id.empty() ||
        infracao.motivo.empty() || infracao.placa.empty()) {
        erro = "CPF, ID, motivo e placa não podem ficar vazios";
        return false;
    }
    return true;
}

struct ResultadoLeitura {
    bool sucesso = false;
    vector<pair<string, Infracao>> registros;
    string mensagem;
};

ResultadoLeitura lerArquivo(const string& caminhoArquivo) {
    ifstream arquivo(caminhoArquivo);
    if (!arquivo.is_open()) {
        return {false, {},
                "Não foi possível abrir o arquivo de infrações: " +
                    caminhoArquivo};
    }

    ResultadoLeitura resultado;
    resultado.sucesso = true;
    set<string> ids;
    string linha;
    size_t numeroLinha = 0;
    while (getline(arquivo, linha)) {
        ++numeroLinha;
        const string linhaAparada = aparar(linha);
        if (linhaAparada.empty() || linhaAparada.front() == '#') {
            continue;
        }

        const vector<string> campos = separarCampos(linha);
        Infracao infracao;
        string erro;
        if (!converterInfracao(campos, infracao, erro)) {
            return {false, {},
                    "Registro inválido no arquivo de infrações, linha " +
                        to_string(numeroLinha) + ": " + erro + "."};
        }
        if (!ids.insert(infracao.id).second) {
            return {false, {},
                    "ID de infração duplicado na linha " +
                        to_string(numeroLinha) + "."};
        }
        resultado.registros.emplace_back(campos[0], infracao);
    }

    if (arquivo.bad()) {
        return {false, {}, "Erro ao ler o arquivo de infrações: " +
                               caminhoArquivo};
    }
    return resultado;
}
}  // namespace

ApiInfracoesTxtMock::ApiInfracoesTxtMock(string caminhoArquivo)
    : caminhoArquivo_(caminhoArquivo) {}

//------------------------------------------------
// CONSULTAR INFRAçOES

ResultadoListaInfracoes ApiInfracoesTxtMock::consultarInfracoes(const string& cpfUsuario) const {
    
    const ResultadoLeitura leitura = lerArquivo(caminhoArquivo_);
    
    if (!leitura.sucesso) {
        return {false, {}, leitura.mensagem};
    }

    ResultadoListaInfracoes resultado;
    resultado.sucesso = true;
    for (const auto& [cpf, infracao] : leitura.registros) {
        if (cpf == cpfUsuario) {
            resultado.infracoes.push_back(infracao);
        }
    }
    return resultado;
}

// ------------------------------------------
// Detalhes
ResultadoDetalhesInfracao ApiInfracoesTxtMock::consultarDetalhes(const string& idInfracao) const {
    const ResultadoLeitura leitura = lerArquivo(caminhoArquivo_);
    
    if (!leitura.sucesso) {
        return {false, nullopt, leitura.mensagem};
    }

    for (const auto& registro : leitura.registros) {
        if (registro.second.id == idInfracao) {
            return {true, registro.second, {}};
        }
    }
    
    return {true, nullopt, {}};
}
