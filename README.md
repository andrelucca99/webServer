# Webserv — Documentação Inicial (Config Parser)


## 📌 Visão Geral

Este documento descreve a implementação inicial do projeto **Webserv**, com foco na construção do **parser de configuração** e na definição das estruturas base utilizadas pelo servidor HTTP.

O objetivo desta etapa foi estabelecer uma **fundação sólida e desacoplada**, permitindo que o desenvolvimento do servidor (infraestrutura) e da camada HTTP (aplicação) ocorram em paralelo.

---

## 🧱 Arquitetura Inicial

A arquitetura atual está dividida em três camadas principais:

```
Config File (.conf)
        ↓
   Tokenizer
        ↓
 ConfigParser
        ↓
     Structs
        ↓
   Server Runtime (futuro)
```

---

## 📁 Estrutura de Diretórios

```
src/
  main.cpp
  config/
    Tokenizer.cpp
    ConfigParser.cpp
  includes/
    Config.hpp
    ServerConfig.hpp
    RouteConfig.hpp
    Tokenizer.hpp
    ConfigParser.hpp
```

---

## 🔤 Tokenizer

### 📄 Responsabilidade

Responsável por transformar o conteúdo bruto do arquivo de configuração em **tokens léxicos**.

### 📥 Entrada

```conf
server {
    listen 8080;
}
```

### 📤 Saída

```
["server", "{", "listen", "8080", ";", "}"]
```

### ⚙️ Regras

* Separa por espaços
* Isola símbolos `{`, `}`, `;`
* Remove whitespace irrelevante

### 🎯 Objetivo

Simplificar o parsing, evitando manipulação direta de strings complexas.

---

## 🧠 ConfigParser

### 📄 Responsabilidade

Interpretar os tokens e construir as estruturas de configuração.

### 🔧 Funcionalidades implementadas

* Parsing de blocos `server`
* Parsing de blocos `location`
* Diretivas suportadas:

  * `listen`
  * `host`
  * `root`
  * `methods`
  * `index`
  * `autoindex`

---

### 🔄 Fluxo de parsing

1. Carrega arquivo `.conf`
2. Tokeniza conteúdo
3. Percorre tokens sequencialmente
4. Identifica blocos (`server`, `location`)
5. Preenche structs

---

### ⚠️ Validações atuais

* Verificação de tokens esperados (`expect`)
* Erro em diretivas desconhecidas
* Erro em fim inesperado de arquivo

---

## 🧱 Estruturas de Configuração

### 📄 Config

```cpp
struct Config {
    std::vector<ServerConfig> servers;
};
```

Representa toda a configuração do sistema.

---

### 📄 ServerConfig

```cpp
struct ServerConfig {
    int port;
    std::string host;
    std::string root;
    std::vector<RouteConfig> routes;
};
```

Representa um servidor (porta + comportamento).

---

### 📄 RouteConfig

```cpp
struct RouteConfig {
    std::string path;
    std::vector<std::string> methods;
    std::string index;
    bool autoindex;
};
```

Representa uma rota (`location`).

---

## 🧪 Teste Atual

### 📄 Arquivo de configuração

```conf
server {
    listen 8080;
    host 127.0.0.1;
    root /var/www/html;

    location / {
        methods GET POST;
        index index.html;
        autoindex on;
    }
}
```

---

### ▶️ Execução

```bash
make
./webserv_test
```

---

### ✅ Saída esperada

```
Server:
Port: 8080
Host: 127.0.0.1
Root: /var/www/html
  Route: /
```

---

## 🔌 Integração com o restante do projeto

Esta camada será consumida pela infraestrutura do servidor.

### Exemplo de uso futuro

```cpp
ConfigParser parser;
Config config = parser.parse("config.conf");

Server server(config.servers[0]);
server.start();
```

---

## 🤝 Divisão de responsabilidades

### 👨‍💻 Infra (Server Core)

* Socket
* Poll / multiplexação
* Gerenciamento de conexões
* Uso da config

---

### 👨‍💻 HTTP Layer

* Parser HTTP
* Response builder
* Métodos (GET, POST, DELETE)
* CGI

---

## 🚀 Próximos passos

### Parser

* Adicionar suporte a:

  * `client_max_body_size`
  * `error_page`
  * `upload`
  * `cgi`
* Melhorar mensagens de erro
* Validação mais robusta

---

### Infra

* Implementar socket server
* Implementar loop com `poll()`
* Associar `ServerConfig` a portas reais

---

### Integração

* Definir:

  * `matchRoute()`
  * seleção de server por porta

---

## 📚 Observações técnicas

* Projeto segue padrão C++98
* Sem uso de bibliotecas externas
* Parser inspirado no modelo do NGINX (simplificado)
* Estrutura pensada para suportar múltiplos servidores

---

## ✅ Status atual

✔ Tokenizer funcional
✔ Parser funcional
✔ Estruturas definidas
✔ Teste básico validado

---

## 📌 Conclusão

A base do sistema foi construída com sucesso.
O projeto agora possui uma estrutura sólida que permite o desenvolvimento paralelo das demais partes com baixo acoplamento.

---
