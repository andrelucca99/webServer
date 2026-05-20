# Testes de integracao

Suites em bash que batem em um `webserv` ja rodando.

## Pre-requisitos

```bash
make
./webserv &     # sobe servidor com config.conf (portas 8080 e 8081)
```

## Rodar

Tudo:

```bash
bash tests/run_all.sh
```

Uma suite individual:

```bash
bash tests/test_get.sh
```

Cada arquivo `test_<feature>.sh` cobre uma fatia da especificacao e e
independente das outras (cada teste limpa as fixtures que criou).

## Variaveis de ambiente

- `BASE_URL` (default `http://127.0.0.1:8080`) - server principal.
- `BASE_URL_RO` (default `http://127.0.0.1:8081`) - server somente GET.

## Verificacao de vazamentos de memoria

Valgrind nao esta disponivel no ambiente, mas o projeto pode ser instrumentado
com AddressSanitizer / LeakSanitizer / UBSan (libasan + libubsan ja instalados).

```bash
make leak-check
```

Esse alvo:

1. Recompila tudo com `-fsanitize=address,undefined` (binario `webserv_san`)
   e gera `parser_smoke` (harness em C++ para Tokenizer + ConfigParser +
   HttpRequestParser).
2. Roda `./parser_smoke` com LSan ativo - qualquer alocacao nao liberada no
   exit (inclusive nos caminhos de excecao em config invalida) faz o teste
   reprovar.
3. Sobe `./webserv_san` em background e executa toda a suite `test_*.sh`.
   Erros de memoria durante o trafego (use-after-free, out-of-bounds,
   double-free, comportamento indefinido) abortam o servidor; o script
   detecta e reprova.

LSan e desabilitado para o servidor (`detect_leaks=0`) porque ele e morto por
sinal e nao tem cleanup explicito do loop principal - ativar LSan no exit
produziria falsos positivos de "still reachable".

Logs ficam em `/tmp/webserv_san_logs/`:
- `parser_smoke.log`
- `webserv_san.log`
- `suite.log`
- `asan_server.*` / `ubsan_server.*` (apenas se houver report)

### Tipos de teste adicionados

- **`tests/parser_smoke.cpp`**: harness standalone que cobre parses validos,
  parses invalidos (excecoes) e stress (5000 tokenizacoes + 2000 parses HTTP).
- **`tests/configs/`**: fixtures de config:
  - `valid_basic.conf`, `valid_multi.conf` - caminhos OK
  - `invalid_missing_brace.conf`, `invalid_unknown_directive.conf`,
    `invalid_unknown_in_location.conf`, `invalid_truncated.conf` - sao
    esperados lancar excecao (sem deixar memoria pendurada)
  - `empty.conf` - parseado com 0 servers (validacao fica em `main.cpp`)
- **`tests/leak_check.sh`**: orquestrador. Tambem rodavel sozinho apos
  `make sanitize`.
