# Testes de integracao

Suites em bash que batem em um `webserv_test` ja rodando.

## Pre-requisitos

```bash
make
./webserv_test &     # sobe servidor com config.conf (portas 8080 e 8081)
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
