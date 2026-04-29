/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 19:09:51 by andre             #+#    #+#             */
/*   Updated: 2026/04/01 19:17:56 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigParser.hpp"
#include "../includes/Tokenizer.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

static size_t parseSize(const std::string& s) {
    size_t val = static_cast<size_t>(atoi(s.c_str()));
    if (!s.empty()) {
        char last = s[s.size() - 1];
        if (last == 'M' || last == 'm') val *= 1024 * 1024;
        else if (last == 'K' || last == 'k') val *= 1024;
    }
    return val;
}

std::string ConfigParser::current() {
    if (pos >= tokens.size())
        throw std::runtime_error("Unexpected end of tokens");
    return tokens[pos];
}

void ConfigParser::advance() {
    pos++;
}

void ConfigParser::expect(const std::string& token) {
    if (current() != token)
        throw std::runtime_error("Expected: " + token + ", got: " + current());
    advance();
}

Config ConfigParser::parse(const std::string& filename) {
    std::ifstream file(filename.c_str());
    if (!file)
        throw std::runtime_error("Cannot open config file");

    std::stringstream buffer;
    buffer << file.rdbuf();

    Tokenizer tokenizer;
    tokens = tokenizer.tokenize(buffer.str());
    pos = 0;

    Config config;

    while (pos < tokens.size()) {
        if (current() == "server") {
            advance();
            config.servers.push_back(parseServer());
        } else {
            throw std::runtime_error("Unknown token: " + current());
        }
    }

    return config;
}

ServerConfig ConfigParser::parseServer() {
    ServerConfig server;

    expect("{");

    while (current() != "}") {
        if (current() == "listen") {
            advance();
            server.port = atoi(current().c_str());
            advance();
            expect(";");
        }
        else if (current() == "host") {
            advance();
            server.host = current();
            advance();
            expect(";");
        }
        else if (current() == "root") {
            advance();
            server.root = current();
            advance();
            expect(";");
        }
        else if (current() == "client_max_body_size") {
            advance();
            server.client_max_body_size = parseSize(current());
            advance();
            expect(";");
        }
        else if (current() == "error_page") {
            advance();
            int code = atoi(current().c_str());
            advance();
            server.error_pages[code] = current();
            advance();
            expect(";");
        }
        else if (current() == "location") {
            advance();
            server.routes.push_back(parseRoute());
        }
        else {
            throw std::runtime_error("Invalid directive in server: " + current());
        }
    }

    expect("}");
    return server;
}

RouteConfig ConfigParser::parseRoute() {
    RouteConfig route;

    route.path = current();
    advance();

    expect("{");

    while (current() != "}") {
        if (current() == "methods") {
            advance();

            while (current() != ";") {
                route.methods.push_back(current());
                advance();
            }
            expect(";");
        }
        else if (current() == "index") {
            advance();
            route.index = current();
            advance();
            expect(";");
        }
        else if (current() == "autoindex") {
            advance();
            route.autoindex = (current() == "on");
            advance();
            expect(";");
        }
        else {
            throw std::runtime_error("Invalid directive in location");
        }
    }

    expect("}");
    return route;
}