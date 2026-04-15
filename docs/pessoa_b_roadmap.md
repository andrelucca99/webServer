# Pessoa B — HTTP / Aplicação: O que já está feito e o que fazer agora

---

## Visão geral das entregas

| Entrega | Status |
|---------|--------|
| HttpRequestParser | ✅ completo |
| HttpResponse | 🔄 em andamento |
| GET — servir arquivos estáticos | ❌ |
| POST — receber dados | ❌ |
| DELETE | ❌ |
| Autoindex (listagem de diretório) | ❌ |
| Upload (`multipart/form-data`) | ❌ |
| CGI | ❌ |

---

## O que já está pronto (feito pela Pessoa A)

O projeto tem uma **camada de configuração completa e funcional**:

- **Tokenizer** (`src/config/Tokenizer.cpp`): lê o arquivo `.conf` e quebra o texto em tokens (`server`, `{`, `listen`, `8080`, `;`, etc.). É o equivalente a um "scanner" — transforma texto bruto em peças com as quais o parser consegue trabalhar.

- **ConfigParser** (`src/config/ConfigParser.cpp`): consome os tokens e monta as estruturas de dados. Entende `server { }` e `location { }`, extrai porta, host, root, métodos, index e autoindex.

- **Estruturas de dados** (`Config`, `ServerConfig`, `RouteConfig`): guardam tudo que foi lido do arquivo de configuração e serão consumidas pelos módulos que você vai construir.

O que **não existe ainda**: nenhum socket, nenhuma conexão de rede, nenhum byte HTTP trafegando. O projeto compila e imprime o resultado do parse — isso é tudo.

---

## Sua primeira tarefa: Parser de Requisições HTTP

Das suas responsabilidades (parser HTTP, respostas HTTP, GET/POST/DELETE, arquivos estáticos, upload, CGI), o **parser de requisições HTTP** é o ponto de partida obrigatório. Tudo o mais depende de você conseguir ler e entender o que o cliente enviou.

Sem o parser, não há como implementar GET (o que o cliente quer?), nem POST (qual é o body?), nem CGI (quais headers passar?).

---

## Divisão da tarefa em partes menores

### Parte 1 — O que é uma requisição HTTP?

**Resumo:** Antes de escrever qualquer código, você precisa entender a estrutura do texto que chega pelo socket. Uma requisição HTTP é texto puro com um formato definido pela RFC 7230.

**O que estudar:**

- **Formato geral de uma requisição HTTP/1.1**
  - Linha de requisição: `METHOD URI HTTP/VERSION\r\n`
  - Cabeçalhos: `Nome: Valor\r\n` (um por linha)
  - Linha em branco obrigatória: `\r\n` (separa headers do body)
  - Body (opcional): presente em POST, ausente em GET/DELETE

- **O que é `\r\n` (CRLF)**
  - Carriage Return + Line Feed. O HTTP exige `\r\n`, não só `\n`. Ignorar isso causa bugs difíceis de diagnosticar.

- **Exemplos concretos para ler/enviar com telnet**
  - `telnet localhost 8080` → digitar manualmente uma requisição GET
  - Objetivo: ver com os próprios olhos o texto que o servidor recebe

- **RFC 7230 (seções 3.1 e 3.2)**
  - Não precisa ler tudo. Leia as seções sobre request-line e header fields.


Resumo do que aprendi:
O http/1.1 é a evolução do http/1.0, sendo uma das principais diferenças ele nao fechar a porta de requição permitindo o connect que antes não era possivel.
o protocolo considste em metodos, que são se não me engano 7, sendo os principais e necessários no projeto, GET, HEAD, POST, DELETE, OPTIONS, PUT

é composto por header, body e o encerramento com \r\n, \r\n também deve ser utilizado para separar o header do body

a linha de header é constituida  pela request line que sempre inicia com o metodo, o que oque/onde vai ser manipulado e a versão do http, sendo também obrigatório o host logo abaixo e pros metodos que precisam o body pode ser adicionado, mas não é obrigatório, tendo o comportamento definido de acordo
---

### Parte 2 — Como o texto chega ao servidor (interface com a Pessoa A)

**Resumo:** Seu parser não lê arquivos — ele recebe uma `std::string` (ou buffer de `char[]`) que a camada de infraestrutura passou. Você precisa entender o contrato entre os dois módulos.

**O que estudar:**

