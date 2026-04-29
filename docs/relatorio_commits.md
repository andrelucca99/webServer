# Relatório de Histórico de Commits — webServer

Gerado em: 2026-04-06  
Finalidade: Documentar o estado de cada commit antes da reescrita do histórico para remover referência ao Co-Authored-By indesejado.

---

## Commit 1 — `d60982706ad287403ce1790f80fafbcdbe5f23eb`

**Autor:** André Lucas <andrelucca99@gmail.com>  
**Data:** Wed Apr 1 19:30:40 2026 -0300  
**Mensagem:** `implementa fundação sólida e desacoplada do projeto`

**Arquivos criados (11 arquivos, +689 linhas):**
- `Makefile` — build system com targets `all`, `re`, `clean`, `fclean`; compila com `-Wall -Wextra -Werror -std=c++98`
- `README.md` — documentação do projeto (~295 linhas)
- `config.conf` — arquivo de configuração de exemplo com bloco `server { listen 8080; ... }`
- `src/config/ConfigParser.cpp` — implementação do parser recursivo descendente
- `src/config/Tokenizer.cpp` — tokenizador que separa `{`, `}`, `;` como tokens próprios
- `src/includes/Config.hpp` — struct raiz com `std::vector<ServerConfig>`
- `src/includes/ConfigParser.hpp` — declaração da classe `ConfigParser` (autor: andre)
- `src/includes/RouteConfig.hpp` — struct com path, métodos, index, autoindex
- `src/includes/ServerConfig.hpp` — struct com port, host, root, `std::vector<RouteConfig>`
- `src/includes/Tokenizer.hpp` — declaração da classe `Tokenizer` (autor: andre)
- `src/main.cpp` — harness de teste: lê `config.conf` e imprime as structs parseadas

---

## Commit 2 — `6690eb53fc7ff936e01524bdf25e5a21ed6fd400`

**Autor:** Jefferson Ferreira <jefferson.ctc.ferreira@outlook.com>  
**Data:** Fri Apr 3 22:10:46 2026 -0300  
**Mensagem:** `docs: adiciona gitignore e roadmap HTTP da Pessoa B`

**Arquivos criados (2 arquivos, +163 linhas):**
- `.gitignore` — ignora binários compilados e arquivos de objetos
- `docs/pessoa_b_roadmap.md` — roadmap de estudo HTTP para a Pessoa B (~158 linhas); inclui estrutura de requisição HTTP, métodos, RFCs de referência e sequência de implementação sugerida

---

## Commit 3 — `af30f159b818bf9092cbf21886c1bebc8bfc975a` ⚠️ COMMIT PROBLEMÁTICO

**Autor:** Jefferson Ferreira <jefferson.ctc.ferreira@outlook.com>  
**Data:** Sat Apr 4 18:26:50 2026 -0300  
**Mensagem:** `docs: adiciona anotações sobre HTTP/1.1 e estrutura de requisição`

**Problema:** Contém linha `Co-Authored-By: Claude Haiku 4.5 <noreply@anthropic.com>` que adiciona o Claude como colaborador no GitHub.

**Alterações feitas neste commit (serão descartadas):**
- `docs/pessoa_b_roadmap.md` — adicionadas 8 linhas de anotações pessoais ao final do arquivo
- `src/includes/ConfigParser.hpp` — reformatação de indentação (sem alteração lógica): `private:` e `public:` passaram a usar indentação extra com os membros também indentados
- `src/includes/Tokenizer.hpp` — mesma reformatação de indentação

---

## Commit 4 — `db34a83384929f630125041db5901d0f984255be`

**Autor:** Jefferson Ferreira <jefferson.ctc.ferreira@outlook.com>  
**Data:** Mon Apr 6 18:46:52 2026 -0300  
**Mensagem:** `feat: adiciona estrutura HttpRequest para representar requisição HTTP`

**Arquivos criados (1 arquivo, +27 linhas):**
- `src/includes/HttpRequest.hpp` — struct `HttpRequest` com campos: `method`, `uri`, `httpVersion`, `headers` (`std::map<string,string>`), `body`

**Nota:** Criado **após** o commit problemático. Será descartado pois o código deve retornar ao estado de `6690eb5`.

---

## Commit 5 — `803d8aa467ef88e19df44ea3a4d414fc544c35c9`

**Autor:** Jefferson Ferreira <jefferson.ctc.ferreira@outlook.com>  
**Data:** Mon Apr 6 18:51:02 2026 -0300  
**Mensagem:** `feat: declara classe HttpRequestParser`

**Arquivos criados (1 arquivo, +26 linhas):**
- `src/includes/HttpRequestParser.hpp` — classe `HttpRequestParser` com método `parse(const std::string& raw)`; inclui `HttpRequest.hpp`

**Nota:** Será descartado pois o código deve retornar ao estado de `6690eb5`.

---

## Commit 6 — `b85b94de43838712abe3694fca87929e4b770558`

**Tipo:** Merge commit  
**Autor:** Jefferson Ferreira <jefferson.ctc.ferreira@outlook.com>  
**Data:** Mon Apr 6 18:55:21 2026 -0300  
**Mensagem:** `Merge branch 'main' of github.com:andrelucca99/webServer`  
**Pais:** `803d8aa` ← `af30f15`

**Efeito:** Trouxe para a branch local as alterações do commit `af30f15` (indentação de headers + adições ao roadmap). Este merge é o motivo pelo qual o commit problemático contaminou o histórico local.

**Será descartado.**

---

## Resultado da Reescrita

| # | Novo hash | Mensagem | Autor |
|---|-----------|----------|-------|
| 1 | (novo)    | `implementa fundação sólida e desacoplada do projeto` | André Lucas |
| 2 | (novo)    | `docs: adiciona gitignore e roadmap HTTP da Pessoa B` | Jefferson Ferreira |

Estado final do código: idêntico ao commit `6690eb5` original, sem nenhuma referência ao commit `af30f15`.
