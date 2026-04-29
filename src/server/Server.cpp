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
#include <cstring>
#include <vector>
#include <map>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#define BUFFER_SIZE 4096

Server::Server(const Config& config) : _config(config) {}
Server::~Server() {}

static int createSocket(const ServerConfig& cfg) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(cfg.port != 0 ? cfg.port : 8080);
    addr.sin_addr.s_addr = cfg.host.empty() ? INADDR_ANY : inet_addr(cfg.host.c_str());

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return -1;
    }

    if (listen(fd, 10) < 0) {
        perror("listen");
        close(fd);
        return -1;
    }

    return fd;
}

void Server::run() {
    std::vector<int> serverFds;
    std::vector<pollfd> fds;
    std::map<int, size_t> clientToServer;

    for (size_t i = 0; i < _config.servers.size(); i++) {
        int fd = createSocket(_config.servers[i]);
        if (fd < 0)
            continue;

        int port = _config.servers[i].port != 0 ? _config.servers[i].port : 8080;
        std::cout << "Servidor " << i << " rodando na porta " << port << std::endl;

        serverFds.push_back(fd);

        pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        pfd.revents = 0;
        fds.push_back(pfd);
    }

    if (fds.empty()) {
        std::cerr << "Nenhum servidor iniciado." << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];

    while (true) {
        int ready = poll(&fds[0], static_cast<nfds_t>(fds.size()), -1);
        if (ready < 0) {
            perror("poll");
            break;
        }

        for (size_t i = 0; i < fds.size(); i++) {
            if (!(fds[i].revents & POLLIN))
                continue;

            // check if it's a listening server socket
            size_t serverIdx = serverFds.size();
            for (size_t j = 0; j < serverFds.size(); j++) {
                if (fds[i].fd == serverFds[j]) {
                    serverIdx = j;
                    break;
                }
            }

            if (serverIdx < serverFds.size()) {
                // new connection
                struct sockaddr_in clientAddr;
                socklen_t addrlen = sizeof(clientAddr);
                int clientFd = accept(fds[i].fd, (struct sockaddr*)&clientAddr, &addrlen);
                if (clientFd < 0) {
                    perror("accept");
                    continue;
                }

                clientToServer[clientFd] = serverIdx;

                pollfd cpfd;
                cpfd.fd = clientFd;
                cpfd.events = POLLIN;
                cpfd.revents = 0;
                fds.push_back(cpfd);
            } else {
                // client request
                size_t idx = clientToServer[fds[i].fd];

                memset(buffer, 0, BUFFER_SIZE);
                ssize_t bytes = read(fds[i].fd, buffer, BUFFER_SIZE - 1);

                if (bytes <= 0) {
                    close(fds[i].fd);
                    clientToServer.erase(fds[i].fd);
                    fds.erase(fds.begin() + i);
                    i--;
                    continue;
                }

                std::string rawRequest(buffer, bytes);
                HttpRequestParser parser;
                HttpRequest request = parser.parse(rawRequest);

                HttpResponse response = Router::handleRequest(request, _config.servers[idx]);
                std::string responseStr = response.build();

                send(fds[i].fd, responseStr.c_str(), responseStr.size(), 0);
                close(fds[i].fd);
                clientToServer.erase(fds[i].fd);
                fds.erase(fds.begin() + i);
                i--;
            }
        }
    }

    for (size_t i = 0; i < fds.size(); i++)
        close(fds[i].fd);
}
