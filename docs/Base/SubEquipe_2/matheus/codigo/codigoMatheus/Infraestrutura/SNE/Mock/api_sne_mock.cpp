#include "api_sne_mock.hpp"
using namespace std;

ApiSNEMock::ApiSNEMock(const RepositorioAdesaoSNETxt& repositorio)
    : repositorio_(repositorio) {}

ResultadoConsultaAdesaoSNE ApiSNEMock::consultarAdesao(
    const string& cpfUsuario) const {
    return repositorio_.consultarPorCPF(cpfUsuario);
}

ResultadoConsultaAdesaoSNE ApiSNEMock::efetivarAdesao(
    const RequisicaoAdesaoSNE& requisicao) const {
    return repositorio_.registrarAdesao(requisicao.cpfUsuario);
}
