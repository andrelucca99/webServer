# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Code Headers

Always use the following author header for new files:
```
By: jtertuli <jtertuli@student.42sp.org.br>
```

Do NOT use "jefferson" or other names in the file headers.

## Git Commits

**IMPORTANT:** Do NOT add `Co-Authored-By:` trailers to commit messages. Keep commits simple with just the message.

### Desativando atribuição automática do Claude Code

O Claude Code adiciona automaticamente `Co-authored-by: Claude <...>` nos commits e PRs, fazendo o Claude aparecer como colaborador no repositório. Para evitar isso, a configuração já foi aplicada em `~/.claude/settings.json` (Windows: `%APPDATA%\claude-code\settings.json`):

```json
{
  "attribution": {
    "commit": "",
    "pr": ""
  }
}
```

## Build Commands

```bash
make        # Build → produces webserv_test executable
make re     # Clean rebuild
make clean  # Remove object files only
make fclean # Remove object files and executable
```

Compiler: `c++` with `-Wall -Wextra -Werror -std=c++98`. No external dependencies.

Run the test binary:
```bash
./webserv_test
```

## Architecture

This is an HTTP web server (NGINX-inspired) built from scratch in C++98. Currently only the configuration layer is implemented; networking and HTTP layers are planned.

### Three-Layer Design

```
Config Layer  →  Infrastructure Layer  →  HTTP Layer
(implemented)    (sockets, poll/epoll)     (request/response, CGI)
                    (planned)                 (planned)
```

### Config Parsing Pipeline

```
config.conf → Tokenizer → token stream → ConfigParser → Config/ServerConfig/RouteConfig structs
```

- **Tokenizer** (`src/config/Tokenizer.cpp`): Splits raw file into tokens, isolating `{`, `}`, `;` as their own tokens.
- **ConfigParser** (`src/config/ConfigParser.cpp`): Recursive descent parser consuming the token stream. Entry point is `parse()`, which delegates to `parseServer()` → `parseRoute()`. Throws `std::runtime_error` on invalid syntax.

### Data Structures (`src/includes/`)

- `Config` — root container holding a `std::vector<ServerConfig>`
- `ServerConfig` — port, host, root dir, `std::vector<RouteConfig>`
- `RouteConfig` — path, allowed HTTP methods, index file, autoindex flag

Future directives to add: `client_max_body_size`, `error_page`, `upload`, `cgi`.

### Supported Config Syntax

```nginx
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

`main.cpp` is a test harness that parses `config.conf` and prints the resulting structures.
