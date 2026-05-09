# Webserv — TODO da dupla (André & Jefferson)

> Estado real do código x requisitos do subject (24.0).
> Atribuição feita a partir do `git log` (autor de cada commit), **não** da
> divisão original do `webserv_divisao_projeto.pdf` — a dupla redividiu as
> tarefas no meio do projeto. 

Legenda: ✅ feito · ⏳ em andamento · ❌ falta · ⚠️ a verificar

---

## Visão geral por área

| Área | Status | Quem fez (commits) |
|---|---|---|
| Fundação do projeto (Makefile, structs base) | ✅ | André — `d609827` |
| Tokenizer + ConfigParser (base) | ✅ | André — `d609827` |
| `HttpRequest` struct | ✅ | Jefferson — `067dccf` |
| `HttpRequestParser` | ✅ | Jefferson — `5d6a59b`, `6cfda8b`, `cdeeba7`, `e42e482` · **validação 400/405/505** André — `c709bad` |
| `HttpResponse` (struct, MIME, reasonPhrase, setHeader/Body, build, location) | ✅ | Jefferson — `055e53a`, `0595d4b`, `4ee8eb7`, `f99c615`, `8b34f2e`, `0f83969`, `042af2d`, `d59fd85` |
| Leitura de request no socket (`read` + buffer) | ✅ | André — `2b2e3b6`, `13900cc` |
| Router (longest-prefix match, GET, segurança path traversal) | ✅ | André — `1437f47`, `13900cc`, `11c5607` |
| POST simples (escrita direta de arquivo) | ✅ → substituído | André — `b2ddca8`, `13d6200` |
| POST multipart / upload de arquivos | ✅ | André — `5808bd3` |
| DELETE | ✅ | Jefferson — `b1034b0` |
| Integração ConfigParser ↔ Router | ✅ | Jefferson — `b1034b0` |
| Múltiplos `server` blocks com `poll()` | ✅ | Jefferson — `1df6cb6` |
| `client_max_body_size` + `error_page` (config + Router) | ✅ | Jefferson — `8a61c22` |
| Autoindex real (listagem de diretório) | ✅ | Jefferson — `b5141dc` |
| `return <code> <url>` (redirect) | ✅ | Jefferson — `a69175d`, `1bf772c`, `042af2d`, `c1a6d76`, `efe65e6` |
| `fix lib <stdio.h>` | ✅ | André — `4c469e2` |
| Limpeza (remoção de classe `Socket` morta + unify `HttpResponse`) | ✅ | Jefferson — `69ab058`, `d59fd85` |
| Build separado de testes (`Makefile.test`, `test_parser`) | ✅ | Jefferson — `bb83dcd`, `31dd67a`, `b3656d3` |
| README.md (cap. V do subject) | ✅ | Jefferson — `7e81e2a` |
| CGI (`fork` + `execve` + `pipe`) | ❌ | Jefferson (divisão original) |
| `root` por location | ❌ | — |
| Diretiva `upload_store` (path configurável de upload por rota) | ❌ | — |
| Timeout de conexão no `poll()` | ❌ | — |
| Tratamento de `POLLHUP` / `POLLERR` | ⚠️ | — |
| Conformidade `errno` após I/O | ⚠️ | — |
| Testes de integração (bash/curl/nc) | ✅ | Jefferson — `2d264be`, `f00e5ea`, `037f0f2`, `c6e1a3c`, `84dbf21`, `658694f`, `0388967`, `55e46a1` |

---

## TODO — André

### ❌ Falta

