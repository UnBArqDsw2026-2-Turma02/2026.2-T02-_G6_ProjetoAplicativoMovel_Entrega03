#include "fluxo_infracoes.hpp"
#include "../../SNE/fluxo_adesao_sne.hpp"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace {

// Usando pra pegar as opções do usuario (simulando os botoes)
bool lerInteiro(istream& entrada,ostream& saida,const string& mensagem,int minimo,int maximo,int& valor) {

    saida << mensagem;
    string linha;

    if (!getline(entrada, linha)) {
        return false;
    }

    istringstream conversor(linha);
    char restante = '\0';
    if (!(conversor >> valor) || (conversor >> restante) || valor < minimo || valor > maximo) {
        saida << "Opção inválida.\n";
        valor = -1;
    }

    return true;
}

// opção de entrada (s/n)
bool lerConfirmacao(
    istream& entrada,
    ostream& saida,
    const string& mensagem,
    bool& confirmou) {
    saida << mensagem << " (s/n): ";
    string resposta;
    if (!getline(entrada, resposta)) {
        return false;
    }
    confirmou = resposta == "s" || resposta == "S";
    if (!confirmou && resposta != "n" && resposta != "N") {
        saida << "Resposta inválida; operação cancelada.\n";
    }
    return true;
}

// prints ao detalhar infração
void mostrarDetalhes(ostream& saida, const Infracao& infracao) {
    saida << "\nID: " << infracao.id << "\nSituação: ";
    
    switch (infracao.status) {
        case StatusInfracao::VENCIDA:
            saida << "Vencida";
            break;
        
        case StatusInfracao::A_VENCER:
            saida << "A vencer";
            break;
        
        case StatusInfracao::PAGA:
            saida << "Paga";
            break;
    }
    
    saida << "\nMotivo: " << infracao.motivo
          << "\nData e horário: " << infracao.data << " às "
          << infracao.horario
          << "\nValor original: R$ " << fixed << setprecision(2)
          << infracao.valorOriginal
          << "\nPlaca: " << infracao.placa
          << "\nRENainf: " << infracao.renainf
          << "\nAuto: " << infracao.numeroAuto
          << "\nCódigo: " << infracao.codigoInfracao
          << "\nÓrgão autuador: " << infracao.orgao.nome
          << "\nLocal: " << infracao.local
          << "\nData/hora do cometimento: " << infracao.dataHoraCometimento
          << "\nNotificação de autuação: " << infracao.dataNotificacaoAutuacao
          << "\nPrazo de defesa: " << infracao.prazoDefesa
          << "\nNotificação de penalidade: "
          << infracao.dataNotificacaoPenalidade
          << "\nPrazo de recurso: " << infracao.prazoRecurso
          << "\nVencimento de desconto: " << infracao.vencimentoDesconto
          << "\n";
}

bool solicitarDesconto(istream& entrada, ostream& saida, ostream& erros, InfracaoFacade& facade, SNEFacade& sneFacade, Usuario& usuario, const Infracao& infracao) {
    
    const vector<int> opcoes = facade.obterOpcoesDesconto(infracao);
    
    if (opcoes.empty()) {
        saida << "Não há descontos disponíveis para esta infração.\n";
        return true;
    }

    saida << "Descontos disponíveis:";
    for (const int percentual : opcoes) {
        saida << ' ' << percentual << '%';
    }
    
    saida << "\nDigite 0 para voltar. Percentual: ";

    int percentual = 0;
    string linha;
    if (!getline(entrada, linha)) {
        return false;
    }
    
    istringstream conversor(linha);
    char restante = '\0';
    
    // validação do desconto escolhido pelo user
    if (!(conversor >> percentual) || (conversor >> restante) || (percentual != 0 && find(opcoes.begin(), opcoes.end(), percentual) == opcoes.end())) {
        saida << "Percentual inválido.\n";
        return true;
    }

    if (percentual == 0) {
        return true;
    }

    bool termoAceito = false;

// ---------------------------------
// Regras para 40% (lado do usuario, aq o orgao ja passou pelo strategy e liverou 40%)
    if (percentual == 40) {
        if (!usuario.aderiuSNE) {
            saida << "O desconto de 40% requer adesão ao SNE.\n";
            
            if (!oferecerAdesaoSNE(usuario, sneFacade, entrada, saida, erros)) {
                return false;
            }
            
            if (!usuario.aderiuSNE) {
                saida << "Solicitação de desconto cancelada: adesão ao SNE não concluída.\n";
                return true;
            }
        }

        saida << "O desconto de 40% exige reconhecimento da infração.\n";
        
        if (!lerConfirmacao(entrada, saida, "Aceita o termo?", termoAceito)) {
            return false;
        }
        
        if (!termoAceito) {
            saida << "Solicitação cancelada.\n";
            return true;
        }
    }

    // envia a solicitação
    const ResultadoOperacaoInfracao resultado = facade.solicitarDesconto(usuario, infracao, percentual, termoAceito);
    (resultado.sucesso ? saida : erros) << resultado.mensagem << '\n';

    return true;
}

