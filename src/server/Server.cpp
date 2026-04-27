/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 19:12:34 by andre             #+#    #+#             */
/*   Updated: 2026/04/27 06:00:12 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Router.hpp"
#include "../includes/HttpRequest.hpp"
#include "../includes/HttpRequestParser.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 4096

Server::Server(const ServerConfig& config) : config(config) {}
Server::~Server() {}

void Server::run() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        return;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int port = config.port != 0 ? config.port : 8080;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = config.host.empty() ? INADDR_ANY : inet_addr(config.host.c_str());
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return;
    }

    std::cout << "Servidor rodando na porta " << port << "...\n";

    while (true) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        memset(buffer, 0, BUFFER_SIZE);
        read(client_socket, buffer, BUFFER_SIZE);

        std::string rawRequest(buffer);
        HttpRequestParser parser;
        HttpRequest request = parser.parse(rawRequest);

        HttpResponse response = Router::handleRequest(request, config);

        std::string responseStr = response.build();

        send(client_socket, responseStr.c_str(), responseStr.size(), 0);
        close(client_socket);
    }
}
