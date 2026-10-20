#pragma once

#include "../../../Aplicacao/SNE/estrategia_documento_sne.hpp"
#include "../repositorio_adesao_sne_txt.hpp"

#include <string>
using namespace std;

// [Adapter: adaptee] Simula a API SNE consultando o TXT.
class ApiSNEMock {
public:
    explicit ApiSNEMock(const RepositorioAdesaoSNETxt& repositorio);

    ResultadoConsultaAdesaoSNE consultarAdesao(
        const string& cpfUsuario) const;
    ResultadoConsultaAdesaoSNE efetivarAdesao(
        const RequisicaoAdesaoSNE& requisicao) const;

private:
    const RepositorioAdesaoSNETxt& repositorio_;
};
