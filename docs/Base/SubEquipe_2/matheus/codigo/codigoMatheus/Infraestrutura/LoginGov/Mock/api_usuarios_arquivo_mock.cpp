#include "api_usuarios_arquivo_mock.hpp"

#include <fstream>
#include <set>
#include <sstream>
#include <vector>
using namespace std;

namespace {

// "limpa" a string, limpando espacos antes da 1° palavra 
string aparar(const string& texto) {
    const string espacos = " \t\r\n";
    const size_t inicio = texto.find_first_not_of(espacos);

    if (inicio == string::npos) {
        return {};
    }

    const size_t fim = texto.find_last_not_of(espacos);
    return texto.substr(inicio, fim - inicio + 1);
}

// Leitura e verificação do usuarios.txt (usando | como separador)
bool separarRegistro(const string& linha, RegistroUsuarioAutenticacao& registro) {
    
    vector<string> campos;
    stringstream entrada(linha);
    string campo;

    while (getline(entrada, campo, '|')) {
        campos.push_back(aparar(campo));
    }
    
    if (!linha.empty() && linha.back() == '|') {
        campos.emplace_back();
    }

    if (campos.size() != 4 && campos.size() != 9) {
        return false;
    }

    registro.cpf = campos[0];
    registro.nome = campos[1];
    registro.senha = campos[2];

    if (campos[3] != "true" && campos[3] != "false") {
        return false;
    }

    registro.aderiuSNE = campos[3] == "true";
    
    if (campos.size() == 9) {
        registro.numeroCNH = campos[4];
        registro.codigoSegurancaCNH = campos[5];
        registro.placa = campos[6];
        registro.renavam = campos[7];
        registro.numeroSegurancaCRV = campos[8];
    }
    
    return !registro.cpf.empty() && !registro.nome.empty() && !registro.senha.empty();
}
}  // namespace

ApiUsuariosArquivoMock::ApiUsuariosArquivoMock(string caminhoArquivo)
    : caminhoArquivo_(caminhoArquivo) {}

ResultadoConsultaUsuario ApiUsuariosArquivoMock::consultarPorCpf(const string& cpf) const {
    
    ifstream arquivo(caminhoArquivo_);
    
    if (!arquivo.is_open()) {
        return {StatusConsultaUsuario::ERRO, {}, "Não foi possível abrir o arquivo de usuários: " + caminhoArquivo_};
    }

    vector<RegistroUsuarioAutenticacao> registros;
    set<string> cpfs;
    string linha;
    size_t numeroLinha = 0;

    while (getline(arquivo, linha)) {

        ++numeroLinha;

        const string linhaAparada = aparar(linha);
        
        // pra poder "comentar" no txt, usei pra cabeçakho como se fosse csv
        if (linhaAparada.empty() || linhaAparada.front() == '#') {
            continue;
        }

        RegistroUsuarioAutenticacao registro;

        // verificação do formato
        if (!separarRegistro(linhaAparada, registro)) {
            return {StatusConsultaUsuario::ERRO, {},
                    "Registro inválido na linha " +
                        to_string(numeroLinha) +
                        ". Use o formato cpf|nome|senha|aderiuSNE, com "
                        "aderiuSNE igual a true ou false; opcionalmente "
                        "adicione numeroCNH|codigoSegurancaCNH|placa|renavam|"
                        "numeroSegurancaCRV."};
        }

        // verificação de CPF repetido
        if (!cpfs.insert(registro.cpf).second) {
            return {StatusConsultaUsuario::ERRO, {}, "CPF duplicado no arquivo, na linha " + to_string(numeroLinha) + "."};
        }

        // registra
        registros.push_back(registro);
    }

    if (arquivo.bad()) {
        return {StatusConsultaUsuario::ERRO, {},
                "Erro ao ler o arquivo de usuários: " + caminhoArquivo_};
    }

    if (registros.empty()) {
        return {StatusConsultaUsuario::ERRO, {},
                "O arquivo de usuários não contém registros."};
    }

    for (const auto& registro : registros) {
        if (registro.cpf == cpf) {
            return {StatusConsultaUsuario::ENCONTRADO, registro, {}};
        }
    }

    return {StatusConsultaUsuario::NAO_ENCONTRADO, {},
            "CPF não cadastrado."};
}