bool fluxoBoleto(istream& entrada, ostream& saida, ostream& erros, InfracaoFacade& facade, SNEFacade& sneFacade, Usuario& usuario, const Infracao& infracao) {
    
    while (true) {
    
        saida << "\n-- Boleto da infração " << infracao.id << " --\n"
              << "1 - Solicitar boleto\n"
              << "2 - Consultar situação\n"
              << "0 - Voltar\n";

        int opcao = -1;
        if (!lerInteiro(entrada, saida, "Escolha: ", 0, 2, opcao)) {
            return false;
        }

        if (opcao == -1) {
            continue;
        }
        

        if (opcao == 0) {
            return true;
        }

// ---------------------------
// Solicitar Boleto
        if (opcao == 1) {
            const ResultadoConsultaBoleto existente = facade.consultarBoleto(infracao.id);

            if (!existente.sucesso) {   // tenta verificar se o boleto ja existe
                erros << "Não foi possível verificar o boleto existente: " << existente.mensagem << '\n';
                continue;
            }
            
            // Se o boleto ja existir avisa e o desconto atual
            if (existente.boleto) { 
                const int percentualAtual = existente.boleto->percentualDesconto;
                saida << "Já existe um boleto com desconto de " << percentualAtual << "%.\n";

                vector<int> opcoes = facade.obterOpcoesDesconto(infracao); // verifica os descontos disponiveis

                opcoes.erase(remove(opcoes.begin(), opcoes.end(), percentualAtual), opcoes.end()); // remove das opções de desconto o atual
                if (opcoes.empty()) { // se nao tiver nenhum desconto disponivel
                    saida << "Não há outro percentual de desconto disponível para atualizar este boleto.\n";
                    continue;
                }

                saida << "Escolha um novo desconto:";
                for (const int percentual : opcoes) {
                    saida << ' ' << percentual << '%';
                }

                saida << " (0 para cancelar): ";
                
                string linha;
                if (!getline(entrada, linha)) {
                    return false;
                }
                
                istringstream conversor(linha);
                int novoPercentual = 0;
                char restante = '\0';

                // verifica se a opção passada pelo usuario esta disponivel/existe
                if (!(conversor >> novoPercentual) || (conversor >> restante) || (novoPercentual != 0 &&
                     find(opcoes.begin(), opcoes.end(), novoPercentual) == opcoes.end())) {
                    saida << "Percentual inválido.\n";
                    continue;
                }

                if (novoPercentual == 0) {
                    continue;
                }

                // Se for escolhido 40% e ainda nn aderiu a SNE aparece oa viso
                if (novoPercentual == 40 && !usuario.aderiuSNE) {
                    saida << "O desconto de 40% requer adesão ao SNE.\n";
                    if (!oferecerAdesaoSNE(usuario, sneFacade, entrada, saida, erros)) {
                        return false;
                    }

                    // Se nao aderir
                    if (!usuario.aderiuSNE) {
                        saida << "Atualização do boleto cancelada: adesão ao "
                                 "SNE não concluída.\n";
                        continue;
                    }
                }

                // Aceitação dos termos de condição (não printei os termos pq ia ficar ruim de ler no terminal)
                bool termoAceito = false;
                if (novoPercentual == 40) {
                    saida << "O desconto de 40% exige reconhecimento da infração.\n";

                    if (!lerConfirmacao(entrada, saida, "Aceita o termo?", termoAceito)) {
                        return false;
                    }

                    if (!termoAceito) {
                        saida << "Atualização cancelada.\n";
                        continue;
                    }
                }

                const ResultadoOperacaoInfracao atualizacao = facade.atualizarBoletoComDesconto(usuario, infracao, novoPercentual, termoAceito);
                
                (atualizacao.sucesso ? saida : erros) << atualizacao.mensagem << '\n';
                continue;
            }

            // Agora após conferir as opção e condição de cada um faz a solicitação
            const ResultadoOperacaoInfracao resultado = facade.solicitarBoleto(usuario, infracao);
            (resultado.sucesso ? saida : erros) << resultado.mensagem << '\n';

            continue;
        }

        // Status do boleto
        const ResultadoConsultaBoleto consulta = facade.consultarSituacaoBoleto(usuario, infracao.id);

        if (!consulta.sucesso) {
            erros << consulta.mensagem << '\n';
            continue;
        }

        if (!consulta.boleto) {
            if (consulta.emAnalise) {
                saida << "Situação: Em análise. O pedido de boleto foi "
                         "registrado e aguarda emissão.\n";
            } else {
                saida << "A API TXT ainda não possui boleto para esta infração "
                         "e não há pedido em análise.\n";
            }
            continue;
        }

        const Boleto& boleto = *consulta.boleto;
        saida << "Situação: ";

        switch (boleto.status) {
            case StatusBoleto::NAO_SOLICITADO:
                saida << "Não solicitado\n";
                break;
            
            case StatusBoleto::PROCESSANDO:
                saida << "Em processamento\n";
                break;
            
            case StatusBoleto::PRONTO:
                saida << "Pronto\n"
                      << "Número do auto: " << boleto.numeroAuto << '\n'
                      << "Vencimento: " << boleto.dataVencimento << '\n'
                      << "Valor original: R$ " << fixed << setprecision(2) << boleto.valorOriginal << '\n'
                      << "Desconto: R$ " << boleto.valorDesconto << '\n'
                      << "Valor a pagar: R$ " << boleto.valorPagar << '\n';
                break;
            
            case StatusBoleto::ERRO:
                saida << "Erro\n";
                break;
        }

        if (boleto.status != StatusBoleto::PRONTO) {
            continue;
        }

        saida << "1 - Copiar código de pagamento\n"
              << "2 - Abrir PDF\n"
              << "3 - Baixar PDF\n"
              << "4 - Compartilhar PDF\n"
              << "5 - Imprimir PDF\n"
              << "0 - Voltar\n";
        int acao = -1;
        
        if (!lerInteiro(entrada, saida, "Escolha: ", 0, 5, acao)) {
            return false;
        }
    
// ---------------------------

        if (acao == 1) {
            if (facade.copiarCodigoPagamento(boleto)) {
                saida << "Código de pagamento copiado (simulação).\n";
            } else {
                erros << "Não foi possível copiar o código de pagamento.\n";
            }
        } 

// ---------------------------
        else if (acao == 2) {
            if (facade.exibirPDFBoleto(boleto)) {
                saida << "PDF aberto (simulação): " << boleto.arquivoPDF
                      << '\n';
            } else {
                erros << "Não foi possível abrir o PDF do boleto.\n";
            }
        } 

// ---------------------------
        else if (acao == 3) {
            if (facade.baixarDocumento(boleto.arquivoPDF)) {
                saida << "PDF baixado (simulação): " << boleto.arquivoPDF
                      << '\n';
            } else {
                erros << "Não foi possível baixar o PDF do boleto.\n";
            }
        } 

// ---------------------------
        else if (acao == 4) {
            if (facade.compartilharDocumento(boleto.arquivoPDF)) {
                saida << "PDF compartilhado (simulação): "
                      << boleto.arquivoPDF << '\n';
            } else {
                erros << "Não foi possível compartilhar o PDF do boleto.\n";
            }
        } 

// ---------------------------
        else if (acao == 5) {
            if (facade.imprimirDocumento(boleto.arquivoPDF)) {
                saida << "Impressão solicitada (simulação): "
                      << boleto.arquivoPDF << '\n';
            } else {
                erros << "Não foi possível imprimir o PDF do boleto.\n";
            }
        } 

// ---------------------------
        else if (acao == -1) {
            continue;
        }
    }
}

