# Auditoria Final — Webserv

**Data:** 2026-05-19
**Reviewer:** Independente (nao-autor)
**Veredito:** APROVADO COM RESSALVAS

Em uma frase: o projeto compila limpo, atende todos os requisitos hard do subject 24.0
e passa 77/77 testes de integracao. Existe **um ponto sensivel** (CGI sincrono que
bloqueia o loop principal ate o timeout de 5s) e **algumas higienes** (arquivo morto
no repo, Makefile.test quebrado, header CGI/1.1 imperfeito) que o defendente deve
saber explicar. Nada bloqueia a nota minima, mas o ponto do CGI pode custar nota
extra dependendo do avaliador.

---

## Conformidade do subject

### Compilacao

| Requisito | Status | Evidencia |
|---|---|---|
| C++98 (`-std=c++98`) | OK | `Makefile:7` |
| `-Wall -Wextra -Werror` | OK | `Makefile:7` |
| Sem warnings/errors | OK | `make re` em WSL Ubuntu — saida 100% limpa |
| Regras `all`, `clean`, `fclean`, `re` | OK | `Makefile:36-50` |
| Regra `$(NAME)` | OK | `Makefile:38` |
| Nome do binario = `webserv` | OK | `Makefile:4` |
| Sem religacao desnecessaria | OK | 2o `make` apos build: `Nothing to be done for 'all'.` |
| Sem dependencia externa | OK | Compila com `c++` standalone |

### Runtime

| Requisito | Status | Evidencia |
|---|---|---|
| `./webserv [config]` ou default | OK | `src/main.cpp:18` (`argv[1]` ou `"config.conf"`) |
| Servidor nao bloqueante | RESSALVA | OK para I/O HTTP comum; **CGI bloqueia o loop** ate o timeout (ver Bloqueadores/Ressalvas) |
| Apenas 1 `poll()` no I/O do servidor | OK | `src/server/Server.cpp:196` — unico poll no loop principal; o `poll()` em `CgiHandler.cpp:322` e do subprocesso CGI |
| `poll()` monitora R+W simultaneamente | OK | `Server.cpp:262` — quando resposta esta pronta: `fds[i].events = POLLIN \| POLLOUT` |
| Nenhum read/write sem `poll()` previo | OK | `Server.cpp:248,267` so executam dentro de branches `revents & POLLIN/POLLOUT`; `CgiHandler.cpp:328,346` idem |
| `errno` nunca consultado pos-I/O | OK | Apenas em comentarios (`Server.cpp:160,199`) — nunca lido |
| Idle timeout | OK | `Server.cpp:290-302` — 30s; `CLIENT_TIMEOUT_S=30` |
| Servidor nao crasha | OK | Testes de resiliencia (truncated, malformed, 5MB > limite, header gigante) — server segue de pe em todos |
| Codigos HTTP precisos | OK | 200/201/204/301/302/400/403/404/405/413/500/502/504/505 cobertos por suite |
| Paginas de erro default | OK | `HttpError.cpp:28-30` — fallback inline quando `error_page` nao existe |
| `fork()` so em CGI | OK | grep confirma: unico `fork(` em `CgiHandler.cpp:245` |
| `fcntl` so com `F_SETFL`/`O_NONBLOCK` | OK | Tres usos, todos `F_SETFL, O_NONBLOCK` (`Server.cpp:49`, `CgiHandler.cpp:288-289`) |

### Funcoes externas

Lista oficial autorizada toda usada (e somente ela):
`socket, bind, listen, accept, setsockopt, send, recv, read, write, close, poll, signal, fork, execve, pipe, dup2, chdir, kill, waitpid, fcntl, stat, opendir, readdir, closedir, htons, htonl`.

Funcoes C stdlib usadas (tradicionalmente aceitas pelos avaliadores 42):
`std::sscanf, std::memset, std::memcpy, std::atoi, std::time, std::isdigit, std::isspace, std::tolower, std::toupper, std::isspace, strtol, _exit`.

