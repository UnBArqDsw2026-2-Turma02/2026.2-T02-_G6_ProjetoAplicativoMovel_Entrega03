#pragma once

#include <string>
#include <vector>
using namespace std;

struct RegistroUsuarioAutenticacao {
    string cpf;
    string nome;
    string senha;
    bool aderiuSNE = false;
    string numeroCNH;
    string codigoSegurancaCNH;
    string placa;
    string renavam;
    string numeroSegurancaCRV;
};

enum class StatusConsultaUsuario {
    ENCONTRADO,
    NAO_ENCONTRADO,
    ERRO
};

struct ResultadoConsultaUsuario {
    StatusConsultaUsuario status = StatusConsultaUsuario::ERRO;
    RegistroUsuarioAutenticacao registro;
    string mensagem;
};

// [Adapter: adaptee] API mockada de consulta aos usuários.
class ApiUsuariosArquivoMock {
public:
    explicit ApiUsuariosArquivoMock(string caminhoArquivo);

    ResultadoConsultaUsuario consultarPorCpf(const string& cpf) const;

private:
    string caminhoArquivo_;
};