bool fluxoDocumentoInfracao(istream& entrada, ostream& saida, ostream& erros, InfracaoFacade& facade, const Infracao& infracao) {

    const string arquivo = infracao.numeroAuto + ".pdf";    // "criação" do pdf

    while (true) {
    
        saida << "\nDocumento da infração: " << arquivo << '\n'
              << "1 - Abrir\n"
              << "2 - Baixar\n"
              << "3 - Compartilhar\n"
              << "4 - Imprimir\n"
              << "0 - Voltar\n";
    
        int acao = -1;
        if (!lerInteiro(entrada, saida, "Escolha: ", 0, 4, acao)) {
            return false;
        }

// ---------------------------
// Voltar
        if (acao == 0) {
            return true;
        }
        
        bool sucesso = false;
        
// ---------------------------
// Abrir
        if (acao == 1) {
            sucesso = facade.exibirPDFInfracao(infracao);
        } 
        
// ---------------------------
// Baixar
        else if (acao == 2) {
            sucesso = facade.baixarDocumento(arquivo);
        }
        
// ---------------------------
        else if (acao == 3) {
            sucesso = facade.compartilharDocumento(arquivo);
        } 

// ---------------------------
        else if (acao == 4) {
            sucesso = facade.imprimirDocumento(arquivo);
        }
        
// ---------------------------
        if (acao < 1 || acao > 4) {
            continue;
        }
        
        (sucesso ? saida : erros) << (sucesso ? "Ação de documento concluída (simulação).\n"
            : "Não foi possível concluir a ação no documento.\n");
    }
}