- [ ] **Timeout de conexão no `poll()`**
  Hoje em [`Server.cpp:147`](../src/server/Server.cpp#L147) o timeout é `-1` (espera infinita). Adicionar timeout (~30s) e fechar conexões ociosas. Subject proíbe travas indefinidas.

- [ ] **Tratar `POLLHUP` / `POLLERR`**
  No loop atual ([`Server.cpp:153-230`](../src/server/Server.cpp#L153-L230)) só `POLLIN` e `POLLOUT` são checados. Confirmar que cliente desconectado é removido limpo (hoje cai no `else { removeFd }` mas verificar).

- [ ] **Conformidade `errno` pós-I/O**
  Subject proíbe inspecionar `errno` após `read`/`write` para decidir comportamento. Auditar `Server.cpp` (uso de `perror` é OK; usar valor de `errno` para lógica não é).

### ✅ Já entregue

- Fundação do projeto: Makefile, `Config`/`ServerConfig`/`RouteConfig`, Tokenizer, ConfigParser base — `d609827`.
- Server TCP (`socket`/`bind`/`listen`/`accept`), leitura de request, parser inicial e Router — `2b2e3b6`, `13900cc`, `1437f47`, `11c5607`.
- POST básico → multipart/upload de arquivos — `b2ddca8`, `13d6200`, `5808bd3`.
- Validações HTTP `400 / 405 / 505` no parser — `c709bad`.
- Fix `<stdio.h>` ausente — `4c469e2`.

---

## TODO — Jefferson

### ❌ Falta

- [ ] **CGI** — requisito obrigatório do subject (era seu na divisão original do PDF).
  Implementar:
  - Diretiva `cgi_extension .py /usr/bin/python3` no bloco `location`.
  - Campo `std::map<std::string,std::string> cgi_extensions` em `RouteConfig`.
  - Classe `CgiHandler`: detecta extensão no path, monta envvars (`REQUEST_METHOD`, `PATH_INFO`, `QUERY_STRING`, `CONTENT_LENGTH`, `CONTENT_TYPE`, `SERVER_NAME`, `SERVER_PORT`, `SCRIPT_FILENAME`, `REDIRECT_STATUS`).
  - Execução via `fork` + `execve` + `pipe` para stdin/stdout. `chdir` para o diretório do script antes do `execve`.
  - Parse da saída do CGI (headers + body) → `HttpResponse`.
  - Integração com event loop do `Server` (síncrono com timeout, ou assíncrono).

- [ ] **`root` por `location`** (o exemplo `/kapouet` do subject)
  - Adicionar `std::string root` em [`RouteConfig.hpp`](../src/includes/RouteConfig.hpp).
  - Parsear `root` dentro de blocos `location` em [`ConfigParser::parseRoute`](../src/config/ConfigParser.cpp#L124).
  - Em [`Router::handleRequest`](../src/http/Router.cpp#L115), usar `route->root` quando definido (com fallback para `config.root`).

- [ ] **Diretiva `upload_store` por rota**
  - Adicionar `std::string upload_store` em [`RouteConfig.hpp`](../src/includes/RouteConfig.hpp).
  - Parsear `upload_store <path>;` em `ConfigParser::parseRoute`.
  - No handler de POST/multipart ([`Router.cpp:241`](../src/http/Router.cpp#L241)), usar `route->upload_store` em vez de `config.root`.

- [ ] **Testes de integração — CGI**
  Pendente até CGI estar implementado. Cobrir execução de script, env vars,
  parsing da saída, timeout. Adicionar como `tests/test_cgi.sh`.

### ✅ Já entregue

- `HttpRequest` + `HttpRequestParser` (parse de request line, headers, body, fragmentação TCP) — `067dccf`, `5d6a59b`, `6cfda8b`, `cdeeba7`, `e42e482`.
- `HttpResponse` completo (struct, `reasonPhraseFor`, `mimeTypeFor`, `setHeader`/`setBody`, status, `build`/`toString`, header `Location`, unificação) — `055e53a`, `0595d4b`, `4ee8eb7`, `f99c615`, `8b34f2e`, `0f83969`, `042af2d`, `d59fd85`.
- Integração ConfigParser ↔ Router + DELETE — `b1034b0`.
- Múltiplos servers com `poll()` — `1df6cb6`.
- `client_max_body_size` + `error_page` (config e enforcement no Router) — `8a61c22`, `de61b66`.
- Autoindex real (listagem de diretório) — `b5141dc`.
- Redirect (`return <code> <url>` no config + header `Location` na resposta) — `a69175d`, `1bf772c`, `042af2d`, `c1a6d76`, `efe65e6`.
- README.md conforme cap. V do subject — `7e81e2a`.
- Build separado de testes unitários (`Makefile.test`, `test_parser`) — `bb83dcd`, `31dd67a`, `b3656d3`.
- Suite de integração modular em `tests/` (GET, POST, DELETE, redirect, errors, autoindex, security, concurrent) — `2d264be`, `f00e5ea`, `037f0f2`, `c6e1a3c`, `84dbf21`, `658694f`, `0388967`, `55e46a1`.
- Limpeza de código morto (classe `Socket`, `HttpResponse` legado) — `69ab058`, `d59fd85`.

---

## Compartilhado / Ambos

- [ ] **Comparação com NGINX** — testar headers e comportamento com `curl -v` e `telnet`, lado a lado com NGINX. Documentar divergências.
- [ ] **Auditoria final do subject** — reler `webserv.pdf` antes da entrega, em particular:
  - Não pode haver `errno` pós-I/O.
  - Nenhuma operação `read`/`write`/`accept` fora de `poll()`.
  - Servidor nunca pode travar indefinidamente.

---

## Observações sobre a divisão

- A divisão original (`webserv_divisao_projeto.pdf`) colocava:
  - **Pessoa A (infra/sockets/poll):** seria André em ideia, mas na prática quem fechou `poll()` com múltiplos servers, autoindex, error_pages, redirect, integração ConfigParser↔Router e cleanups foi **Jefferson**.
  - **Pessoa B (parser HTTP, response, GET/POST/DELETE/upload/CGI):** era Jefferson, mas **André** acabou implementando GET, POST, multipart/upload, validações HTTP no parser e parte da segurança do Router.
- Resultado: a dupla cruzou responsabilidades, mas **CGI** continua com o Jefferson conforme a divisão original do PDF — é o maior gap obrigatório e ninguém começou ainda.
