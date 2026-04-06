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
    (void)line;
    (void)req;
}

void HttpRequestParser::parseUri(const std::string& uri, HttpRequest& req) {
    (void)uri;
    (void)req;
}

void HttpRequestParser::parseHeaders(const std::string& raw_headers, HttpRequest& req) {
    (void)raw_headers;
    (void)req;
}

void HttpRequestParser::parseBody(const std::string& raw_body, HttpRequest& req) {
    (void)raw_body;
    (void)req;
}

HttpRequest HttpRequestParser::parse(const std::string& raw) {
    (void)raw;
    return HttpRequest();
}