bool mostrarInfracao(istream& entrada, ostream& saida, ostream& erros, InfracaoFacade& facade, SNEFacade& sneFacade, Usuario& usuario, const Infracao& infracao) {
 
    while (true) {
        saida << "\n-- Infração " << infracao.id << " --\n"
              << "1 - Consultar detalhes\n"
              << "2 - Consultar/solicitar desconto\n"
              << "3 - Solicitar boleto\n"
              << "4 - Abrir documentos da infração\n"
              << "0 - Voltar\n";
 
        int opcao = -1;
        
        if (!lerInteiro(entrada, saida, "Escolha: ", 0, 4, opcao)) 
            return false;
        
        if (opcao == -1) 
            continue;

// ---------------------------
// Voltear
        if (opcao == 0) 
            return true;

            // ---------------------------
// Consultar detalhes
        if (opcao == 1) {
            const ResultadoDetalhesInfracao detalhes = facade.obterDetalhes(infracao.id);
            
            if (!detalhes.sucesso) {
                erros << detalhes.mensagem << '\n';
                return false;
            }

            if (!detalhes.infracao) {
                saida << "Infração não encontrada.\n";
            } 
            
            else {
                mostrarDetalhes(saida, *detalhes.infracao);
            }

        } 
        
// ---------------------------
// Desconto
        else if (opcao == 2) {
            if (!solicitarDesconto(entrada, saida, erros, facade, sneFacade, usuario, infracao)) {
                return false;
            }
        } 
        
// ---------------------------
// Boleto
        else if (opcao == 3) {
            if (!fluxoBoleto(entrada, saida, erros, facade, sneFacade, usuario, infracao)) {
                return false;
            }
        } 

// ---------------------------
// Documento da infração
        else if (opcao == 4 && !fluxoDocumentoInfracao(entrada, saida, erros, facade, infracao)) {
            return false;
        }
    }
}

