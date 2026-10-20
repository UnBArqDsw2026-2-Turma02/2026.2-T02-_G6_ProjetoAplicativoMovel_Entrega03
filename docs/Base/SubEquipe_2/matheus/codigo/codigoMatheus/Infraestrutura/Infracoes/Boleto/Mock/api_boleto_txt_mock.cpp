#include <cstdio>
#include <fstream>
#include <iomanip>
#include <locale>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include "api_boleto_txt_mock.hpp"

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

bool converterBoleto(
    const vector<string>& campos,
    Boleto& boleto,
    string& erro) {
    if (campos.size() != 11) {
        erro = "esperados 11 campos separados por '|'";
        return false;
    }

    if (campos[2] == "NAO_SOLICITADO") {
        boleto.status = StatusBoleto::NAO_SOLICITADO;
    } else if (campos[2] == "PROCESSANDO") {
        boleto.status = StatusBoleto::PROCESSANDO;
    } else if (campos[2] == "PRONTO") {
        boleto.status = StatusBoleto::PRONTO;
    } else if (campos[2] == "ERRO") {
        boleto.status = StatusBoleto::ERRO;
    } else {
        erro = "status deve ser NAO_SOLICITADO, PROCESSANDO, PRONTO ou ERRO";
        return false;
    }

    try {
        size_t caracteresConvertidos = 0;
        boleto.valorOriginal = stod(campos[5], &caracteresConvertidos);
        if (caracteresConvertidos != campos[5].size()) {
            erro = "valorOriginal inválido";
            return false;
        }
        caracteresConvertidos = 0;
        boleto.valorDesconto = stod(campos[6], &caracteresConvertidos);
        if (caracteresConvertidos != campos[6].size()) {
            erro = "valorDesconto inválido";
            return false;
        }
        caracteresConvertidos = 0;
        boleto.valorPagar = stod(campos[7], &caracteresConvertidos);
        if (caracteresConvertidos != campos[7].size()) {
            erro = "valorPagar inválido";
            return false;
        }
        caracteresConvertidos = 0;
        boleto.percentualDesconto =
            stoi(campos[8], &caracteresConvertidos);
        if (caracteresConvertidos != campos[8].size() ||
            boleto.percentualDesconto < 0 ||
            boleto.percentualDesconto > 100) {
            erro = "percentualDesconto inválido";
            return false;
        }
    } catch (const invalid_argument&) {
        erro = "valor numérico inválido";
        return false;
    } catch (const out_of_range&) {
        erro = "valor numérico fora do limite";
        return false;
    }

    boleto.id = campos[0];
    boleto.idInfracao = campos[1];
    boleto.numeroAuto = campos[3];
    boleto.dataVencimento = campos[4];
    boleto.codigoPagamento = campos[9];
    boleto.arquivoPDF = campos[10];
    if (boleto.id.empty() || boleto.idInfracao.empty()) {
        erro = "ID do boleto e ID da infração não podem ficar vazios";
        return false;
    }
    return true;
}
}  // namespace

ApiBoletoTxtMock::ApiBoletoTxtMock(string caminhoArquivo)
    : caminhoArquivo_(caminhoArquivo) {}

// ---------------------------------
// Retorna o boleto associado a idInfracao
ResultadoConsultaBoleto ApiBoletoTxtMock::obterBoleto(const string& idInfracao) const {

    ifstream arquivo(caminhoArquivo_);
    
    if (!arquivo.is_open()) {
        return {false, nullopt, "Não foi possível abrir o arquivo de boletos: " + caminhoArquivo_};
    }

    string linha;
    size_t numeroLinha = 0;
    set<string> ids;
    optional<Boleto> encontrado;

    while (getline(arquivo, linha)) {
        ++numeroLinha;
        const string linhaAparada = aparar(linha);
    
        if (linhaAparada.empty() || linhaAparada.front() == '#') {
            continue;
        }

        Boleto boleto;
        string erro;
    
        if (!converterBoleto(separarCampos(linha), boleto, erro)) {
            return {false, nullopt,
                    "Registro inválido no arquivo de boletos, linha " +
                        to_string(numeroLinha) + ": " + erro + "."};
        }
    
        if (!ids.insert(boleto.id).second) {
            return {false, nullopt,
                    "ID de boleto duplicado na linha " +
                        to_string(numeroLinha) + "."};
        }
    
        if (boleto.idInfracao == idInfracao) {
            if (encontrado) {
                return {false, nullopt,
                        "Há mais de um boleto cadastrado para a infração " +
                            idInfracao + "."};
            }
            encontrado = boleto;
        }
    }
    
    if (arquivo.bad()) {
        return {false, nullopt,
                "Erro ao ler o arquivo de boletos: " + caminhoArquivo_};
    }
    
    return {true, encontrado, {}};
}

