/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 19:12:34 by andre             #+#    #+#             */
/*   Updated: 2026/04/15 19:36:03 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sys/socket.h>

#define BUFFER_SIZE 4096

Server::Server(const ServerConfig& config)
    : _config(config), _socket(config.port, config.host) {}

void Server::run() {
    _socket.init();
    _socket.startListening();

    while (true) {
        int client_fd = _socket.acceptConnection();
        if (client_fd >= 0) {

            char buffer[BUFFER_SIZE];
            std::memset(buffer, 0, BUFFER_SIZE);

            ssize_t bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

            if (bytes > 0) {
                std::cout << "----- REQUEST RECEBIDA -----\n";
                std::cout << buffer << std::endl;
                std::cout << "----------------------------\n";
            }

            const char* response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 12\r\n"
                "\r\n"
                "Hello World!";

            send(client_fd, response, strlen(response), 0);

            close(client_fd);
        }
    }
}