**Atencao — fora da lista oficial, possivelmente cobrado:**
- `std::remove` (de `<cstdio>`) em `src/http/Router.cpp:159` para o DELETE.
  Avaliadores geralmente aceitam (e equivalente a `unlink`, e nem `unlink` esta
  na lista). Tenha a justificativa pronta.

### Config

| Recurso | Status | Onde |
|---|---|---|
| Multiplos pares interface:porta | OK | `config.conf` tem dois `server` blocks (8080 e 8081); `Server.cpp:167` itera |
| `error_page` | OK | `ConfigParser.cpp:103-110`; consumido em `HttpError.cpp` |
| `client_max_body_size` (com K/M) | OK | `ConfigParser.cpp:21-29` (sufixos K/M); aplicado em `Router.cpp:126` |
| Por rota: metodos | OK | `RouteConfig.methods`, `Router.cpp:92-100` |
| Por rota: redirect HTTP | OK | `return <code> <url>;`, `Router.cpp:111-117` |
| Por rota: `root` | OK | `RouteConfig.root`, `Router.cpp:143` |
| Por rota: autoindex on/off | OK | `Router.cpp:284` |
| Por rota: index default | OK | `Router.cpp:271` |
| Por rota: `upload_store` | OK | `Router.cpp:237-242` |
| Por rota: `cgi_extension` | OK | `CgiHandler::matchCgi`, `Router.cpp:151-156` |

### HTTP

| Recurso | Status | Onde |
|---|---|---|
| GET | OK | `Router.cpp:268-310` |
| POST | OK | `Router.cpp:170-266` (multipart/form-data) |
| DELETE | OK | `Router.cpp:158-168` |
| Upload de arquivos | OK | multipart parser em `Router.cpp:190-253` + sanitizacao em `Router.cpp:25-42` |
| CGI (>=1 interpretador) | OK | Python via `cgi_extension .py /usr/bin/python3` |
| CGI/1.1 env vars completas | RESSALVA | Falta `REMOTE_ADDR/REMOTE_HOST`. `PATH_INFO` esta **incorreto** (igual ao SCRIPT_NAME). Detalhe nao costuma ser cobrado, mas e uma imprecisao. |
| EOF marca fim de body do CGI | OK | `CgiHandler.cpp:344-357` le ate POLLHUP/n<=0; sem dependencia de Content-Length |
| `chdir` ao diretorio do script | OK | `CgiHandler.cpp:270` (no child antes do execve) |

### README (cap. V do subject)

| Item | Status | Linha |
|---|---|---|
| Primeira linha italicizada com login(s) e curriculo 42 | OK | `README.md:1` |
| Secao "Description" | OK | `README.md:5` |
| Secao "Instructions" (build + run) | OK | `README.md:30` |
| Secao "Resources" com referencias | OK | `README.md:94-101` |
| Descricao do uso de IA | OK | `README.md:102-110` — diz para quais partes (poll loop + CGI) e como foi revisado |

---

## Bloqueadores (NOTA = 0 se nao corrigidos)

**Nenhum bloqueador absoluto identificado.** O projeto compila, atende a check-list
do subject, passa a suite, sobrevive a clientes hostis e nao crasha.

---

## Ressalvas (defensaveis, mas podem custar pontos)

### R1. CGI bloqueia o loop principal ate o timeout (5s)
**Severidade: alta — pode aparecer na defense.**

`Router::handleRequest` chama `CgiHandler::execute()` **sincronamente** dentro do
loop de eventos do `Server::run()`. Toda a logica de `pipe + fork + poll do filho +
waitpid` mora em `execute()` e **so retorna ao loop principal quando o CGI termina
ou expira (5s)**.

Medi empiricamente:
- Disparei `/cgi-bin/slow.py` (sleep 10).
- Imediatamente em paralelo: `GET /` (estatico trivial).
- O GET levou **4.82 s** para responder (a duracao do timeout do CGI).

Reproducao (script ja removido apos auditoria):
```bash
curl /cgi-bin/slow.py &        # dispara CGI
sleep 0.3
time curl /                    # outro cliente trivial -> ~4.8s
```

