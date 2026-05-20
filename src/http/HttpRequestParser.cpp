/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alucas-e <alucas-e@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 12:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/05/03 09:23:55 by alucas-e         ###   ########.fr       */
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

bool HttpRequestParser::isValidNumber(const std::string& str) {
    for (size_t i = 0; i < str.size(); i++) {
        if (!std::isdigit(str[i]))
            return false;
    }
    return !str.empty();
}

ParseStatus HttpRequestParser::parseRequestLine(const std::string& line, HttpRequest& req) {
    size_t first = line.find(' ');
    size_t second = line.find(' ', first + 1);

    if (first == std::string::npos || second == std::string::npos)
        return PARSE_BAD_REQUEST;

    req.method = line.substr(0, first);
    req.uri = line.substr(first + 1, second - first - 1);
    req.http_version = line.substr(second + 1);

    if (req.http_version != "HTTP/1.1" && req.http_version != "HTTP/1.0")
        return PARSE_HTTP_VERSION;

    parseUri(req.uri, req);
    return PARSE_OK;
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

ParseStatus HttpRequestParser::parseHeaders(const std::string& raw, HttpRequest& req) {
    size_t start = 0;

    while (start < raw.size()) {
        size_t end = raw.find("\r\n", start);
        if (end == std::string::npos)
            end = raw.size();

        std::string line = raw.substr(start, end - start);

        if (line.empty())
            break;

        size_t colon = line.find(':');
        if (colon == std::string::npos)
            return PARSE_BAD_REQUEST;

        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);

        while (!value.empty() && value[0] == ' ')
            value.erase(0, 1);

        if (!value.empty() && value[value.size() - 1] == '\r')
            value.erase(value.size() - 1);

        for (size_t i = 0; i < key.size(); i++)
            key[i] = static_cast<char>(std::tolower(key[i]));

        req.headers[key] = value;

        if (key == "content-type") {
            if (value.find("multipart/form-data") != std::string::npos) {

                size_t pos = value.find("boundary=");
                if (pos != std::string::npos) {

                    size_t end = value.find(';', pos);
                    std::string boundaryValue;

                    if (end != std::string::npos)
                        boundaryValue = value.substr(pos + 9, end - (pos + 9));
                    else
                        boundaryValue = value.substr(pos + 9);

                    req.isMultipart = true;
                    req.boundary = "--" + boundaryValue;
                }
            }
        }

        if (end == raw.size())
            break;

        start = end + 2;
    }

    return PARSE_OK;
}

void HttpRequestParser::parseBody(const std::string& raw_body, HttpRequest& req) {
    req.body = raw_body;
}

ParseStatus HttpRequestParser::parse(const std::string& raw, HttpRequest& req) {
    size_t requestLineEnd = raw.find("\r\n");
    if (requestLineEnd == std::string::npos)
        return PARSE_BAD_REQUEST;

    ParseStatus status = parseRequestLine(raw.substr(0, requestLineEnd), req);
    if (status != PARSE_OK)
        return status;

    size_t headersStart = requestLineEnd + 2;
    size_t headersEnd = raw.find("\r\n\r\n");

    if (headersEnd == std::string::npos)
        return PARSE_BAD_REQUEST;

    status = parseHeaders(raw.substr(headersStart, headersEnd - headersStart), req);
    if (status != PARSE_OK)
        return status;

    std::string body = raw.substr(headersEnd + 4);

    std::map<std::string, std::string>::iterator it = req.headers.find("content-length");
    if (it != req.headers.end()) {
        if (!isValidNumber(it->second))
            return PARSE_BAD_REQUEST;

        size_t len = static_cast<size_t>(atoi(it->second.c_str()));
        if (body.size() < len)
            return PARSE_BAD_REQUEST;

        req.body = body.substr(0, len);
    } else {
        req.body = body;
    }

    if (req.http_version == "HTTP/1.1") {
        std::map<std::string, std::string>::iterator hostIt = req.headers.find("host");

        if (hostIt == req.headers.end())
            return PARSE_BAD_REQUEST;

        std::string host = hostIt->second;

        while (!host.empty() && std::isspace(host[0]))
            host.erase(0, 1);

        while (!host.empty() && std::isspace(host[host.size() - 1]))
            host.erase(host.size() - 1);

        if (host.empty())
            return PARSE_BAD_REQUEST;
    }

    return PARSE_OK;
}
