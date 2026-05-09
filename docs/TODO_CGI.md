# TODO — CGI (Webserv)

Plano granular para implementar o requisito **CGI** do subject (24.0).
Cada item é dimensionado para virar **1 commit** (mesmo padrão de
`upload_store` e `root` por location). Marcar com `✅` ao concluir.

> Responsável: Jefferson (divisão original do PDF).
> Branch: `jefferson` (continuar nela ou abrir `jefferson/cgi`).

Legenda: ✅ feito · ⏳ em andamento · ❌ falta

---

## Fase 1 — Configuração

- [ ] **1. Campo `cgi_extensions` em `RouteConfig`**
  Adicionar `std::map<std::string, std::string> cgi_extensions;` em
  [`src/includes/RouteConfig.hpp`](../src/includes/RouteConfig.hpp).
  Chave = extensão (`.py`), valor = caminho do interpretador (`/usr/bin/python3`).
  Inicializar vazio no construtor (não precisa entrar na lista de inicialização).

- [ ] **2. Parsear diretiva `cgi_extension`**
  Em [`ConfigParser::parseRoute`](../src/config/ConfigParser.cpp#L124), adicionar
  branch para `cgi_extension <ext> <interpreter>;`. Sintaxe esperada:
  ```nginx
  location /cgi-bin {
      methods GET POST;
      cgi_extension .py /usr/bin/python3;
  }
  ```
  Permitir múltiplas linhas para extensões diferentes (`.py`, `.sh`, `.php`).

---

## Fase 2 — `CgiHandler` (boilerplate)

- [ ] **3. Skeleton de `CgiHandler`**
  Criar `src/includes/CgiHandler.hpp` e `src/http/CgiHandler.cpp` com:
  - Construtor recebendo `(const HttpRequest&, const RouteConfig&, const ServerConfig&, const std::string& scriptPath, const std::string& interpreter)`.
  - Método público `HttpResponse execute();`.
  - Cabeçalho `By: jtertuli <jtertuli@student.42sp.org.br>`.
  Adicionar `src/http/CgiHandler.cpp` ao `Makefile` (variável `SRCS`).

- [ ] **4. Helper de detecção por extensão**
  Função estática (em `Router.cpp` ou `CgiHandler.cpp`):
  ```cpp
  // retorna interpretador se path bate com alguma extensão registrada na rota,
  // string vazia caso contrário
  static std::string matchCgi(const RouteConfig& route, const std::string& path);
  ```
  Comparar pelo sufixo do path (antes do `?`, se houver query string).

---

## Fase 3 — Execução do processo CGI

- [ ] **5. Construir env vars CGI**
  Método privado que monta `std::map<std::string, std::string>` com:
  - `REQUEST_METHOD` (GET/POST/DELETE)
  - `PATH_INFO` (path do script)
  - `QUERY_STRING` (parte após `?`, vazia se ausente)
  - `CONTENT_LENGTH` (tamanho do body, vazio se GET)
  - `CONTENT_TYPE` (header Content-Type da request)
  - `SERVER_NAME`, `SERVER_PORT` (do `ServerConfig`)
  - `SCRIPT_FILENAME` (path absoluto do script no disco)
  - `SCRIPT_NAME` (path do script no URI)
  - `REDIRECT_STATUS=200` (necessário para PHP-CGI; benigno em outros)
  - `GATEWAY_INTERFACE=CGI/1.1`
  - `SERVER_PROTOCOL=HTTP/1.1`
  - `HTTP_*` para cada header da request (ex.: `User-Agent` → `HTTP_USER_AGENT`).

- [ ] **6. Converter env map → `char**`**
  Helper `char** buildEnvp(const std::map<std::string,std::string>&)`:
  - Aloca `KEY=VALUE` para cada par.
  - Termina com `NULL`.
  - Helper paralelo `freeEnvp(char**)` para limpeza.
  Idem para `argv` (`[interpreter, script, NULL]`).

- [ ] **7. Pipes + `fork()`**
  Criar dois `pipe()`s — um para stdin do filho, um para stdout.
  Chamar `fork()`. Em caso de erro, retornar `502 Bad Gateway`.

- [ ] **8. Branch filho**
  No processo filho:
  1. `dup2(stdin_pipe[0], STDIN_FILENO)` e `dup2(stdout_pipe[1], STDOUT_FILENO)`.
  2. Fechar todos os fds não usados (incluindo as outras pontas dos pipes).
  3. `chdir()` para o diretório do script (necessário para PATH_INFO relativo).
  4. `execve(interpreter, argv, envp)`.
  5. Em caso de falha do `execve`, `exit(1)` (pai detecta via waitpid).

- [ ] **9. Branch pai — I/O via `poll()`**
  No processo pai:
  1. Fechar pontas não usadas dos pipes.
  2. Escrever `request.body` no `stdin_pipe[1]` em loop e fechar.
  3. Ler `stdout_pipe[0]` com `poll()` + timeout (5000 ms) em buffer dinâmico.
  4. Subject **proíbe inspecionar `errno` pós-I/O**: usar apenas o retorno
     de `read`/`write` para decidir continuação.

- [ ] **10. Supervisão do processo**
  - `waitpid(pid, &status, WNOHANG)` em loop com timeout total (~5s).
  - Se timeout estourar: `kill(pid, SIGKILL)` + `waitpid` final, retornar
    `504 Gateway Timeout`.
  - Fechar todos os fds restantes (sem deixar zumbi).

---

## Fase 4 — Resposta CGI

- [ ] **11. Parser da saída CGI → `HttpResponse`**
  - Localizar primeira linha em branco (`\r\n\r\n` ou `\n\n`) → split headers/body.
  - Para cada header:
    - `Status: 302 Found` → setar `res.status = 302` e `reasonPhrase`.
    - `Content-Type: …` → `res.contentType`.
    - `Location: …` → `res.location` (CGI pode emitir redirect).
    - Demais headers → adicionar via `res.setHeader`.
  - Default: `status = 200`, `Content-Type = text/html` se CGI omitir.
  - `res.body = body`.

---

## Fase 5 — Integração no Router e tratamento de erros

- [ ] **12. Dispatch CGI no `Router::handleRequest`**
  Em [`src/http/Router.cpp:115`](../src/http/Router.cpp#L115), **antes** do
  branch de DELETE/POST/GET (mas depois de método permitido,
  `client_max_body_size` e proteção `..`):
  ```cpp
  std::string interpreter = matchCgi(*route, request.path);
  if (route && !interpreter.empty()) {
      CgiHandler cgi(request, *route, config, fullPath, interpreter);
      return cgi.execute();
  }
  ```
  Atenção: `fullPath` deve respeitar `route->root` (já implementado).

- [ ] **13. Mapear falhas CGI para HTTP**
  - `fork`/`pipe` falham → 500.
  - Filho sai com status ≠ 0 (exec falhou) → 502.
  - Timeout do `poll`/`waitpid` → 504.
  - Saída CGI inválida (sem blank line separadora) → 502.
  - Body via `errorBody(status, config)` para usar `error_page` configurado.

---

## Fase 6 — Fixtures, testes e docs

- [ ] **14. Fixture: `www/cgi-bin/hello.py`**
  Script Python que:
  - Imprime `Content-Type: text/html\n\n` seguido de body HTML.
  - Inclui `os.environ.get('QUERY_STRING')` no body para conferir env.
  - Lê `sys.stdin` para POST e ecoa.
  - Adicionar entry em `config.conf`:
    ```nginx
    location /cgi-bin {
        methods GET POST;
        cgi_extension .py /usr/bin/python3;
    }
    ```

- [ ] **15. Suite `tests/test_cgi.sh`**
  Cobrir:
  - GET `/cgi-bin/hello.py?name=alice` → 200, body contém `alice`.
  - POST `/cgi-bin/hello.py` com body → 200, body ecoa stdin.
  - CGI emitindo `Status: 302 Location: /` → 302 com `Location` propagado.
  - Script inexistente → 502 (ou 404 se preferir bloquear antes do `fork`).
  - Script com `time.sleep(10)` → 504 (timeout).
  - Script sem permissão de execução → 502.
  Reaproveitar `lib.sh` e `assert_status`/`assert_body`/`assert_header`.

- [ ] **16. Hook em `run_all.sh` + validação completa**
  Adicionar `tests/test_cgi.sh` à lista do `tests/run_all.sh`.
  Build limpo via WSL: `make re && tests/_run_with_server.sh tests/test_cgi.sh`.
  Rodar suite inteira (`run_all.sh`) e confirmar 0 regressões.

- [ ] **17. Atualizar `docs/TODO.md`**
  - Tabela "Visão geral": linha CGI de `❌` → `✅` com hashes dos commits.
  - Remover bloco CGI da seção "Falta — Jefferson".
  - Adicionar entrada em "Já entregue — Jefferson".
  - Marcar também o item "Testes de integração — CGI" como entregue.

---

## Notas e armadilhas

- **Subject proíbe `errno` pós-I/O**: a decisão de continuar/parar deve
  vir do retorno de `read`/`write`/`poll`, nunca de `errno`. `perror`
  para log é OK.
- **Nunca chamar `read`/`write` sem ter passado por `poll()`** —
  inclui o pipe do CGI no parent.
- **`fork` é permitido apenas para CGI** (segundo o subject).
  Não introduzir `fork` em outros lugares.
- **`chdir` no filho**: depois do `chdir`, `argv[1]` (script path) deve
  ser **relativo** ao diretório de chdir, ou caminho absoluto.
- **Limpar fds**: vazamento de fds no parent quebra `poll()` em sessões
  longas (limite do FD set). Sempre fechar as pontas usadas pelo filho.
- **`waitpid` zumbi**: depois do `kill` por timeout, fazer `waitpid`
  final para reaproveitar o slot.
- **NGINX comparison**: rodar o mesmo script CGI atrás do NGINX e
  comparar headers/body lado a lado durante a auditoria final.