O subject diz literalmente: *"Your server must be non-blocking and use only one
[...] poll()"* e *"Server should never block and the client can be bounced
properly if necessary."* A interpretacao estrita reprova; a interpretacao
benevolente (que e o que a maioria dos webserv 42 entrega) aceita, porque o
timeout do servidor (5s) garante que o loop nao trava indefinidamente.

**Como defender**: argumentar que (a) ha timeout duro (5s) que impede travar
indefinidamente, (b) `waitpid(WNOHANG)` integrado ao poll loop principal foi um
trade-off de complexidade vs. risco que mantemos por simplicidade. **Se o avaliador
nao aceitar, e perda de nota.**

### R2. PATH_INFO incorreto no CGI/1.1
**Severidade: baixa.**

`CgiHandler.cpp:114-116`:
```cpp
env["SCRIPT_NAME"] = _request.path;
env["PATH_INFO"]   = _request.path;
```
RFC 3875: `PATH_INFO` deveria ser o trecho do path **apos** o script (ex:
`/cgi-bin/foo.py/extra` -> `PATH_INFO=/extra`, `SCRIPT_NAME=/cgi-bin/foo.py`).
Aqui sao iguais. Avaliadores raramente testam isso; o suite passa porque
nenhum script depende de `PATH_INFO`.

### R3. CGI sem `REMOTE_ADDR/REMOTE_HOST`
**Severidade: muito baixa.** Subject pede ambiente "completo". RFC 3875 lista
`REMOTE_ADDR` como obrigatorio. Nao quebra os tests, raramente cobrado.

### R4. `serverIdx` desalinhado se algum bind falhar
**Severidade: muito baixa (cenario quase impossivel em defense).**

Em `Server.cpp:167-184`: o loop pula `_config.servers[i]` quando `createSocket`
falha (`continue`). Resultado: `serverFds` pode ser menor que `_config.servers`,
e o indice retornado por `isServerFd` (`Server.cpp:118-126`) e o **indice em
`serverFds`**, nao em `_config.servers`. Quando o cliente entra, salvamos esse
indice em `ClientState.serverIdx`, e mais tarde indexamos `_config.servers[
st.serverIdx]` (`Server.cpp:258`) — o que aponta para o server config errado se
algum bind falhou.

Reprodutivel apenas se duas portas no `config.conf` tem conflito real.
Nao reprovavel em condicoes normais, mas e um bug latente.

### R5. DELETE devolve sempre 404 quando `std::remove` falha
**Severidade: baixa.** `Router.cpp:158-168`: `remove` pode falhar por permissao
(deveria ser 403) ou por path ser diretorio (poderia ser 409). Sempre vira 404.
Suite passa porque so testa "arquivo existe" e "arquivo nao existe".

### R6. `waitpid` bloqueante no fim do CGI
**Severidade: muito baixa.** `CgiHandler.cpp:367`: `waitpid(pid, &status, 0)`.
Apos `kill SIGKILL` o filho some quase instantaneamente, mas formalmente e mais
uma chamada bloqueante que mora dentro do loop principal. Aceito pela mesma
logica de "ha timeout duro".

### R7. `std::remove` fora da lista oficial
**Severidade: baixa.** Ver secao "Funcoes externas". Tenha justificativa.

### R8. Codigo morto / lixo no repositorio
**Severidade: baixa (higiene).**

- `src/http/HttpRequest.cpp` define `void parseRequest(...)` que **nao e chamada
  em lugar algum** (Server.cpp usa `HttpRequestParser`). Arquivo entra no build
  mas a funcao e linkavel-mas-orfa.
- `src/http/test_parser.cpp` **nao compila** com o `HttpRequestParser` atual
  (chama `parser.parse(raw)` em uma assinatura que nao existe — a real e
  `parse(raw, req)` com 2 argumentos). `Makefile.test` referencia esse arquivo.
  Reproducao: `make -f Makefile.test` -> erros.
- `webserv_test` (binario antigo de 284 KB) presente no disco mas listado no
  `.gitignore`. Nao vai ao commit.

