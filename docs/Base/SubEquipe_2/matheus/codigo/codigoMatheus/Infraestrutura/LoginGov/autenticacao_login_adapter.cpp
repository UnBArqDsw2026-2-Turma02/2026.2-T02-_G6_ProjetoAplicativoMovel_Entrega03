#include "autenticacao_login_adapter.hpp"
using namespace std;

AutenticacaoLoginAdapter::AutenticacaoLoginAdapter(const ApiUsuariosArquivoMock& apiUsuarios)
    : apiUsuarios_(apiUsuarios) {}

ResultadoAutenticacao AutenticacaoLoginAdapter::autenticar(const string& cpf, const string& senha) const {
    
    // Converte a entrada do Gateway em consulta por CPF
    const ResultadoConsultaUsuario consulta = apiUsuarios_.consultarPorCpf(cpf);

    if (consulta.status == StatusConsultaUsuario::ERRO) {
        return {StatusAutenticacao::ERRO, {}, consulta.mensagem};
    }

    if (consulta.status == StatusConsultaUsuario::NAO_ENCONTRADO || consulta.registro.senha != senha) {
        return {StatusAutenticacao::CREDENCIAIS_INVALIDAS, {}, "CPF ou senha inválidos."};
    }

    // Converte o registro do arquivo no modelo Usuario do domínio
    Usuario usuario;
    usuario.cpf = consulta.registro.cpf;
    usuario.nome = consulta.registro.nome;
    usuario.aderiuSNE = consulta.registro.aderiuSNE;
    usuario.numeroCNH = consulta.registro.numeroCNH;
    usuario.codigoSegurancaCNH = consulta.registro.codigoSegurancaCNH;
    usuario.placa = consulta.registro.placa;
    usuario.renavam = consulta.registro.renavam;
    usuario.numeroSegurancaCRV = consulta.registro.numeroSegurancaCRV;

    return {StatusAutenticacao::AUTENTICADO, usuario, {}};
}
