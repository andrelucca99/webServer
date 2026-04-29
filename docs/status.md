# Status do Projeto — Webserv (42)

> Análise do estado atual do código vs. requisitos do subject (versão 24.0).
> Divisão de responsabilidades conforme `webserv_divisao_projeto.pdf`.

---

## O que está implementado

### Config Layer
- `Tokenizer` — divide o arquivo em tokens (`{`, `}`, `;`, palavras)
- `ConfigParser` — parser recursivo descendente; suporta múltiplos blocos `server`
- Diretivas de servidor: `listen`, `host`, `root`, `client_max_body_size`, `error_page`
- Diretivas de location: `methods`, `index`, `autoindex`, `return <code> <url>`
- Structs: `Config` → `ServerConfig` → `RouteConfig`

### HTTP Layer
- `HttpRequestParser` — parse de request line, headers e body (respeita `Content-Length`)
- Validação de versão HTTP (aceita 1.0 e 1.1, rejeita 2.0+)
- `HttpResponse` — build de resposta com status, headers e body; MIME types; reason phrases
- `Router::handleRequest` — longest-prefix match de rotas, GET/POST/DELETE, autoindex,
  validação de método (405), limite de body (413), proteção contra path traversal, redirects

### Server Layer
- `Server::run()` — event loop com `poll()`, I/O não bloqueante (`O_NONBLOCK`)
- Suporte a múltiplos server blocks em portas distintas
- Accept de conexões, leitura em buffer, detecção de request completa, envio de resposta
- `File::readFile` / `File::writeFile` — utilitários de I/O de arquivo

### Testes
- `test_parser.cpp` — 8 casos de teste para o `HttpRequestParser` (compilado separado)

---

## O que falta — por responsabilidade

---

### Pessoa A — Infraestrutura / Core do Servidor

**Feito:**
- Sockets (`socket`, `bind`, `listen`, `accept`) — inline em `Server.cpp`
- Multiplexação com `poll()`
- Gerenciamento de clientes (`ClientState`, buffers de leitura/escrita)
- Loop principal não bloqueante
- Múltiplas portas

**Falta:**

#### Conformidade estrita com o subject
- **Verificação de `errno` após read/write** — o subject proíbe explicitamente usar `errno`
  para ajustar comportamento após operações de I/O. Verificar se `Server.cpp` viola isso.
- **Timeout de requisição** — `poll()` está com `timeout = -1` (espera infinita). O subject
  exige que nenhuma requisição trave indefinidamente. Adicionar timeout e fechar conexões
  travadas.
- **Desconexão de cliente** — verificar se `POLLHUP` / `POLLERR` são tratados e a conexão
  é encerrada limpa e corretamente.


---

### Pessoa B — HTTP / Aplicação

**Feito:**
- Parser de requisições HTTP (`HttpRequestParser`)
- Construção de respostas HTTP (`HttpResponse`)
- Métodos GET, POST e DELETE
- Servir arquivos estáticos, autoindex, index file, error pages

**Falta:**

#### CGI — **não implementado** (obrigatório pelo subject)
O subject exige suporte a pelo menos um CGI (ex: `python3`, `php-cgi`).

O que precisa ser construído:
- Diretiva de config por location: `cgi_extension .py /usr/bin/python3` (ou similar)
- Campo `cgi_extensions` em `RouteConfig` (`std::map<std::string, std::string>`)
- `CgiHandler` — detecta extensão no path, monta variáveis de ambiente obrigatórias:
  - `REQUEST_METHOD`, `PATH_INFO`, `QUERY_STRING`, `CONTENT_LENGTH`, `CONTENT_TYPE`,
    `SERVER_NAME`, `SERVER_PORT`, `SCRIPT_FILENAME`, `REDIRECT_STATUS`
- Execução via `fork` + `execve` + `pipe` para stdin/stdout
- `chdir` para o diretório do script antes do `execve`
- Leitura da saída do CGI (parse de headers + body) e construção da `HttpResponse`
- Integração com o event loop do `Server` (resposta CGI assíncrona ou síncrona com timeout)

#### Root por location — **falta no config e no roteamento**
O subject descreve: "se `/kapouet` tiver root em `/tmp/www`, então `/kapouet/pouic/toto/pouet`
busca `/tmp/www/pouic/toto/pouet`." Hoje só existe `root` no nível do `ServerConfig`.

O que precisa ser feito:
- Adicionar campo `std::string root` em `RouteConfig`
- Parsear diretiva `root` dentro de blocos `location` em `ConfigParser`
- Ajustar `Router` para usar o `root` da route quando definido, com fallback para o do server

#### Diretiva de upload — **não implementada**
O subject pede que o local de armazenamento de uploads seja configurável por rota.

O que precisa ser feito:
- Adicionar campo `std::string upload_store` em `RouteConfig`
- Parsear diretiva `upload_store <path>` em `ConfigParser`
- Ajustar o handler de POST no `Router` para salvar no `upload_store` da rota

#### Inconsistência em `HttpResponse`
Existem dois builders com campos diferentes:
- `toString()` — usa `_status_code`, `_reason_phrase`, `_headers`, `_body` (privados)
- `build()` — usa `status`, `body`, `contentType`, `location` (públicos)

O `Router` usa `build()`. Consolidar em uma única interface para evitar bugs futuros.

---

### Responsabilidades Compartilhadas

**Feito:**
- Parser do arquivo de configuração (parcialmente — ver gaps acima)
- Definição de interfaces entre módulos

**Falta:**

#### README.md — **obrigatório pelo subject**
O subject (capítulo V) exige um `README.md` com:
- Primeira linha italicizada: *Este projeto foi criado como parte do currículo da 42 por `<login1>`[, `<login2>`].*
- Seção **Descrição** — objetivo e visão geral
- Seção **Instruções** — compilação, instalação, execução
- Seção **Recursos** — referências (RFCs, tutoriais) + descrição de como a IA foi usada

#### Testes de integração — **inexistentes**
O subject recomenda explicitamente escrever testes em Python, Golang ou C++.
Coberturas mínimas necessárias:
- GET de arquivo estático, GET de diretório (autoindex e index)
- POST (upload), DELETE
- Redirect (301/302)
- Erro 404, 405, 413
- CGI (quando implementado)
- Múltiplas conexões simultâneas (resiliência)

#### Comparação com NGINX
- Testar headers de resposta com `curl -v` e comparar com NGINX
- Testar comportamento com `telnet` (requisições malformadas, conexão abrupta)

---

## Resumo executivo

| Área | Status |
|---|---|
| Config parsing (base) | Feito |
| Root por location | **Falta** |
| Upload store config | **Falta** |
| HTTP request parser | Feito |
| HTTP response builder | Feito (mas inconsistente) |
| GET / POST / DELETE | Feito |
| CGI | **Falta** (maior gap) |
| poll() event loop | Feito |
| Timeout de conexão | **Falta** |
| errno após I/O | A verificar |
| README.md | **Falta** |
| Testes de integração | **Falta** |

### Ordem de prioridade sugerida

1. **Root por location** — desbloqueia roteamento correto para CGI
2. **CGI** — requisito obrigatório mais trabalhoso; Pessoa B
3. **Upload store** — pequena adição de config + Router
4. **Timeout de conexão** — conformidade com subject; Pessoa A
5. **Consolidar HttpResponse** — elimina fonte de bugs
6. **README.md** — obrigatório para entrega
7. **Testes de integração** — qualidade e resiliência
