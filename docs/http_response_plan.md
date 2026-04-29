# Plano de Implementação — Respostas HTTP

## Progresso

| Passo | Descrição | Status |
|-------|-----------|--------|
| 1 | Struct + forma canônica | ✅ feito |
| 2 | `reasonPhraseFor` (status codes → reason phrases) | ✅ feito |
| 3 | `mimeTypeFor` (extensão → MIME type) | ❌ |
| 4 | `setHeader` + `setBody` (Content-Length automático) | ❌ |
| 5 | `serialize()` (monta a string HTTP final) | ❌ |
| 6 | `buildError()` (resposta de erro com HTML) | ❌ |
| 7 | Testes isolados | ❌ |

---

## Objetivo
Criar a estrutura e o builder que montam uma resposta HTTP completa a partir de um status code, headers e body, produzindo a string pronta para ser enviada pelo socket.

---

## Por que fazer isso agora?

O parser já sabe **ler** o que o cliente envia. Agora o servidor precisa **responder**. Sem a camada de resposta, não há como implementar GET, POST, DELETE, error pages — nada sai do servidor. Esta é a peça que conecta o parser (entrada) ao socket (saída).

---

## O que é uma resposta HTTP?

Antes de codar, entenda o formato. Uma resposta HTTP/1.1 é texto puro:

```
HTTP/1.1 200 OK\r\n
Content-Type: text/html\r\n
Content-Length: 13\r\n
\r\n
Hello, world!
```

Estrutura:
1. **Status-line**: `HTTP/1.1 <status_code> <reason_phrase>\r\n`
2. **Headers**: `Nome: Valor\r\n` (um por linha)
3. **Linha em branco**: `\r\n` (separa headers do body)
4. **Body**: conteúdo da resposta (HTML, arquivo, JSON, etc.)

Referência: RFC 7230 seção 3.1.2 (status-line) e RFC 7231 seção 6 (status codes).

---

## Arquivos a criar

1. `src/includes/HttpResponse.hpp` — struct que guarda os dados da resposta
2. `src/http/HttpResponse.cpp` — implementação (serialize + helpers)
3. `src/http/test_response.cpp` — testes isolados

---

## Interface proposta

### Struct HttpResponse

```cpp
struct HttpResponse {
    std::string http_version;                        // "HTTP/1.1"
    int         status_code;                         // 200, 404, 500...
    std::string reason_phrase;                       // "OK", "Not Found"...
    std::map<std::string, std::string> headers;      // Content-Type, Content-Length, etc.
    std::string body;                                // conteudo da resposta

    HttpResponse();

    // Monta a string final pronta para enviar pelo socket
    std::string serialize() const;

    // Insere um header (conveniencia)
    void setHeader(const std::string& key, const std::string& value);

    // Define body e atualiza Content-Length automaticamente
    void setBody(const std::string& content);

    // Helpers estaticos
    static std::string getReasonPhrase(int code);
    static std::string getMimeType(const std::string& extension);
};
```

**Nota:** Diferente do parser (que é uma classe separada da struct), aqui faz sentido colocar `serialize()` direto na struct. O motivo: a resposta é montada aos poucos (setHeader, setBody) e depois serializada de uma vez. Não há parsing complexo — é construção linear.

---

## Passos de implementação

### Passo 1 — Struct e construtor padrão

Criar `HttpResponse.hpp` com todos os campos. O construtor padrão deve inicializar:
- `http_version = "HTTP/1.1"`
- `status_code = 200`
- `reason_phrase = "OK"`
- headers e body vazios

**O que estudar:**
- Nada novo — mesma ideia do `HttpRequest`, mas com campos diferentes.

---

### Passo 2 — Mapa de status codes → reason phrases

Implementar `getReasonPhrase(int code)` que retorna a string correspondente. Códigos necessários para o webserv:

| Código | Reason Phrase | Quando usar |
|--------|---------------|-------------|
| 200 | OK | Sucesso geral (GET, POST, DELETE) |
| 201 | Created | Recurso criado com sucesso (POST/upload) |
| 204 | No Content | DELETE bem-sucedido, sem body |
| 301 | Moved Permanently | Redirecionamento permanente |
| 400 | Bad Request | Request malformada (parser lança erro) |
| 403 | Forbidden | Sem permissão para acessar o recurso |
| 404 | Not Found | Arquivo/recurso não existe |
| 405 | Method Not Allowed | Método não está no `methods` da RouteConfig |
| 413 | Payload Too Large | Body excede client_max_body_size (futuro) |
| 500 | Internal Server Error | Erro genérico do servidor |
| 505 | HTTP Version Not Supported | Versão != 1.0 e != 1.1 |

