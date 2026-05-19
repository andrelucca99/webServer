*Este projeto foi criado como parte do currículo da 42 por jtertuli, andrelucca99.*

---

## Description

Webserv is an HTTP/1.1 web server written in C++98, inspired by NGINX. It handles
multiple simultaneous connections using a single `poll()` loop, supports multiple
virtual servers on different ports, serves static content, accepts uploads, executes
CGI scripts and supports per-route configuration.

Features:
- Non-blocking I/O via a single `poll()` (read and write monitored simultaneously)
- Multiple server blocks on distinct interface:port pairs
- HTTP methods: `GET`, `POST`, `DELETE`
- Per-route method restrictions (`405 Method Not Allowed`)
- HTTP redirects (`return <code> <url>`)
- Directory index file and autoindex listing
- File upload via `multipart/form-data` with configurable `upload_store`
- CGI execution via `fork`/`execve` (e.g. Python scripts) with per-extension
  interpreter mapping. Output of CGI (headers + body) is parsed back into the
  HTTP response.
- Configurable `client_max_body_size` (`413 Payload Too Large`)
- Configurable `error_page` per status code, with built-in fallback pages
- Per-route `root` (NGINX-style)
- Path traversal protection (`403 Forbidden`)
- Idle-connection timeout (30s) and CGI timeout (5s) to guarantee the server
  never blocks indefinitely

## Instructions

**Build:**
```bash
make
```
Compiled with `c++ -Wall -Wextra -Werror -std=c++98`. No external dependencies.

**Run:**
```bash
./webserv config.conf      # explicit config
./webserv                  # falls back to ./config.conf
```

**Example config (`config.conf`):**
```nginx
server {
    listen 8080;
    host 127.0.0.1;
    root ./www;
    client_max_body_size 1M;
    error_page 404 /errors/404.html;

    location / {
        methods GET POST DELETE;
        index index.html;
        autoindex on;
    }

    location /old {
        return 301 /;
    }

    location /upload {
        methods POST;
        upload_store ./www/uploads;
    }

    location /cgi-bin {
        methods GET POST;
        cgi_extension .py /usr/bin/python3;
    }
}
```

**Quick tests:**
```bash
curl http://127.0.0.1:8080/                                  # static
curl -F "file=@/etc/hostname" http://127.0.0.1:8080/upload   # upload
curl "http://127.0.0.1:8080/cgi-bin/hello.py?x=1"            # CGI
curl -X DELETE http://127.0.0.1:8080/upload/hostname         # delete
```

**Integration test suite (bash + curl):**
```bash
make
./webserv &
bash tests/run_all.sh
```
9 suites cover GET, POST/upload, DELETE, redirect, errors, autoindex, security,
concurrent connections and CGI.

**Makefile rules:** `all`, `clean`, `fclean`, `re`

## Resources

- [RFC 7230 – HTTP/1.1 Message Syntax and Routing](https://datatracker.ietf.org/doc/html/rfc7230)
- [RFC 7231 – HTTP/1.1 Semantics and Content](https://datatracker.ietf.org/doc/html/rfc7231)
- [RFC 3875 – The Common Gateway Interface (CGI) v1.1](https://datatracker.ietf.org/doc/html/rfc3875)
- [NGINX configuration documentation](https://nginx.org/en/docs/)
- `man 2 poll`, `man 2 socket`, `man 2 fcntl`, `man 2 fork`, `man 2 execve`

**AI usage:**
Claude (claude.ai/code) was used to assist with: (a) integration between the
config parser and the HTTP router, (b) the `poll()`-based multi-server event
loop with idle-connection timeout, (c) the CGI handler (`fork`/`execve`/pipes
with non-blocking I/O on both ends of the pipe, output parsing) and (d) the
autoindex directory listing. All generated code was reviewed, tested with the
integration suite in `tests/`, and understood by both authors before being
committed; CGI conformance was verified against the subject (no `errno` after
I/O, every `read`/`write` gated by `poll`, `fork` used only for CGI).