- **O que é um file descriptor (fd)**
  - Um número inteiro que representa uma conexão aberta. `read(fd, buf, size)` preenche o buffer com bytes do cliente.
  - Você não vai chamar `read()` diretamente (isso é responsabilidade da Pessoa A), mas precisa entender o que chega.

- **Por que os dados podem chegar em pedaços (fragmentação TCP)**
  - O TCP não garante que uma requisição HTTP completa chegue de uma vez. Pode chegar `GET /ind` num read e `ex.html HTTP/1.1\r\n...` no próximo.
  - Seu parser precisa ser capaz de acumular dados parciais e só processar quando tiver a requisição completa.

- **Como detectar o fim dos headers**
  - A sequência `\r\n\r\n` marca o fim dos cabeçalhos. Enquanto ela não aparecer no buffer acumulado, a requisição não está completa.

---

### Parte 3 — Estrutura de dados para representar a requisição

**Resumo:** Antes de codificar o parser, defina a struct/classe que vai guardar o resultado. Isso força você a pensar no que precisa extrair.

**O que estudar:**

- **Campos obrigatórios a extrair**
  - `method`: `"GET"`, `"POST"`, `"DELETE"`
  - `uri`: `/index.html`, `/upload?name=foto`
  - `http_version`: `"HTTP/1.1"`
  - `headers`: mapa de `string → string` (ex: `"Content-Type" → "text/html"`)
  - `body`: `std::string` (vazio em GET, preenchido em POST)

- **`std::map<std::string, std::string>` em C++98**
  - Como declarar, inserir e consultar. Será seu container de headers.

- **Por que separar URI de query string**
  - `/search?q=hello` → path é `/search`, query é `q=hello`. Você vai precisar dos dois separadamente.

---

### Parte 4 — Implementar o parser linha a linha

**Resumo:** Agora sim, escrever o código. O parser recebe uma `std::string` com a requisição e preenche a struct da Parte 3.

**O que estudar:**

- **`std::string::find()`, `substr()`, `find_first_of()`**
  - Ferramentas principais para localizar `\r\n`, `:`, espaços dentro da string.

- **Parsing da request-line**
  - Localizar o primeiro espaço → `method`
  - Localizar o segundo espaço → `uri`
  - O restante até `\r\n` → `http_version`

- **Parsing dos headers**
  - Iterar linha a linha (separar por `\r\n`)
  - Para cada linha, encontrar o `:` que separa nome do valor
  - Normalizar o nome para lowercase (boa prática, facilita comparações)

- **Leitura do body**
  - O header `Content-Length` diz quantos bytes esperar depois do `\r\n\r\n`
  - Se `Content-Length` não existe, body está vazio (para GET/DELETE)

- **Tratamento de erros básico**
  - Request-line malformada → status `400 Bad Request`
  - Versão HTTP diferente de `HTTP/1.1` ou `HTTP/1.0` → status `505 HTTP Version Not Supported`

---

### Parte 5 — Testar o parser isoladamente

**Resumo:** Testar o parser sem precisar do servidor rodando. Você passa strings hardcoded e verifica se o resultado está correto. Isso é fundamental para ter confiança antes de integrar.

**O que estudar:**

- **Como escrever um main de teste em C++98**
  - Criar um `test_parser.cpp` separado que instancia o parser com strings fixas e imprime os campos extraídos.

- **Casos de teste que você deve cobrir manualmente:**
  1. GET simples: `GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n`
  2. POST com body: `POST /upload HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello`
  3. Requisição com múltiplos headers
  4. Linha de requisição malformada (sem espaço, sem versão)
  5. Dados chegando em dois pedaços (simular fragmentação)

- **`telnet` como ferramenta de exploração**
  - Quando o servidor da Pessoa A estiver disponível, use `telnet` para enviar requisições manuais e ver o que o parser recebe de verdade.
  - Comando: `telnet 127.0.0.1 8080`

---

## Resumo visual da ordem

```
Parte 1: Entender o formato HTTP (teoria + RFC + telnet)
   ↓
Parte 2: Entender como os dados chegam (TCP, fragmentação, \r\n\r\n)
   ↓
Parte 3: Definir a struct HttpRequest com todos os campos necessários
   ↓
Parte 4: Implementar o parser que preenche a struct
   ↓
Parte 5: Testar com strings fixas antes de integrar ao servidor
```

Ao terminar a Parte 5 com testes passando, você tem o **parser HTTP completo e confiável** — base para tudo que vem depois: construir respostas, servir arquivos, implementar GET, POST e DELETE.
