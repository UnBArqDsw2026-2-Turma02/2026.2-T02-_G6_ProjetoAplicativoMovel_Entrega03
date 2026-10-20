#include "repositorio_adesao_sne_txt.hpp"

#include <fstream>
#include <set>
using namespace std;

namespace {

// "limpeza" da string
string aparar(const string& texto) {
    const string espacos = " \t\r\n";
    const size_t inicio = texto.find_first_not_of(espacos);
    if (inicio == string::npos) {
        return {};
    }
    const size_t fim = texto.find_last_not_of(espacos);
    return texto.substr(inicio, fim - inicio + 1);
}
}  // namespace

RepositorioAdesaoSNETxt::RepositorioAdesaoSNETxt(
    string caminhoArquivo)
    : caminhoArquivo_(caminhoArquivo) {}

ResultadoConsultaAdesaoSNE RepositorioAdesaoSNETxt::consultarPorCPF(const string& cpf) const {
    
    ifstream arquivo(caminhoArquivo_);
    if (!arquivo.is_open()) {
        return {false, false,
                "Não foi possível abrir o arquivo de adesões SNE: " +
                    caminhoArquivo_};
    }

    set<string> cpfs;
    string linha;
    size_t numeroLinha = 0;

    while (getline(arquivo, linha)) {
        ++numeroLinha;
        const string cpfRegistrado = aparar(linha);
        
        if (cpfRegistrado.empty() || cpfRegistrado.front() == '#') { // comentario
            continue;
        }
        
        if (cpfRegistrado.find('|') != string::npos) {
            return {false, false,
                    "Registro inválido no arquivo de adesões SNE, linha " +
                        to_string(numeroLinha) + "."};
        }
        if (!cpfs.insert(cpfRegistrado).second) {
            return {false, false,
                    "CPF duplicado no arquivo de adesões SNE, linha " +
                        to_string(numeroLinha) + "."};
        }
    }

    if (arquivo.bad()) {
        return {false, false,
                "Erro ao ler o arquivo de adesões SNE: " + caminhoArquivo_};
    }

    return {true, cpfs.find(cpf) != cpfs.end(), {}};
}

// Insere no .txt o novo cpf
ResultadoConsultaAdesaoSNE RepositorioAdesaoSNETxt::registrarAdesao(const string& cpf) const {

    if (cpf.empty() || cpf.find_first_of("|\r\n") != string::npos) {
        return {false, false, "CPF inválido para registrar adesão ao SNE."};
    }

    const ResultadoConsultaAdesaoSNE consulta = consultarPorCPF(cpf);
    if (!consulta.sucesso) {
        return consulta;
    }
    if (consulta.aderiu) {
        return {true, true, {}};
    }

    ofstream arquivo(caminhoArquivo_, ios::app);
    if (!arquivo.is_open()) {
        return {false, false,
                "Não foi possível gravar o arquivo de adesões SNE: " +
                    caminhoArquivo_};
    }
    arquivo << cpf << '\n';
    if (!arquivo.good()) {
        return {false, false,
                "Erro ao gravar a adesão no arquivo SNE: " +
                    caminhoArquivo_};
    }
    
    return {true, true, {}};
}