**O que estudar:**
- Como C++98 não tem `std::unordered_map`, use `std::map<int, std::string>` ou uma função com `switch/case`.

---

### Passo 3 — Mapa de extensão → MIME type

Implementar `getMimeType(const std::string& extension)` para determinar o `Content-Type` correto ao servir arquivos. Extensões necessárias:

| Extensão | MIME Type |
|----------|-----------|
| `.html` | `text/html` |
| `.css` | `text/css` |
| `.js` | `application/javascript` |
| `.json` | `application/json` |
| `.png` | `image/png` |
| `.jpg` / `.jpeg` | `image/jpeg` |
| `.gif` | `image/gif` |
| `.ico` | `image/x-icon` |
| `.txt` | `text/plain` |
| (default) | `application/octet-stream` |

**O que estudar:**
- `std::string::rfind('.')` para extrair a extensão de um path como `/img/foto.png`

---

### Passo 4 — setHeader e setBody

- `setHeader(key, value)`: insere no map de headers.
- `setBody(content)`: atribui ao campo `body` **e** chama `setHeader("Content-Length", ...)` com o tamanho convertido para string.

**O que estudar:**
- Converter `int` para `std::string` em C++98: use `std::ostringstream` (de `<sstream>`).

---

### Passo 5 — serialize()

Monta a string final concatenando tudo no formato HTTP:

```
HTTP/1.1 200 OK\r\n
Content-Type: text/html\r\n
Content-Length: 13\r\n
\r\n
Hello, world!
```

Algoritmo:
1. Montar status-line: `http_version + " " + code + " " + reason + "\r\n"`
2. Iterar o map de headers: para cada par, `key + ": " + value + "\r\n"`
3. Adicionar `"\r\n"` (linha em branco)
4. Adicionar body

**O que estudar:**
- Converter `int` para string para o status code (mesmo `std::ostringstream`).
- Iterar `std::map` com iteradores em C++98.

---

### Passo 6 — Helper para respostas de erro

Criar uma função (pode ser estática ou livre) que gera uma resposta de erro completa com HTML mínimo:

```cpp
static HttpResponse buildError(int code);
```

Que produz algo como:

```html
<html>
<head><title>404 Not Found</title></head>
<body><h1>404 Not Found</h1></body>
</html>
```

E já preenche `Content-Type: text/html` e `Content-Length`.

Isso vai ser usado em todos os handlers futuros (arquivo não encontrado, método não permitido, etc.).

---

### Passo 7 — Testes isolados

Criar `test_response.cpp` com os seguintes cenários:

1. **Resposta 200 simples**: criar HttpResponse, setBody("hello"), verificar que serialize() produz a string HTTP correta
2. **Headers customizados**: setHeader("X-Custom", "valor"), verificar que aparece no serialize()
3. **Content-Length automático**: setBody("abc"), verificar que header Content-Length == "3"
4. **getReasonPhrase**: verificar que 200→"OK", 404→"Not Found", 500→"Internal Server Error"
5. **getMimeType**: verificar que ".html"→"text/html", ".png"→"image/png", desconhecido→"application/octet-stream"
6. **buildError(404)**: verificar status 404, body contém "404", Content-Type é text/html
7. **buildError(405)**: verificar status 405, reason "Method Not Allowed"

---

## Tratamento de erros

| Situação | Comportamento |
|---|---|
| Status code desconhecido no getReasonPhrase | Retornar "Unknown" |
| Extensão desconhecida no getMimeType | Retornar "application/octet-stream" |
| serialize() chamado sem body | Funciona normalmente (body vazio, sem Content-Length) |

---

## Ordem de execução

```
Passo 1: Struct HttpResponse + construtor padrão
    ↓
Passo 2: getReasonPhrase (mapa status → texto)
    ↓
Passo 3: getMimeType (mapa extensão → MIME)
    ↓
Passo 4: setHeader + setBody (com Content-Length automático)
    ↓
Passo 5: serialize() (monta a string HTTP final)
    ↓
Passo 6: buildError() (gera respostas de erro com HTML)
    ↓
Passo 7: Testes isolados
```

---

## Como vai se integrar com o resto

```
Cliente envia bytes → [Pessoa A: socket/poll] → std::string bruta
    ↓
HttpRequestParser::parse(raw) → HttpRequest   ← (já feito por você)
    ↓
Handler decide o que fazer (GET/POST/DELETE)   ← (próxima etapa, depois deste plano)
    ↓
HttpResponse resposta;                         ← (este plano)
resposta.setHeader("Content-Type", ...);
resposta.setBody(conteudo);
    ↓
std::string saida = resposta.serialize();
    ↓
[Pessoa A: write(fd, saida)] → Cliente recebe a resposta
```
