#pragma once

#include "../../Dominios/usuario.hpp"

#include <optional>
#include <string>
using namespace std;

enum class StatusAutenticacao {
    AUTENTICADO,
    CREDENCIAIS_INVALIDAS,
    ERRO
};

struct ResultadoAutenticacao {
    StatusAutenticacao status = StatusAutenticacao::ERRO;
    optional<Usuario> usuario;
    string mensagem;

    bool autenticado() const {
        return status == StatusAutenticacao::AUTENTICADO && usuario.has_value();
    }
};

// [Adapter: interface]
// Target: contrato esperado pelo cliente para autenticar um usuário
class GatewayAutenticacao {
public:
    virtual ~GatewayAutenticacao() = default;

    virtual ResultadoAutenticacao autenticar(
        const string& cpf,
        const string& senha) const = 0;
};
