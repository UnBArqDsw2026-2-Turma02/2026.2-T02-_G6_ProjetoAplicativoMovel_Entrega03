/* MVP SubEquipe_01 - lógica compartilhada pelas telas. */

const CHAVE_SELECAO = "mvp-subequipe01-selecao";

// ---------- Notificações ----------
function notificar(titulo, mensagem, tipoNotificacao = "info") {
  const areaNotificacoes = document.getElementById("notificacoes");
  const notificacao = document.createElement("div");
  notificacao.className = `notificacao notificacao--${tipoNotificacao}`;
  notificacao.setAttribute("role", tipoNotificacao === "erro" ? "alert" : "status");
  notificacao.innerHTML = `
    <div class="notificacao__texto">
      <span class="notificacao__titulo"></span>
      <span class="notificacao__mensagem"></span>
    </div>
    <button class="notificacao__fechar" aria-label="Fechar notificação">&times;</button>`;
  notificacao.querySelector(".notificacao__titulo").textContent = titulo;
  notificacao.querySelector(".notificacao__mensagem").textContent = mensagem;

  const fecharNotificacao = () => {
    notificacao.classList.add("notificacao--saindo");
    setTimeout(() => notificacao.remove(), 300);
  };
  notificacao.querySelector(".notificacao__fechar").addEventListener("click", fecharNotificacao);
  areaNotificacoes.appendChild(notificacao);
  setTimeout(fecharNotificacao, 5000);
}

// ---------- API ----------
async function chamarApi(caminhoRota, corpoRequisicao) {
  const opcoesRequisicao = corpoRequisicao
    ? { method: "POST", headers: { "Content-Type": "application/json" }, body: JSON.stringify(corpoRequisicao) }
    : {};
  try {
    const resposta = await fetch(caminhoRota, opcoesRequisicao);
    const conteudo = await resposta.json();
    if (!resposta.ok) throw new Error(conteudo.erro || "Erro na requisição.");
    return conteudo;
  } catch (erroRequisicao) {
    notificar("Não foi possível concluir", erroRequisicao.message, "erro");
    throw erroRequisicao;
  }
}

// ---------- Seleção de veículo (persistida entre as telas) ----------
function lerSelecao() {
  try {
    return JSON.parse(localStorage.getItem(CHAVE_SELECAO)) || { uf: "DF", placa: null };
  } catch {
    return { uf: "DF", placa: null };
  }
}

function salvarSelecao(selecaoAtual) {
  try {
    localStorage.setItem(CHAVE_SELECAO, JSON.stringify(selecaoAtual));
  } catch {
    /* navegação privada: a seleção vale só para esta tela */
  }
}

async function montarSeletorVeiculo(aoSelecionarVeiculo) {
  const containerSeletor = document.getElementById("seletor-veiculo");
  containerSeletor.innerHTML = `
    <div class="seletor">
      <div class="campo">
        <label for="campo-uf">Unidade federativa (base Multiton)</label>
        <select id="campo-uf"><option value="DF">DF</option><option value="GO">GO</option></select>
      </div>
      <div class="campo">
        <label for="campo-veiculo">Veículo</label>
        <select id="campo-veiculo"></select>
      </div>
    </div>`;
  const campoUf = containerSeletor.querySelector("#campo-uf");
  const campoVeiculo = containerSeletor.querySelector("#campo-veiculo");
  let veiculosDaBase = [];
  let identificadorBase = null;

  const selecionarVeiculo = (avisarUsuario) => {
    const veiculoEscolhido = veiculosDaBase.find((veiculo) => veiculo.placa === campoVeiculo.value);
    salvarSelecao({ uf: campoUf.value, placa: veiculoEscolhido.placa });
    if (avisarUsuario) {
      notificar("Veículo selecionado", `${veiculoEscolhido.placa} (${veiculoEscolhido.proprietario}) na base ${campoUf.value}.`, "info");
    }
    aoSelecionarVeiculo({ uf: campoUf.value, veiculo: veiculoEscolhido, idBase: identificadorBase });
  };

  const carregarVeiculos = async (avisarUsuario) => {
    const respostaVeiculos = await chamarApi(`/api/veiculos?uf=${campoUf.value}`);
    veiculosDaBase = respostaVeiculos.veiculos;
    identificadorBase = respostaVeiculos.id_base;
    campoVeiculo.innerHTML = veiculosDaBase
      .map((veiculo) => `<option value="${veiculo.placa}">${veiculo.placa} — ${veiculo.proprietario}</option>`)
      .join("");
    const selecaoSalva = lerSelecao();
    if (veiculosDaBase.some((veiculo) => veiculo.placa === selecaoSalva.placa)) {
      campoVeiculo.value = selecaoSalva.placa;
    }
    selecionarVeiculo(avisarUsuario);
  };

  campoUf.value = lerSelecao().uf || "DF";
  campoUf.addEventListener("change", () => {
    notificar("Base trocada", `Usando a instância do Multiton para ${campoUf.value}.`, "info");
    carregarVeiculos(false);
  });
  campoVeiculo.addEventListener("change", () => selecionarVeiculo(true));
  await carregarVeiculos(false);
}

