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
#include <cstdlib>
#include <vector>
#include <map>

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>

#define BUFFER_SIZE 4096

struct ClientState {
    size_t      serverIdx;
    std::string readBuf;
    std::string writeBuf;
};

Server::Server(const Config& config) : _config(config) {}
Server::~Server() {}

static void setNonBlocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

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

    if (listen(fd, 128) < 0) {
        perror("listen");
        close(fd);
        return -1;
    }

    return fd;
}

static bool isRequestComplete(const std::string& buf) {
    size_t headerEnd = buf.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return false;

    std::string headers = buf.substr(0, headerEnd);

    size_t clPos = headers.find("Content-Length:");
    if (clPos == std::string::npos)
        clPos = headers.find("content-length:");
    if (clPos == std::string::npos)
        return true;

    size_t colon = headers.find(':', clPos);
    size_t valStart = headers.find_first_not_of(" \t", colon + 1);
    if (valStart == std::string::npos)
        return true;

    size_t contentLength = static_cast<size_t>(atoi(headers.c_str() + valStart));
    return buf.size() >= headerEnd + 4 + contentLength;
}

static bool isServerFd(const std::vector<int>& serverFds, int fd, size_t& idx) {
    for (size_t j = 0; j < serverFds.size(); j++) {
        if (serverFds[j] == fd) {
            idx = j;
            return true;
        }
    }
    return false;
}

static void removeFd(std::vector<pollfd>& fds, std::map<int, ClientState>& clients, size_t i) {
    close(fds[i].fd);
    clients.erase(fds[i].fd);
    fds.erase(fds.begin() + i);
}

void Server::run() {
    std::vector<int>    serverFds;
    std::vector<pollfd> fds;
    std::map<int, ClientState> clients;

    for (size_t i = 0; i < _config.servers.size(); i++) {
        int fd = createSocket(_config.servers[i]);
        if (fd < 0)
            continue;

        setNonBlocking(fd);

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
            if (!fds[i].revents)
                continue;

            size_t serverIdx = 0;
            if (isServerFd(serverFds, fds[i].fd, serverIdx)) {
                if (!(fds[i].revents & POLLIN))
                    continue;

                struct sockaddr_in clientAddr;
                socklen_t addrlen = sizeof(clientAddr);
                int clientFd = accept(fds[i].fd, (struct sockaddr*)&clientAddr, &addrlen);
                if (clientFd < 0)
                    continue;

                setNonBlocking(clientFd);

                ClientState st;
                st.serverIdx = serverIdx;
                clients[clientFd] = st;

                pollfd cpfd;
                cpfd.fd = clientFd;
                cpfd.events = POLLIN;
                cpfd.revents = 0;
                fds.push_back(cpfd);
            } else {
                ClientState& st = clients[fds[i].fd];

                if (fds[i].revents & POLLIN) {
                    ssize_t bytes = read(fds[i].fd, buffer, BUFFER_SIZE - 1);
                    if (bytes <= 0) {
                        removeFd(fds, clients, i);
                        i--;
                        continue;
                    }

                    st.readBuf.append(buffer, bytes);

                    if (isRequestComplete(st.readBuf)) {
                        HttpRequestParser parser;
                        HttpRequest request = parser.parse(st.readBuf);
                        HttpResponse response = Router::handleRequest(request, _config.servers[st.serverIdx]);
                        st.writeBuf = response.build();
                        fds[i].events = POLLOUT;
                    }
                } else if (fds[i].revents & POLLOUT) {
                    ssize_t sent = send(fds[i].fd, st.writeBuf.c_str(), st.writeBuf.size(), 0);
                    if (sent > 0)
                        st.writeBuf.erase(0, static_cast<size_t>(sent));

                    if (sent <= 0 || st.writeBuf.empty()) {
                        removeFd(fds, clients, i);
                        i--;
                    }
                } else {
                    removeFd(fds, clients, i);
                    i--;
                }
            }
        }
    }

    for (size_t i = 0; i < fds.size(); i++)
        close(fds[i].fd);
}
