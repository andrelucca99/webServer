/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 19:12:34 by andre             #+#    #+#             */
/*   Updated: 2026/04/17 17:30:04 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Router.hpp"
#include "../includes/HttpRequest.hpp"
#include "../includes/HttpResponse.hpp"

#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sstream>

#define BUFFER_SIZE 4096

static void parseRequest(HttpRequest& req, const std::string& raw) {
    std::istringstream stream(raw);
    std::string line;

    if (std::getline(stream, line)) {
        std::istringstream firstLine(line);
        firstLine >> req.method >> req.uri >> req.http_version;

        size_t pos = req.uri.find('?');
        if (pos != std::string::npos) {
            req.path = req.uri.substr(0, pos);
            req.query_string = req.uri.substr(pos + 1);
        } else {
            req.path = req.uri;
        }
    }

    while (std::getline(stream, line)) {
        if (line == "\r" || line.empty())
            break;

        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (!value.empty() && value[0] == ' ')
                value.erase(0, 1);

            if (!value.empty() && value[value.size() - 1] == '\r')
                value.erase(value.size() - 1);

            req.headers[key] = value;
        }
    }

    std::string body;
    while (std::getline(stream, line)) {
        body += line;
    }
    req.body = body;
}

Server::Server(const ServerConfig& config)
    : _config(config), _socket(config.port, config.host) {}

void Server::run() {
    _socket.init();
    _socket.startListening();

    while (true) {
        int client_fd = _socket.acceptConnection();
        if (client_fd < 0)
            continue;

        char buffer[BUFFER_SIZE];
        std::memset(buffer, 0, BUFFER_SIZE);

        ssize_t bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes > 0) {
            std::string rawRequest(buffer);

            // DEBUG
            std::cout << "----- REQUEST RECEBIDA -----\n";
            std::cout << rawRequest << std::endl;
            std::cout << "----------------------------\n";

            HttpRequest request;
            parseRequest(request, rawRequest);

            HttpResponse response = Router::handleRequest(request);

            std::string responseStr = response.build();

            send(client_fd, responseStr.c_str(), responseStr.size(), 0);
        }

        close(client_fd);
    }
}
