*Este projeto foi criado como parte do currículo da 42 por jtertuli, andrelucca99.*

---

## Description

Webserv is an HTTP/1.1 web server written in C++98, inspired by NGINX. It handles multiple simultaneous connections using a single `poll()` loop, supports multiple virtual servers on different ports, and serves static content with configurable routing.

Features:
- Non-blocking I/O via `poll()` (read and write)
- Multiple server blocks on distinct ports
- GET, POST and DELETE methods
- Per-route method restrictions (returns 405)
- Directory index file and autoindex listing
- Configurable `client_max_body_size` (returns 413)
- Configurable `error_page` per status code
- Path traversal protection

## Instructions

**Build:**
```bash
make
```

**Run:**
```bash
./webserv config.conf
```
If no argument is given, `config.conf` in the current directory is used.

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
}
```

**Test:**
```bash
curl http://127.0.0.1:8080/
curl -X POST http://127.0.0.1:8080/upload.txt -d "hello"
curl -X DELETE http://127.0.0.1:8080/upload.txt
```

**Makefile rules:** `all`, `clean`, `fclean`, `re`

## Resources

- [RFC 7230 – HTTP/1.1 Message Syntax](https://datatracker.ietf.org/doc/html/rfc7230)
- [RFC 7231 – HTTP/1.1 Semantics](https://datatracker.ietf.org/doc/html/rfc7231)
- [NGINX documentation](https://nginx.org/en/docs/)
- `man poll`, `man socket`, `man fcntl`

**AI usage:**  
Claude (claude.ai/code) was used to assist with the integration between the config parser and the HTTP router, implementation of the `poll()`-based multi-server event loop, and the autoindex directory listing feature. All generated code was reviewed, tested, and understood by both authors before being committed.