// ---------- Utilitários de renderização ----------
function escaparHtml(textoOriginal) {
  const elementoTemporario = document.createElement("span");
  elementoTemporario.textContent = textoOriginal ?? "";
  return elementoTemporario.innerHTML;
}

function listaHtml(itensLista) {
  return itensLista.length
    ? `<ul>${itensLista.map((itemLista) => `<li>${escaparHtml(itemLista)}</li>`).join("")}</ul>`
    : `<p class="vazio">Nada consta.</p>`;
}

function dadosVeiculoHtml(veiculoAtual) {
  const enderecoAtual = veiculoAtual.endereco;
  return `
    <dl class="dados-veiculo">
      <div><dt>Placa</dt><dd>${escaparHtml(veiculoAtual.placa)}</dd></div>
      <div><dt>RENAVAM</dt><dd>${escaparHtml(veiculoAtual.renavam)}</dd></div>
      <div><dt>Proprietário</dt><dd>${escaparHtml(veiculoAtual.proprietario)}</dd></div>
      <div><dt>Endereço</dt><dd>${escaparHtml(enderecoAtual.uf)} · CEP ${escaparHtml(enderecoAtual.cep)}</dd></div>
    </dl>`;
}

function documentoHtml(documentoEmitido) {
  return `
    <article class="documento">
      <div class="documento__cabecalho"><span>${escaparHtml(documentoEmitido.tipo)}</span><span>Exercício ${escaparHtml(String(documentoEmitido.exercicio))}</span></div>
      <p><strong>Placa:</strong> ${escaparHtml(documentoEmitido.placa)} · <strong>RENAVAM:</strong> ${escaparHtml(documentoEmitido.renavam)}</p>
      <p>${escaparHtml(documentoEmitido.orgao_emissor)} · layout ${escaparHtml(documentoEmitido.versao_layout)}</p>
      <span class="documento__id">id do objeto: ${documentoEmitido.id}</span>
    </article>`;
}