// Atualiza o desconto de um boleto ja existente
ResultadoConsultaBoleto ApiBoletoTxtMock::atualizarDesconto(const string& idInfracao, int percentualDesconto) const {
    
    if (percentualDesconto < 0 || percentualDesconto > 100) {
        return {false, nullopt, "Percentual de desconto inválido."};
    }

    const ResultadoConsultaBoleto consulta = obterBoleto(idInfracao);

    if (!consulta.sucesso || !consulta.boleto) {
        return {false, nullopt, 
            consulta.sucesso ? "Não existe boleto para atualizar nesta infração." : consulta.mensagem};
    }

    if (consulta.boleto->percentualDesconto == percentualDesconto) {
        return {false, consulta.boleto,
                "O boleto já possui esse percentual de desconto."};
    }

    ifstream arquivo(caminhoArquivo_);
    if (!arquivo.is_open()) {
        return {false, nullopt,
                "Não foi possível abrir o arquivo de boletos: " +
                    caminhoArquivo_};
    }

    const string caminhoTemporario = caminhoArquivo_ + ".tmp"; // usa arquivo temporario que depois substitui o o original (pra nao apagar algo errado na hora de att)
    ofstream temporario(caminhoTemporario, ios::trunc);

    if (!temporario.is_open()) {
        return {false, nullopt,
                "Não foi possível criar o arquivo temporário de boletos: " +
                    caminhoTemporario};
    }


    // cria o novo objeto do boleto
    Boleto boletoAtualizado             = *consulta.boleto;
    boletoAtualizado.percentualDesconto = percentualDesconto;
    boletoAtualizado.valorDesconto      = boletoAtualizado.valorOriginal * percentualDesconto / 100.0;
    boletoAtualizado.valorPagar         = boletoAtualizado.valorOriginal - boletoAtualizado.valorDesconto;
    boletoAtualizado.codigoPagamento    = "SIM-" + boletoAtualizado.id + "-D" + to_string(percentualDesconto);

    string linha;
    size_t numeroLinha = 0;
    bool atualizouLinha = false;

    while (getline(arquivo, linha)) {
        ++numeroLinha;
        const string linhaAparada = aparar(linha);
        
        // Cabeçalhos, linhas vazias e boletos de outras infrações 
        if (!linhaAparada.empty() && linhaAparada.front() != '#') {

            vector<string> campos = separarCampos(linha);
            if (campos.size() == 11 && campos[1] == idInfracao) {
                // Mais de um registro para a mesma infração
                if (atualizouLinha) {
                    
                    temporario.close();
                    remove(caminhoTemporario.c_str());
                    return {false, nullopt,
                            "Há mais de um boleto para atualizar na linha " +
                                to_string(numeroLinha) + "."};
                }

                // Formata valores e atualiza no registro apenas os campos do desconto
                ostringstream valorDesconto;
                ostringstream valorPagar;
                
                valorDesconto.imbue(locale::classic());
                valorPagar.imbue(locale::classic());

                valorDesconto << fixed << setprecision(2) << boletoAtualizado.valorDesconto;
                valorPagar << fixed << setprecision(2) << boletoAtualizado.valorPagar;
                
                campos[6] = valorDesconto.str();
                campos[7] = valorPagar.str();
                campos[8] = to_string(percentualDesconto);
                campos[9] = boletoAtualizado.codigoPagamento;

                ostringstream linhaAtualizada;
                for (size_t indice = 0; indice < campos.size(); ++indice) {
                    if (indice != 0) {
                        linhaAtualizada << '|';
                    }
                    linhaAtualizada << campos[indice];
                }
                
                // Grava a linha alterada no temporário e marca que o boleto foi localizado
                temporario << linhaAtualizada.str();
                atualizouLinha = true;
            } 
            
            else {
                // Mantém os registros válidos que não correspondem ao boleto procurado
                temporario << linha;
            }
        } else {
            // preserva cabeçalhos e linhas vazias do arquivo original.
            temporario << linha;
        }
        temporario << '\n';
    }

    if (arquivo.bad() || !atualizouLinha) {
        temporario.close();
        remove(caminhoTemporario.c_str());
        return {false, nullopt,
                arquivo.bad()
                    ? "Erro ao ler o arquivo de boletos."
                    : "O boleto não foi encontrado para atualização."};
    }

    arquivo.close();
    temporario.close();
    if (!temporario) {
        remove(caminhoTemporario.c_str());
        return {false, nullopt,
                "Erro ao gravar o arquivo temporário de boletos."};
    }

#ifdef _WIN32
    // No Windows, substitui o arquivo existente e solicita que a gravação seja persistida
    const BOOL substituido = MoveFileExA(
        caminhoTemporario.c_str(), caminhoArquivo_.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
    if (!substituido) {
        remove(caminhoTemporario.c_str());
        return {false, nullopt,
                "Não foi possível substituir o arquivo de boletos."};
    }
#else
    // Em outros sistemas, renomeia o temporário para o caminho do arquivo original (nao testei) [IA]
    if (rename(caminhoTemporario.c_str(), caminhoArquivo_.c_str()) != 0) {
        remove(caminhoTemporario.c_str());
        return {false, nullopt,
                "Não foi possível substituir o arquivo de boletos."};
    }
#endif

    // Retorna o boleto atualizado somente depois que o arquivo foi substituído.
    return {true, boletoAtualizado, {}};
}