bool selecionarInfracao(istream& entrada, ostream& saida, ostream& erros, InfracaoFacade& facade, SNEFacade& sneFacade, Usuario& usuario, const vector<Infracao>& infracoes) {
    
    if (infracoes.empty()) {
        saida << "Nenhuma infração encontrada para este filtro.\n";
        return true;
    }

    while (true) {

        saida << "\nInfrações:\n";
        
        for (size_t i = 0; i < infracoes.size(); ++i) {
            saida << i + 1 << " - " << infracoes[i].id << " | " << infracoes[i].motivo 
            << " | " << infracoes[i].placa << "\n";
        }
        
        int opcao = -1;
        if (!lerInteiro(entrada, saida, "Selecione (0 para voltar): ", 0, static_cast<int>(infracoes.size()), opcao)) {
            return false;
        }

        if (opcao == -1) 
            continue;
        
        
        if (opcao == 0) 
            return true;
     
        
        if (!mostrarInfracao(entrada, saida, erros, facade, sneFacade, usuario, infracoes[static_cast<size_t>(opcao - 1)])) {
            return false;
        }
    }
}

bool consultarPorVeiculo(istream& entrada, ostream& saida, ostream& erros, InfracaoFacade& facade, SNEFacade& sneFacade, Usuario& usuario, const vector<Infracao>& infracoes) {

    set<string> placas;
    for (const Infracao& infracao : infracoes) {
        placas.insert(infracao.placa);
    }

    if (placas.empty()) {
        saida << "Nenhum veículo com infrações foi encontrado.\n";
        return true;
    }

    const vector<string> listaPlacas(placas.begin(), placas.end());

    while (true) {
        saida << "\nVeículos:\n";
        for (size_t i = 0; i < listaPlacas.size(); ++i) {
            saida << i + 1 << " - " << listaPlacas[i] << '\n';
        }

        int opcao = -1;
        
        if (!lerInteiro(entrada, saida, "Selecione (0 para voltar): ", 0, static_cast<int>(listaPlacas.size()), opcao)) {
            return false;
        }

        if (opcao == -1) {
            continue;
        }

        if (opcao == 0) {
            return true;
        }

        const string& placa = listaPlacas[static_cast<size_t>(opcao - 1)]; // placa ja escolhida pelo usuario
        vector<Infracao> associadas;
      
        for (const Infracao& infracao : infracoes) { // infracoes relacionada a placa escolhida pelo usuario
            if (infracao.placa == placa) {
                associadas.push_back(infracao);
            }
        }
      
        if (!selecionarInfracao(entrada, saida, erros, facade, sneFacade, usuario, associadas)) {
            return false;
        }
    }
}
}  // namespace

bool executarFluxoInfracoes(Usuario& usuario, InfracaoFacade& facade, SNEFacade& sneFacade, istream& entrada, ostream& saida, ostream& erros) {

    const ResultadoListaInfracoes consulta = facade.consultarInfracoes(usuario.cpf);
    
    if (!consulta.sucesso) {
        erros << "Erro ao consultar infrações: " << consulta.mensagem << '\n';
        return false;
    }

    while (true) {
        saida << "\n=== Infrações de " << usuario.nome << " ===\n"
              << "1 - Consultar por veículo\n"
              << "2 - Consultar por infrator\n";
        
        if (!usuario.aderiuSNE) {
            saida << "3 - Aderir ao SNE\n";
        }
        saida << "0 - Voltar\n";
        int opcao = -1;
        const int opcaoMaxima = usuario.aderiuSNE ? 2 : 3;
        if (!lerInteiro(entrada, saida, "Escolha: ", 0, opcaoMaxima, opcao)) {
            return true;
        }
        if (opcao == -1) {
            continue;
        }
        if (opcao == 0) {
            return true;
        }
        if (opcao == 3 && !usuario.aderiuSNE) {
            if (!oferecerAdesaoSNE(usuario, sneFacade, entrada, saida, erros)) {
                return false;
            }
            continue;
        }
        if (opcao == 1) {
            if (!consultarPorVeiculo(entrada, saida, erros, facade, sneFacade, usuario, consulta.infracoes)) {
                return false;
            }
        } else if (!selecionarInfracao(entrada, saida, erros, facade, sneFacade, usuario, consulta.infracoes)) {
            return false;
        }
    }
}
