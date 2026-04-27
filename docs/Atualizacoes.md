# 🌐 Webserv - Divisão de Trabalho e Status do Projeto

## 📌 Visão Geral

Este documento descreve:

* O que já foi implementado
* O que ainda falta
* Como o projeto está dividido entre os membros

---

# ✅ PARTE 1 — Infra / Core do Servidor (Responsável: André)

## 🔧 Já implementado

### 🧱 Servidor TCP (base)

* Criação de socket (`socket`)
* Bind (`bind`)
* Escuta (`listen`)
* Aceitação de conexões (`accept`)
* Loop principal do servidor

---

### 📥 Leitura de Request

* Recebimento via `read`
* Bufferização da request
* Log da request recebida

---

### 🧠 Parser HTTP

* Estrutura `HttpRequest`
* Parser dedicado (`HttpRequestParser`)
* Suporte a:

  * Method (GET, POST)
  * Path
  * Headers
  * Body

---

### 🔀 Router

* Separação de responsabilidades (Server → Router)
* Função central:

  ```cpp
  Router::handleRequest()
  ```

---

### 📄 Servir arquivos (GET)

* Leitura de arquivos (`readFile`)
* Resposta HTTP dinâmica
* Suporte a MIME types:

  * HTML
  * CSS
  * JS
  * Imagens

---

### 📝 POST (criação de arquivos)

* Escrita de arquivos (`writeFile`)
* Retorno:

  * `201 Created`
  * `500 Internal Server Error`

---

### 🔐 Segurança (IMPORTANTE)

Proteção contra **Path Traversal**:

* Sanitização do path
* Remoção de `/`
* Bloqueio de `..`
* Suporte a URL encoded (`%2e%2e`)
* Uso de `urlDecode`

✔️ Protege contra:

```
/../../file
/%2e%2e/%2e%2e/file
```

---

### 📡 Respostas HTTP

* `200 OK`
* `201 Created`
* `403 Forbidden`
* `404 Not Found`
* `405 Method Not Allowed`
* `500 Internal Server Error`

---

### 🧪 Testes realizados

```bash
curl http://127.0.0.1:8080/
curl http://127.0.0.1:8080/naoexiste
curl -X POST http://127.0.0.1:8080/test.txt -d "ok"
curl -X POST "http://127.0.0.1:8080/%2e%2e/%2e%2e/hack.txt" -d "hack"
```

---

## ⏳ O que falta (Sua parte)

### 🔥 Prioridade alta

* [ ] Implementar DELETE
* [ ] Melhorar parser (headers completos)
* [ ] Suporte a Content-Length correto
* [ ] Tratamento de múltiplas requisições (keep-alive opcional)

---

### ⚙️ Intermediário

* [ ] Melhorar tratamento de erros
* [ ] Logs estruturados
* [ ] Refatorar organização de arquivos

---

### 🧠 Avançado (se tiver tempo)

* [ ] Chunked transfer encoding
* [ ] Multipart/form-data (upload)
* [ ] CGI (dependendo do escopo)

---

# 🤝 PARTE 2 — Compartilhado (Ambos)

## ⚙️ Parser de Configuração (tipo NGINX)

### Objetivo

Interpretar arquivo `.conf` com:

* porta
* host
* root
* rotas
* métodos permitidos
* CGI

---

### Divisão sugerida

| Pessoa | Responsabilidade                             |
| ------ | -------------------------------------------- |
| Você   | Parser (Tokenizer + leitura)                 |
| Colega | Structs + models (ServerConfig, RouteConfig) |

---

### Status

* [x] Estrutura inicial criada
* [ ] Integração completa com Router
* [ ] Suporte a múltiplos servers

---

# 👥 PARTE 3 — Responsabilidade do Colega - Jefferson

## 🧩 Configuração e Estrutura

### 📦 Models

* `ServerConfig`
* `RouteConfig`

---

### ⚙️ Parser Config

* Tokenizer
* Leitura de blocos
* Validação

---

### 🔀 Integração com Router

* Rotas customizadas
* Root dinâmica
* Métodos permitidos por rota

---

### 🛣️ Routing avançado

* Match de rotas
* Fallback
* Prioridade de rotas

---

### 🔐 Validações

* Métodos permitidos (GET/POST/DELETE)
* Retornar `405` corretamente

---

## ⏳ O que ele precisa fazer

* [ ] Finalizar parser de config
* [ ] Integrar config no Router
* [ ] Implementar rotas dinâmicas
* [ ] Controlar métodos por rota
* [ ] Suporte a múltiplos servers

---

# 🚀 Estado atual do projeto

```text
Server Core        ✔️
Socket             ✔️
HTTP Parser        ✔️
Router             ✔️
GET                ✔️
POST               ✔️
Segurança          ✔️
Config Parser      ⚠️ parcial
DELETE             ❌ pendente
```

---

# 🎯 Próximos passos recomendados

1. Implementar DELETE
2. Integrar Config Parser com Router
3. Validar métodos por rota
4. Testes completos (curl + browser)

---

# 🧠 Conclusão

O projeto já possui uma base sólida:

* Arquitetura correta
* Separação de responsabilidades
* Segurança implementada
* HTTP funcional

👉 Falta agora **fechar funcionalidades e integração**, não mais base.

---