Sugestoes: remover `test_parser.cpp`, remover `Makefile.test`, ou consertar.
Remover `HttpRequest.cpp` da build (a funcao `parseRequest` nao e usada) ou
deletar o arquivo.

### R9. `signal(SIGPIPE, SIG_IGN)` duplicado
**Severidade: trivial.** Chamado em `Server.cpp:161` e novamente em
`CgiHandler.cpp:226`. Inofensivo.

### R10. `error_page` aponta para arquivos que nao existem
**Severidade: trivial.** `config.conf:6-8` lista `/errors/404.html` etc, mas
`www/errors/` nao existe. Cai no fallback inline e os testes esperam exatamente
isso. Eh **intencional** (test_errors testa o fallback), mas um avaliador
desavisado pode achar que e bug. Documentar ou criar os arquivos resolveria.

### R11. Tokenizer nao suporta comentarios nem strings com espaco
**Severidade: trivial.** `#`, `//`, `"foo bar"` nao sao reconhecidos. Aceitavel
para o subject mas pode ser cobrado se o avaliador testar.

### R12. Parser de IPv4 manual nao suporta `localhost` nem `0.0.0.0`
**Severidade: trivial.** `parseIPv4` so aceita `A.B.C.D`. Para qualquer outra
coisa cai em `INADDR_ANY`. Config padrao usa `127.0.0.1` entao funciona.
`host localhost;` daria `INADDR_ANY` silenciosamente.

---

## Achados por arquivo

### `Makefile`
- Compilador `c++` com flags corretas (`Makefile:6-7`).
- Regra `%.o: %.cpp` usa `-I src/includes` no compile, mas link nao usa
  (irrelevante).
- `.PHONY` declarado para `all clean fclean re` (`Makefile:56`).
- Sem regra `bonus` (nao requerido).
- **Sugestao**: adicionar `MAKEFLAGS += --no-print-directory` e `-MMD -MP` para
  detectar mudancas de header (nao requerido pelo subject).

### `Makefile.test`
- **QUEBRADO**. Compila `src/http/test_parser.cpp` que esta dessincronizado com
  a API atual do parser. Ver R8.

### `config.conf`
- Demonstra todos os recursos exigidos: 2 servers, error_page, client_max_body_size,
  metodos por rota, redirect, autoindex, upload_store, root por rota, cgi_extension.
- `error_page` aponta para arquivos inexistentes. Ver R10.

### `src/main.cpp`
- Try/catch ao redor do parse, retorna 1 em erro. OK.
- `argc > 1 ? argv[1] : "config.conf"` — exatamente como pede o subject.
- Sem header `By: jtertuli` (o arquivo e do co-autor `andre`).

### `src/server/Server.cpp`
- Core do poll loop. **Bem comentado em ingles tecnico**, explica decisoes do
  subject (`Server.cpp:158-161`, `194-200`, `259-263`, `287-289`).
- `BUFFER_SIZE=4096` no read — razoavel.
- `POLL_TIMEOUT_MS=1000` permite varrer idle clients a cada segundo.
- `setNonBlocking` aplica somente em socket pos-accept e pos-listen. OK.
- `parseIPv4` manual (subject nao autoriza `inet_addr`). Bom.
- `SO_REUSEADDR` ligado em todos os listening sockets. OK.
- `signal(SIGPIPE, SIG_IGN)` no inicio do `run()`. OK.
- `poll` so e chamado uma vez por iteracao do loop. OK.
- Tratamento de POLLERR/POLLNVAL/POLLHUP para clientes. OK.
- Server fds nao recebem POLLOUT, nao saem do loop. OK.
- Idle scan no fim de cada iteracao. OK.
- Ressalva R4 sobre indice serverIdx.

