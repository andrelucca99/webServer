/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_smoke.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*                                                     #+#    #+#             */
/*                                                    ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../src/includes/ConfigParser.hpp"
#include "../src/includes/HttpRequestParser.hpp"
#include "../src/includes/HttpRequest.hpp"
#include "../src/includes/Tokenizer.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

static int g_pass = 0;
static int g_fail = 0;

static void ok(const std::string& name) {
    std::cout << "  OK   " << name << std::endl;
    ++g_pass;
}

static void fail(const std::string& name, const std::string& why) {
    std::cerr << "  FAIL " << name << " :: " << why << std::endl;
    ++g_fail;
}

static void expect_parse_ok(const char* path, size_t expected_servers) {
    try {
        ConfigParser p;
        Config c = p.parse(path);
        if (c.servers.size() == expected_servers) {
            std::ostringstream tag;
            tag << path << " (" << expected_servers << " servers)";
            ok(tag.str());
        } else {
            std::ostringstream why;
            why << "esperado " << expected_servers
                << " servers, recebido " << c.servers.size();
            fail(path, why.str());
        }
    } catch (const std::exception& e) {
        fail(path, std::string("excecao inesperada: ") + e.what());
    }
}

static void expect_parse_throws(const char* path) {
    try {
        ConfigParser p;
        Config c = p.parse(path);
        (void)c;
        fail(path, "esperava excecao, parseou ok");
    } catch (const std::exception&) {
        ok(std::string(path) + " (lancou)");
    }
}

static void test_config_parser() {
    std::cout << "[ConfigParser]" << std::endl;
    expect_parse_ok("config.conf", 2);
    expect_parse_ok("tests/configs/valid_basic.conf", 1);
    expect_parse_ok("tests/configs/valid_multi.conf", 3);
    expect_parse_throws("tests/configs/invalid_missing_brace.conf");
    expect_parse_throws("tests/configs/invalid_unknown_directive.conf");
    expect_parse_throws("tests/configs/invalid_unknown_in_location.conf");
    expect_parse_throws("tests/configs/invalid_truncated.conf");

    expect_parse_ok("tests/configs/empty.conf", 0);
    expect_parse_throws("tests/configs/nonexistent.conf");
}

static void test_tokenizer() {
    std::cout << "[Tokenizer]" << std::endl;
    Tokenizer t;
    std::vector<std::string> toks = t.tokenize("server { listen 8080; }");
    if (toks.size() == 6 && toks[0] == "server" && toks[1] == "{" &&
        toks[2] == "listen" && toks[3] == "8080" && toks[4] == ";" &&
        toks[5] == "}") {
        ok("tokenize basico");
    } else {
        std::ostringstream why;
        why << "tokens=" << toks.size();
        fail("tokenize basico", why.str());
    }

    for (int i = 0; i < 5000; ++i) {
        std::vector<std::string> v = t.tokenize("a { b c; d e; }");
        (void)v;
    }
    ok("tokenize 5000x stress");
}

static std::string build_req(const std::string& method,
                             const std::string& path,
                             const std::string& body,
                             const std::string& extra = "") {
    std::ostringstream r;
    r << method << " " << path << " HTTP/1.1\r\n"
      << "Host: 127.0.0.1\r\n"
      << extra
      << "Content-Length: " << body.size() << "\r\n"
      << "\r\n"
      << body;
    return r.str();
}

static void test_http_request_parser() {
    std::cout << "[HttpRequestParser]" << std::endl;

    {
        HttpRequestParser p;
        HttpRequest req;
        ParseStatus s = p.parse(build_req("GET", "/", ""), req);
        if (s == PARSE_OK && req.method == "GET" && req.path == "/")
            ok("GET simples");
        else
            fail("GET simples", "parse falhou");
    }

    {
        HttpRequestParser p;
        HttpRequest req;
        std::string body = "name=value&foo=bar";
        std::string raw = build_req("POST", "/x", body,
            "Content-Type: application/x-www-form-urlencoded\r\n");
        ParseStatus s = p.parse(raw, req);
        if (s == PARSE_OK && req.method == "POST" && req.body == body)
            ok("POST com body");
        else
            fail("POST com body", "parse falhou");
    }

    {
        HttpRequestParser p;
        HttpRequest req;
        std::string raw =
            "POST /upload HTTP/1.1\r\n"
            "Host: 127.0.0.1\r\n"
            "Content-Type: multipart/form-data; boundary=XYZ\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
        ParseStatus s = p.parse(raw, req);
        if (s == PARSE_OK && req.isMultipart && req.boundary == "--XYZ")
            ok("multipart boundary");
        else
            fail("multipart boundary", "boundary nao detectada");
    }

    {
        HttpRequestParser p;
        HttpRequest req;
        ParseStatus s = p.parse("LIXO\r\n\r\n", req);
        if (s == PARSE_BAD_REQUEST) ok("malformed -> 400");
        else fail("malformed -> 400", "status inesperado");
    }

    {
        HttpRequestParser p;
        HttpRequest req;
        std::string raw = "GET / HTTP/2.0\r\nHost: x\r\n\r\n";
        ParseStatus s = p.parse(raw, req);
        if (s == PARSE_HTTP_VERSION) ok("HTTP/2 -> 505");
        else fail("HTTP/2 -> 505", "status inesperado");
    }

    {
        HttpRequestParser p;
        HttpRequest req;
        std::string raw = "GET / HTTP/1.1\r\n\r\n"; 
        ParseStatus s = p.parse(raw, req);
        if (s == PARSE_BAD_REQUEST) ok("sem Host -> 400");
        else fail("sem Host -> 400", "status inesperado");
    }

    for (int i = 0; i < 2000; ++i) {
        HttpRequestParser p;
        HttpRequest req;
        std::string body((i % 200) + 1, 'A');
        std::string raw = build_req("POST", "/x", body);
        p.parse(raw, req);
    }
    ok("2000 parses stress");
}

int main() {
    test_tokenizer();
    test_config_parser();
    test_http_request_parser();
    std::cout << "------------------------------" << std::endl;
    std::cout << g_pass << " passed, " << g_fail << " failed" << std::endl;
    return g_fail == 0 ? 0 : 1;
}
