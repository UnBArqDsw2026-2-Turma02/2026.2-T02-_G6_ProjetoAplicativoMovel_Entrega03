#pragma once

#include "../../Aplicacao/LoginGov/gateway_autenticacao.hpp"
#include "Mock/api_usuarios_arquivo_mock.hpp"
using namespace std;

// [Adapter] Converte a resposta da API mockada no Target.
// GatewayAutenticacao e entrega somente o modelo de domínio Usuario.
class AutenticacaoLoginAdapter : public GatewayAutenticacao {
public:
    explicit AutenticacaoLoginAdapter(const ApiUsuariosArquivoMock& apiUsuarios);

    ResultadoAutenticacao autenticar(const string& cpf, const string& senha) const override;

private:
    const ApiUsuariosArquivoMock& apiUsuarios_;
};
