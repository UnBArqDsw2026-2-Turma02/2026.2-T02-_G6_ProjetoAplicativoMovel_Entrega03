#pragma once

#include "../../Dominios/usuario.hpp"

#include <memory>
#include <optional>
#include <string>
using namespace std;

enum class TipoDocumentoSNE {
    CNH,
    CRLV
};

struct DadosAdesaoSNE {
    string numeroCNH;
    string codigoSegurancaCNH;
    string placa;
    string renavam;
    string numeroSegurancaCRV;
};

struct RequisicaoAdesaoSNE {
    string cpfUsuario;
    TipoDocumentoSNE tipoDocumento;
    string identificadorDocumento;
};

// [Strategy: interface]
class EstrategiaDocumentoSNE {
public:
    virtual ~EstrategiaDocumentoSNE() = default;

    virtual optional<RequisicaoAdesaoSNE> validarEPreparar(const Usuario& usuario, const DadosAdesaoSNE& dados) const = 0;
};

//----------------------------------------------------------------------------
// [Strategy: concrete]
class EstrategiaDocumentoCNH final : public EstrategiaDocumentoSNE {
public:
    optional<RequisicaoAdesaoSNE> validarEPreparar(const Usuario& usuario, const DadosAdesaoSNE& dados) const override {
        
        if (dados.numeroCNH.empty() || dados.codigoSegurancaCNH.empty() || usuario.numeroCNH.empty() ||
            usuario.codigoSegurancaCNH.empty() || dados.numeroCNH != usuario.numeroCNH || dados.codigoSegurancaCNH != usuario.codigoSegurancaCNH) {
            
                return nullopt;
        }

        return RequisicaoAdesaoSNE{
            usuario.cpf, TipoDocumentoSNE::CNH, dados.numeroCNH
        };
    }
};

//-------------------------------------------------------------------------------
// [Strategy: concrete]
class EstrategiaDocumentoCRLV final : public EstrategiaDocumentoSNE {
public:
    optional<RequisicaoAdesaoSNE> validarEPreparar(const Usuario& usuario, const DadosAdesaoSNE& dados) const override {
        if (dados.placa.empty() || dados.renavam.empty() ||
            dados.numeroSegurancaCRV.empty() || usuario.placa.empty() ||
            usuario.renavam.empty() || usuario.numeroSegurancaCRV.empty() ||
            dados.placa != usuario.placa ||
            dados.renavam != usuario.renavam ||
            dados.numeroSegurancaCRV != usuario.numeroSegurancaCRV) {
                return nullopt;
        }

        return RequisicaoAdesaoSNE{
            usuario.cpf, TipoDocumentoSNE::CRLV, dados.placa + "/" + dados.renavam
        };
    }
};

// [Strategy: context] Mantém a Strategy selecionada e delega a validação.
class ContextoValidacaoDocumentoSNE {
public:
    explicit ContextoValidacaoDocumentoSNE(unique_ptr<EstrategiaDocumentoSNE> estrategia = {})
        : estrategia_() {
        estrategia_.swap(estrategia);
    }

    void definirEstrategia(unique_ptr<EstrategiaDocumentoSNE>& estrategia) {
        estrategia_.swap(estrategia);
    }

    optional<RequisicaoAdesaoSNE> validarEPreparar(const Usuario& usuario, const DadosAdesaoSNE& dados) const {
        
        if (!estrategia_) {
            return nullopt;
        }
        
        return estrategia_->validarEPreparar(usuario, dados);
    }

private:
    unique_ptr<EstrategiaDocumentoSNE> estrategia_;
};
