#pragma once

#include "../../../Dominios/infracao.hpp"

#include <memory>
#include <vector>
using namespace std;

// [Strategy: interface]
class PoliticaBeneficio {
public:
    virtual ~PoliticaBeneficio() = default;
    virtual vector<int> obterPercentuaisDisponiveis() const = 0;
};

// [Strategy: concrete]
class PoliticaOrgaoSNE final : public PoliticaBeneficio {
public:
    vector<int> obterPercentuaisDisponiveis() const override {
        return {20, 40};
    }
};

// [Strategy: concrete]
class PoliticaCDT20 final : public PoliticaBeneficio {
public:
    vector<int> obterPercentuaisDisponiveis() const override {
        return {20};
    }
};

// [Strategy: concrete]
class PoliticaSomenteConsulta final : public PoliticaBeneficio {
public:
    vector<int> obterPercentuaisDisponiveis() const override {
        return {};
    }
};

// [Strategy: context] Seleciona e executa a política aplicável à infração.
class BeneficioService {
public:
    vector<int> obterOpcoes(const Infracao& infracao) {
        if (infracao.status != StatusInfracao::A_VENCER) {
            return {};
        }

        if (infracao.orgao.aderiuSNE) {
            estrategia_ = make_unique<PoliticaOrgaoSNE>();
        } else if (infracao.orgao.ofereceDesconto20CDT) {
            estrategia_ = make_unique<PoliticaCDT20>();
        } else {
            estrategia_ = make_unique<PoliticaSomenteConsulta>();
        }

        return estrategia_->obterPercentuaisDisponiveis();
    }

private:
    unique_ptr<PoliticaBeneficio> estrategia_;
};