### `src/http/Router.cpp`
- `findRoute`: longest-prefix match. OK.
- `sanitizeFilename`: remove `/` e `\` do filename. Aceita `..hack.txt` literal
  (sem `..` separando) — verifica `..` no path do request em `Router.cpp:136`
  antes do upload, entao path traversal no PATH e bloqueado; no FILENAME (parte
  multipart) tambem (suite confirma).
- `urlDecode`: decoda `%XX` mas **decoda DEPOIS** de checar `..`. Espera: na
  verdade ele decoda PRIMEIRO (`Router.cpp:107`) e depois testa `..`
  (`Router.cpp:136`). OK, certo.
- POST sem multipart -> 400 (OK).
- `client_max_body_size` aplicado **depois** do parse — body ja foi totalmente
  recebido em memoria. Isso e aceitavel mas significa que rejeitar 1GB exige
  ter consumido 1GB no buffer. Subject nao exige rejeicao early.
- Multipart parser eh ingenuo (linhas 190-253). Funciona para casos normais,
  pode quebrar em multipart sem boundary fechado.
- DELETE -> R5.

### `src/http/CgiHandler.cpp`
- Estrutura `pipe -> fork -> chdir+execve -> poll(parent)` — correta.
- envp/argv allocados com `new char[]`, liberados em todos os paths do parent
  com `freeStrArray`. Auditei: SEM LEAK no caminho de erro do parent.
- No child apos `chdir`, `delete[] argv[1]` e `dupStr(base)` — re-aloca o
  basename para usar como argv1 do interpreter. OK.
- `_parseOutput` aceita `\r\n\r\n` E `\n\n` como separador headers/body. Bom.
- `Status: <code>` parseado. `Location:` sem Status promove para 302. Bom.
- `Content-Length` e `Connection` do CGI sao filtrados (nao re-emite). Bom — o
  `HttpResponse::build` sempre gera Content-Length proprio.
- Timeout 5s -> 504. Script com exit!=0 -> 502. Sinal -> 502. Output sem
  separador -> 502. Bom.
- Ressalvas R1 (bloqueante), R2 (PATH_INFO), R3 (REMOTE_ADDR), R6 (waitpid).

### `src/http/HttpRequestParser.cpp`
- Parse de request line: 3 tokens, valida HTTP/1.0 e HTTP/1.1.
- Headers para lowercase (case-insensitive lookup). Bom.
- Detecta `multipart/form-data` e extrai boundary. OK.
- `Content-Length` valida que so tem digitos. Bom.
- HTTP/1.1 exige header `Host:`. OK.
- **Nao limita** tamanho de header. Server gigante consegue causar uso de
  memoria, mas o `client_max_body_size` e isolado do tamanho da request line/headers.
  Subject nao exige limite para headers. Aceitavel.

### `src/http/HttpRequest.cpp`
- Define `parseRequest(...)` que **nao e usada em lugar nenhum**. Codigo morto.
  Ver R8.

### `src/http/HttpResponse.cpp`
- `build()` sempre escreve `Connection: close`. Bom (server nao mantem keep-alive).
- Calcula `Content-Length` a partir do body (correto).
- `reasonPhraseFor` cobre os codigos usados pelo sistema. OK.
- `mimeTypeFor` cobre as extensoes principais. OK.

### `src/http/HttpError.cpp`
- Tenta servir o arquivo de `error_pages`; se falhar (vazio ou nao existe),
  fallback inline `<h1>NNN reason</h1>`. OK.

### `src/config/ConfigParser.cpp`
- Recursive descent simples. Throws em todos os erros — `main.cpp` captura.
- `parseSize` interpreta sufixos K/M (`ConfigParser.cpp:21-29`). OK.
- Nao valida que `cgi_extension` comeca com ponto. Nao reprova.
- Aceita config vazio? Loop principal nao entra; main checa `config.servers.empty()`
  e sai. OK.

### `src/config/Tokenizer.cpp`
- Splits em whitespace e isola `{ } ;` como tokens. OK.
- `isspace(char)` com char assinado e UB em chars >= 128 — code smell trivial.
  Ver R11.

### `src/utils/File.cpp`
- `readFile`/`writeFile`. `<fstream>` (autorizado, e C++ stdlib).
- `writeFile` retorna `false` se nao abriu — mas nao retorna false se o write
  falhou no meio (stream errors). Code smell pequeno; suite passa porque
  os destinos sao /tmp/uploads owned by user.

### Headers `src/includes/*.hpp`
- Todos usam `#pragma once`. OK (apesar de `#ifndef` ser tradicional, `pragma once`
  e amplamente aceito).
- `Config.hpp`, `ServerConfig.hpp`, `RouteConfig.hpp`: structs simples. OK.
- `HttpRequest.hpp` tem campo `bool valid` que nao e inicializado nem usado.
  Limpeza recomendada.

### `tests/`
- 9 suites + lib.sh + run_all.sh. **Todos os 77 asserts passaram** nesta auditoria.
- Cobertura razoavelmente boa. Falta um teste explicito para "varios servers
  config com mesma porta" (cenario de erro) e "config invalido".
- `tests/README.md` documenta como rodar.

---

## Testes executados (em WSL Ubuntu)

### Compilacao
- `make fclean && make` -> **OK, zero warnings**, todos os 10 .o + binario.
- `make` (re-run) -> `Nothing to be done` (sem relinkage).
- `make -f Makefile.test` -> **FALHA** (erros de compilacao em test_parser.cpp). Ver R8.

### Suite automatizada (`tests/_run_with_server.sh`)
- AUTOINDEX: 9/9 OK
- CGI: 12/12 OK
- CONCURRENT: 6/6 OK
- DELETE: 5/5 OK
- ERRORS: 9/9 OK
- GET: 14/14 OK
- POST: 11/11 OK
- REDIRECT: 6/6 OK
- SECURITY: 5/5 OK
- **Total: 77/77 OK, 0 falhas, todas as 9 suites verdes.**

### Resiliencia manual (script `_audit_resilience.sh`, ja removido)
- Truncated request `GET /` + sleep -> servidor segue: `after-truncated=200`.
- Header malformado (linha sem `:`) -> 400, servidor segue: `after-malformed=200`.
- POST 5MB com limite 1M -> 413, servidor segue: `after-big=200`.
- Garbage binario `\x00\x01..` -> 400, servidor segue: `after-garbage=200`.
- Header value de 16KB -> 200, servidor segue: `after-bigheader=200`.

### Sinais
- SIGINT em terminal interativo (job control on) **mata o servidor** (sinal default).
- SIGINT herdado via shell em background fica como SIG_IGN — isso e comportamento
  padrao do bash (nao do servidor). SIGTERM mata em qualquer caso.

### CGI bloqueante (medicao especifica)
- `slow.py` (sleep 10) com timeout do server 5s + `GET /` paralelo:
  - GET paralelo demorou **4.82s**. Confirma R1.

---

## Lista compacta de TODO antes da defense (em ordem de prioridade)

1. **Saber explicar R1** — "por que CGI bloqueia o loop principal? como vc defenderia se
   o avaliador pegar?" Resposta sugerida: ha timeout duro de 5s; em escala real seria
   movido para `waitpid(WNOHANG)` integrado ao poll loop principal.
2. **Remover `src/http/test_parser.cpp` e `Makefile.test`** (ou consertar). Sao lixo
   que aparece em `ls` e podem confundir o avaliador.
3. **Remover `src/http/HttpRequest.cpp` da build** (a funcao `parseRequest` nao e
   usada). Ou deletar o arquivo e remover do Makefile.
4. **Considerar tirar `webserv_test` do disco** (ja esta em `.gitignore`, so esta
   confundindo localmente).
5. (Opcional) Criar `www/errors/404.html`, `403.html`, `413.html` para que o
   `error_page` configurado seja efetivo, ou remover do `config.conf` esses
   error_pages se forem so demonstrativos.
6. (Opcional) Corrigir `PATH_INFO` no CGI para o trecho pos-script.

---

## Conclusao

Pode ir para a defense. Saiba explicar a R1 (CGI sincrono) em voz alta —
e o unico ponto onde um avaliador estrito pode pegar e tirar nota. Os outros
itens sao higiene/defensiveis. Compilacao limpa, testes 100%, resiliencia
confirmada, funcoes externas dentro da lista (so `std::remove` fora, defensavel).
