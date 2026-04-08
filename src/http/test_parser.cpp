/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_parser.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>     +#+  +:+       +#+       */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/07 00:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/04/07 00:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpRequestParser.hpp"
#include <iostream>
#include <cassert>
#include <string>

static int g_passed = 0;
static int g_failed = 0;

static void ok(const std::string& name) {
    std::cout << "[PASS] " << name << "\n";
    g_passed++;
}

static void fail(const std::string& name, const std::string& msg) {
    std::cout << "[FAIL] " << name << " — " << msg << "\n";
    g_failed++;
}

#define CHECK(name, expr) \
    do { if (expr) ok(name); else fail(name, #expr); } while (0)

int main() {
    HttpRequestParser parser;

    // 1. GET simples
    {
        std::string raw = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
        HttpRequest r = parser.parse(raw);
        CHECK("GET method",        r.method       == "GET");
        CHECK("GET uri",           r.uri          == "/index.html");
        CHECK("GET path",          r.path         == "/index.html");
        CHECK("GET query_string",  r.query_string == "");
        CHECK("GET http_version",  r.http_version == "HTTP/1.1");
        CHECK("GET Host header",   r.headers.count("host") && r.headers.at("host") == "localhost");
        CHECK("GET body empty",    r.body         == "");
    }

    // 2. POST com body e Content-Length
    {
        std::string raw = "POST /upload HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello";
        HttpRequest r = parser.parse(raw);
        CHECK("POST method",           r.method == "POST");
        CHECK("POST path",             r.path   == "/upload");
        CHECK("POST body",             r.body   == "hello");
        CHECK("POST content-length",   r.headers.count("content-length"));
    }

    // 3. Multiplos headers
    {
        std::string raw = "GET / HTTP/1.1\r\nHost: example.com\r\nAccept: text/html\r\nConnection: keep-alive\r\n\r\n";
        HttpRequest r = parser.parse(raw);
        CHECK("Multi headers host",       r.headers.count("host")       && r.headers.at("host")       == "example.com");
        CHECK("Multi headers accept",     r.headers.count("accept")     && r.headers.at("accept")     == "text/html");
        CHECK("Multi headers connection", r.headers.count("connection") && r.headers.at("connection") == "keep-alive");
    }

    // 4. Request-line malformada
    {
        bool threw = false;
        try { parser.parse("BADREQUEST\r\n\r\n"); }
        catch (std::runtime_error&) { threw = true; }
        CHECK("Malformed request-line throws", threw);
    }

    // 5. URI com query string
    {
        std::string raw = "GET /search?q=hello&lang=pt HTTP/1.1\r\nHost: localhost\r\n\r\n";
        HttpRequest r = parser.parse(raw);
        CHECK("Query path",         r.path         == "/search");
        CHECK("Query query_string", r.query_string == "q=hello&lang=pt");
    }

    // 6. Versao HTTP invalida
    {
        bool threw = false;
        try { parser.parse("GET / HTTP/2.0\r\nHost: x\r\n\r\n"); }
        catch (std::runtime_error&) { threw = true; }
        CHECK("Invalid HTTP version throws", threw);
    }

    // 7. HTTP/1.0 aceito
    {
        std::string raw = "GET / HTTP/1.0\r\nHost: localhost\r\n\r\n";
        HttpRequest r = parser.parse(raw);
        CHECK("HTTP/1.0 accepted", r.http_version == "HTTP/1.0");
    }

    // 8. Content-Length limita body
    {
        std::string raw = "POST /data HTTP/1.1\r\nContent-Length: 3\r\n\r\nhello_extra";
        HttpRequest r = parser.parse(raw);
        CHECK("Body limited by Content-Length", r.body == "hel");
    }

    std::cout << "\n" << g_passed << " passed, " << g_failed << " failed\n";
    return g_failed > 0 ? 1 : 0;
}
