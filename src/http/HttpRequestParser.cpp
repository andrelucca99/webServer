/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>     +#+  +:+       +#+       */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 12:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/04/06 12:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpRequestParser.hpp"
#include <stdexcept>
#include <cctype>
#include <cstdlib>

HttpRequestParser::HttpRequestParser() {}

HttpRequestParser::HttpRequestParser(const HttpRequestParser& other) {
    *this = other;
}

HttpRequestParser& HttpRequestParser::operator=(const HttpRequestParser& other) {
    (void)other;
    return *this;
}

HttpRequestParser::~HttpRequestParser() {}

void HttpRequestParser::parseRequestLine(const std::string& line, HttpRequest& req) {
    std::size_t first = line.find(' ');
    if (first == std::string::npos)
        throw std::runtime_error("Invalid request line: " + line);

    std::size_t second = line.find(' ', first + 1);
    if (second == std::string::npos)
        throw std::runtime_error("Invalid request line: " + line);

    req.method       = line.substr(0, first);
    req.uri          = line.substr(first + 1, second - first - 1);
    req.http_version = line.substr(second + 1);

    if (req.http_version != "HTTP/1.0" && req.http_version != "HTTP/1.1")
        throw std::runtime_error("505 HTTP Version Not Supported: " + req.http_version);

    parseUri(req.uri, req);
}

void HttpRequestParser::parseUri(const std::string& uri, HttpRequest& req) {
    std::size_t pos = uri.find('?');
    if (pos == std::string::npos) {
        req.path         = uri;
        req.query_string = "";
    } else {
        req.path         = uri.substr(0, pos);
        req.query_string = uri.substr(pos + 1);
    }
}

void HttpRequestParser::parseHeaders(const std::string& raw_headers, HttpRequest& req) {
    std::size_t start = 0;
    while (start < raw_headers.size()) {
        std::size_t end = raw_headers.find("\r\n", start);
        if (end == std::string::npos)
            end = raw_headers.size();

        std::string header_line = raw_headers.substr(start, end - start);
        std::size_t colon = header_line.find(':');
        if (colon != std::string::npos) {
            std::string key   = header_line.substr(0, colon);
            std::string value = header_line.substr(colon + 1);
            std::size_t value_start = value.find_first_not_of(' ');
            if (value_start != std::string::npos)
                value = value.substr(value_start);
            for (std::size_t i = 0; i < key.size(); i++)
                key[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(key[i])));
            req.headers[key] = value;
        }

        if (end == raw_headers.size())
            break;
        start = end + 2;
    }
}

void HttpRequestParser::parseBody(const std::string& raw_body, HttpRequest& req) {
    req.body = raw_body;
}

HttpRequest HttpRequestParser::parse(const std::string& raw) {
    HttpRequest req;

    std::size_t request_line_end = raw.find("\r\n");
    if (request_line_end == std::string::npos)
        throw std::runtime_error("Malformed HTTP request: missing request line");

    std::string request_line = raw.substr(0, request_line_end);
    parseRequestLine(request_line, req);

    std::size_t headers_start = request_line_end + 2;
    std::size_t headers_end   = raw.find("\r\n\r\n", headers_start);
    if (headers_end == std::string::npos) {
        parseHeaders(raw.substr(headers_start), req);
        return req;
    }

    parseHeaders(raw.substr(headers_start, headers_end - headers_start), req);

    std::string full_body = raw.substr(headers_end + 4);
    std::map<std::string, std::string>::const_iterator it = req.headers.find("content-length");
    if (it != req.headers.end()) {
        std::size_t len = static_cast<std::size_t>(std::atoi(it->second.c_str()));
        parseBody(full_body.substr(0, len), req);
    } else {
        parseBody(full_body, req);
    }

    return req;
}
