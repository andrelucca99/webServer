# Plano de Implementação — HttpRequestParser

## Progresso

| Passo | Descrição | Status |
|-------|-----------|--------|
| 1 | Separar headers do body (`\r\n\r\n`) | ✅ feito |
| 2 | Parse da request-line (method + uri + version) | ✅ feito |
| 3 | Parse da URI (path + query_string) | ✅ feito |
| 4 | Parse dos headers (`map<string,string>`) | ✅ feito |
| 5 | Extração do body (Content-Length) | ✅ feito |
| 6 | Testes isolados (`test_parser.cpp`) | ✅ feito |

---

## Objetivo
Criar um parser que recebe uma `std::string` com dados brutos do socket e preenche a struct `HttpRequest`.

---

## Arquivos a criar

1. `src/includes/HttpRequestParser.hpp` — declaração da classe
2. `src/http/HttpRequestParser.cpp` — implementação

---

## Interface da classe

```cpp
class HttpRequestParser {
public:
    HttpRequest parse(const std::string& raw);
};
```

---

## Passos de implementação

### Passo 1 — Separar headers do body
- Localizar `\r\n\r\n` na string bruta
- Tudo antes → seção de headers
- Tudo depois → body

### Passo 2 — Fazer parse da request-line
- Primeira linha dos headers (até `\r\n`)
- Extrair `method` (antes do 1º espaço)
- Extrair `uri` (entre 1º e 2º espaço)
- Extrair `http_version` (após 2º espaço)
- Lançar erro 400 se malformada
- Lançar erro 505 se versão != HTTP/1.0 e != HTTP/1.1

### Passo 3 — Fazer parse da URI
- Verificar se contém `?`
- Se sim: separar `path` e `query_string`
- Se não: `path = uri`, `query_string` vazio

### Passo 4 — Fazer parse dos headers
- Iterar linha a linha (separar por `\r\n`)
- Para cada linha, localizar `:` que separa nome do valor
- Normalizar nome para lowercase
- Inserir no `map<string, string>`

### Passo 5 — Extrair o body
- Verificar header `content-length`
- Se presente: extrair N bytes do body
- Se ausente: body vazio

### Passo 6 — Tratar fragmentação TCP
- Parser deve aceitar dados parciais
- Só processar quando `\r\n\r\n` for encontrado
- Acumular em buffer externo (responsabilidade de quem chama o parser)

---

## Tratamento de erros

| Situação | Comportamento |
|---|---|
| `\r\n\r\n` ausente | Retornar indicação de requisição incompleta |
| request-line malformada | Lançar exceção com código 400 |
| Versão HTTP inválida | Lançar exceção com código 505 |
| Header sem `:` | Ignorar linha |

---

## Casos de teste (Passo 5 do roadmap)

1. GET simples: `GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n`
2. POST com body: `POST /upload HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello`
3. Múltiplos headers
4. Request-line malformada
5. URI com query string: `GET /search?q=hello HTTP/1.1\r\n...`

---

## Ordem de execução

```
Passo 1: Separar headers do body (\r\n\r\n)
    ↓
Passo 2: Parse da request-line (method + uri + version)
    ↓
Passo 3: Parse da URI (path + query_string)
    ↓
Passo 4: Parse dos headers (map<string,string>)
    ↓
Passo 5: Extração do body (Content-Length)
    ↓
Passo 6: Criar arquivo de teste isolado
```