// ---------- Telas ----------
const TELAS = {
  async inicio() {
    const painelVeiculo = document.getElementById("painel-veiculo");
    await montarSeletorVeiculo(({ veiculo }) => {
      const situacaoVeiculo = veiculo.debitos.length || veiculo.restricoes.length || veiculo.financiamentos.length
        ? `<p class="resultado resultado--pendente">Este veículo tem pendências: veja as consultas antes de emitir o CRLV-e.</p>`
        : `<p class="resultado resultado--limpo">Veículo sem pendências: pode seguir para a emissão do CRLV-e.</p>`;
      painelVeiculo.innerHTML = `
        ${dadosVeiculoHtml(veiculo)}
        ${situacaoVeiculo}`;
    });
  },

  async consultas() {
    const areaResultados = document.getElementById("resultados-consultas");
    const botaoProximo = document.getElementById("botao-proximo");
    const titulosConsultas = { debitos: "Débitos", restricoes: "Restrições", financiamentos: "Financiamentos", nada_consta: "Nada Consta" };
    await montarSeletorVeiculo(async ({ uf, veiculo }) => {
      const respostaConsultas = await chamarApi(`/api/consultas?uf=${uf}&renavam=${veiculo.renavam}`);
      areaResultados.innerHTML = Object.entries(respostaConsultas.resultados)
        .map(([nomeConsulta, resultadoConsulta]) => `
          <div class="resultado ${resultadoConsulta.limpo ? "resultado--limpo" : "resultado--pendente"}">
            <h3>${titulosConsultas[nomeConsulta]}</h3>
            ${listaHtml(resultadoConsulta.itens)}
          </div>`)
        .join("");
      const nadaConstaLimpo = respostaConsultas.resultados.nada_consta.limpo;
      notificar(
        "Consultas concluídas",
        nadaConstaLimpo ? `${veiculo.placa}: nada consta.` : `${veiculo.placa}: há pendências registradas.`,
        nadaConstaLimpo ? "sucesso" : "aviso",
      );
      botaoProximo.textContent = nadaConstaLimpo ? "Seguir para a emissão do CRLV-e →" : "Ver alteração de endereço →";
      botaoProximo.href = nadaConstaLimpo ? "crlve.html" : "endereco.html";
    });
  },

  async endereco() {
    const formularioEndereco = document.getElementById("formulario-endereco");
    const areaResultado = document.getElementById("resultado-endereco");
    const painelVeiculo = document.getElementById("painel-veiculo");
    let selecaoAtual = null;
    await montarSeletorVeiculo((selecaoRecebida) => {
      selecaoAtual = selecaoRecebida;
      painelVeiculo.innerHTML = dadosVeiculoHtml(selecaoRecebida.veiculo);
      areaResultado.innerHTML = "";
    });
    formularioEndereco.addEventListener("submit", async (eventoEnvio) => {
      eventoEnvio.preventDefault();
      const dadosFormulario = new FormData(formularioEndereco);
      const resultadoOperacao = await chamarApi("/api/endereco", {
        uf: selecaoAtual.uf,
        placa: selecaoAtual.veiculo.placa,
        uf_destino: dadosFormulario.get("uf_destino"),
        cep_destino: dadosFormulario.get("cep_destino"),
        caixa_postal: dadosFormulario.get("caixa_postal") === "on",
      });
      areaResultado.innerHTML = `<p class="resultado ${resultadoOperacao.sucesso ? "resultado--limpo" : "resultado--erro"}">${escaparHtml(resultadoOperacao.mensagem)}</p>`;
      if (resultadoOperacao.sucesso) {
        painelVeiculo.innerHTML = dadosVeiculoHtml(resultadoOperacao.dados.veiculo);
        notificar("Endereço atualizado", resultadoOperacao.mensagem, "sucesso");
      } else {
        notificar("Alteração recusada", resultadoOperacao.mensagem, "erro");
      }
    });
  },

  async crlve() {
    const botaoEmitir = document.getElementById("botao-emitir");
    const campoExercicio = document.getElementById("campo-exercicio");
    const areaResultado = document.getElementById("resultado-emissao");
    const painelPrototipo = document.getElementById("painel-prototipo");
    let selecaoAtual = null;

    const atualizarPainelPrototipo = async () => {
      const estadoAtual = await chamarApi("/api/prototipo");
      const prototipo = estadoAtual.prototipo;
      const linhasDocumentos = estadoAtual.documentos
        .map((documentoEmitido) => `<tr><td>${escaparHtml(documentoEmitido.placa)}</td><td>${documentoEmitido.exercicio}</td><td><code>${documentoEmitido.id}</code></td></tr>`)
        .join("");
      painelPrototipo.innerHTML = `
        <p><strong>Protótipo</strong> · id <code>${prototipo.id}</code> · placa: <code>${prototipo.placa ?? "vazia"}</code> · RENAVAM: <code>${prototipo.renavam ?? "vazio"}</code></p>
        ${estadoAtual.documentos.length
          ? `<table><thead><tr><th>Placa</th><th>Exercício</th><th>id do clone</th></tr></thead><tbody>${linhasDocumentos}</tbody></table>
             <p class="resultado ${estadoAtual.todos_independentes ? "resultado--limpo" : "resultado--erro"}">
               ${estadoAtual.todos_independentes ? "Todos os documentos são objetos distintos entre si e do protótipo." : "Há objetos repetidos."}</p>`
          : `<p class="vazio">Nenhum documento emitido ainda.</p>`}`;
    };

    await montarSeletorVeiculo((selecaoRecebida) => {
      selecaoAtual = selecaoRecebida;
      areaResultado.innerHTML = "";
    });
    await atualizarPainelPrototipo();

    botaoEmitir.addEventListener("click", async () => {
      botaoEmitir.disabled = true;
      try {
        const resultadoEmissao = await chamarApi("/api/crlve", {
          uf: selecaoAtual.uf,
          placa: selecaoAtual.veiculo.placa,
          exercicio: Number(campoExercicio.value),
        });
        if (resultadoEmissao.sucesso) {
          areaResultado.innerHTML = documentoHtml(resultadoEmissao.dados.documento);
          notificar("CRLV-e emitido", `Documento de ${selecaoAtual.veiculo.placa} criado por clonagem do protótipo.`, "sucesso");
        } else {
          const pendencias = resultadoEmissao.dados?.pendencias || [];
          areaResultado.innerHTML = `
            <div class="resultado resultado--erro"><h3>${escaparHtml(resultadoEmissao.mensagem)}</h3>${listaHtml(pendencias)}
              <p>Nenhum clone foi criado. <a href="consultas.html">Ver consultas</a></p></div>`;
          notificar("Emissão bloqueada", `${pendencias.length} pendência(s) no Nada Consta.`, "erro");
        }
        await atualizarPainelPrototipo();
      } finally {
        botaoEmitir.disabled = false;
      }
    });
  },
};

document.addEventListener("DOMContentLoaded", () => {
  const nomeTela = document.body.dataset.pagina;
  document.querySelectorAll(".navegacao a").forEach((linkNavegacao) => {
    if (linkNavegacao.dataset.pagina === nomeTela) linkNavegacao.setAttribute("aria-current", "page");
  });
  document.querySelectorAll(".fluxo li").forEach((passoFluxo) => {
    if (passoFluxo.dataset.pagina === nomeTela) passoFluxo.classList.add("atual");
  });
  if (TELAS[nomeTela]) TELAS[nomeTela]();
});
