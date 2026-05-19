/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 19:12:34 by andre             #+#    #+#             */
/*   Updated: 2026/05/19 00:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Router.hpp"
#include "../includes/HttpRequest.hpp"
#include "../includes/HttpRequestParser.hpp"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <vector>
#include <map>

#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <signal.h>

#define BUFFER_SIZE       4096
#define POLL_TIMEOUT_MS   1000
#define CLIENT_TIMEOUT_S  30

struct ClientState {
    size_t      serverIdx;
    std::string readBuf;
    std::string writeBuf;
    bool        responseReady;
    time_t      lastActivity;
};

Server::Server(const Config& config) : _config(config) {}
Server::~Server() {}

static void setNonBlocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

// Parser simples de IPv4 ("A.B.C.D" -> uint32_t em network byte order).
// Subject nao autoriza inet_addr; usamos parsing manual.
static uint32_t parseIPv4(const std::string& host) {
    if (host.empty())
        return htonl(INADDR_ANY);
    unsigned int a = 0, b = 0, c = 0, d = 0;
    if (std::sscanf(host.c_str(), "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
        return htonl(INADDR_ANY);
    if (a > 255 || b > 255 || c > 255 || d > 255)
        return htonl(INADDR_ANY);
    return htonl((a << 24) | (b << 16) | (c << 8) | d);
}

static int createSocket(const ServerConfig& cfg) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::perror("socket");
        return -1;
    }

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(cfg.port != 0 ? cfg.port : 8080);
    addr.sin_addr.s_addr = parseIPv4(cfg.host);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::perror("bind");
        close(fd);
        return -1;
    }

    if (listen(fd, 128) < 0) {
        std::perror("listen");
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

    size_t contentLength = static_cast<size_t>(std::atoi(headers.c_str() + valStart));
    return buf.size() >= headerEnd + 4 + contentLength;
}

static bool isServerFd(const std::map<int, size_t>& serverFds, int fd, size_t& idx) {
    std::map<int, size_t>::const_iterator it = serverFds.find(fd);
    if (it == serverFds.end())
        return false;
    idx = it->second;
    return true;
}

static void removeFd(std::vector<pollfd>& fds, std::map<int, ClientState>& clients, size_t i) {
    close(fds[i].fd);
    clients.erase(fds[i].fd);
    fds.erase(fds.begin() + i);
}

static void buildResponse(ClientState& st, const ServerConfig& server) {
    HttpRequestParser parser;
    HttpRequest       request;
    HttpResponse      response;

    ParseStatus status = parser.parse(st.readBuf, request);
    if (status == PARSE_BAD_REQUEST) {
        response.status      = 400;
        response.body        = "<h1>400 Bad Request</h1>";
        response.contentType = "text/html";
    } else if (status == PARSE_HTTP_VERSION) {
        response.status      = 505;
        response.body        = "<h1>505 HTTP Version Not Supported</h1>";
        response.contentType = "text/html";
    } else {
        response = Router::handleRequest(request, server);
    }

    st.writeBuf      = response.build();
    st.readBuf.clear();
    st.responseReady = true;
}

void Server::run() {
    // Subject: lidar adequadamente com desconexoes. SIGPIPE em write() para
    // socket fechado mataria o processo; ignoramos para tratar via -1/EPIPE
    // pelo retorno da chamada (sem inspecionar errno).
    signal(SIGPIPE, SIG_IGN);

    // serverFds: fd -> indice em _config.servers (preserva o indice original
    // mesmo que algum bind falhe; bug latente R4 da auditoria).
    std::map<int, size_t>      serverFds;
    std::vector<pollfd>        fds;
    std::map<int, ClientState> clients;

    for (size_t i = 0; i < _config.servers.size(); i++) {
        int fd = createSocket(_config.servers[i]);
        if (fd < 0)
            continue;

        setNonBlocking(fd);

        int port = _config.servers[i].port != 0 ? _config.servers[i].port : 8080;
        std::cout << "Servidor " << i << " rodando na porta " << port << std::endl;

        serverFds[fd] = i;

        pollfd pfd;
        pfd.fd      = fd;
        pfd.events  = POLLIN;
        pfd.revents = 0;
        fds.push_back(pfd);
    }

    if (fds.empty()) {
        std::cerr << "Nenhum servidor iniciado." << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];

    while (true) {
        // Timeout finito para podermos varrer conexoes ociosas mesmo que
        // nenhum fd dispare evento. Subject: "servidor nunca deve travar".
        int ready = poll(&fds[0], static_cast<nfds_t>(fds.size()), POLL_TIMEOUT_MS);
        if (ready < 0) {
            // poll() pode retornar -1 com EINTR por sinal benigno; subject
            // proibe usar errno aqui, entao apenas reintegramos no loop.
            continue;
        }

        for (size_t i = 0; i < fds.size(); i++) {
            short revents = fds[i].revents;
            fds[i].revents = 0;

            // Erro/hangup em fd de cliente: limpa a conexao sem mais I/O.
            // Server fd nao deveria receber POLLHUP/POLLERR; se receber, ignora.
            size_t serverIdx = 0;
            bool isServer = isServerFd(serverFds, fds[i].fd, serverIdx);

            if (!isServer && (revents & (POLLERR | POLLNVAL))) {
                removeFd(fds, clients, i);
                i--;
                continue;
            }

            if (isServer) {
                if (!(revents & POLLIN))
                    continue;

                struct sockaddr_in clientAddr;
                socklen_t addrlen = sizeof(clientAddr);
                int clientFd = accept(fds[i].fd, (struct sockaddr*)&clientAddr, &addrlen);
                if (clientFd < 0)
                    continue;

                setNonBlocking(clientFd);

                ClientState st;
                st.serverIdx     = serverIdx;
                st.responseReady = false;
                st.lastActivity  = std::time(NULL);
                clients[clientFd] = st;

                pollfd cpfd;
                cpfd.fd      = clientFd;
                cpfd.events  = POLLIN;
                cpfd.revents = 0;
                fds.push_back(cpfd);
                continue;
            }

            // cliente
            ClientState& st = clients[fds[i].fd];

            if (revents & POLLIN) {
                ssize_t bytes = read(fds[i].fd, buffer, BUFFER_SIZE - 1);
                if (bytes <= 0) {
                    removeFd(fds, clients, i);
                    i--;
                    continue;
                }
                st.lastActivity = std::time(NULL);
                st.readBuf.append(buffer, static_cast<size_t>(bytes));

                if (!st.responseReady && isRequestComplete(st.readBuf)) {
                    buildResponse(st, _config.servers[st.serverIdx]);
                    // Resposta pronta: monitora R|W simultaneamente
                    // (subject: "poll() deve monitorar leitura e escrita
                    // simultaneamente"). POLLIN aqui detecta FIN do cliente.
                    fds[i].events = POLLIN | POLLOUT;
                }
            }

            if (st.responseReady && (revents & POLLOUT)) {
                ssize_t sent = send(fds[i].fd, st.writeBuf.c_str(), st.writeBuf.size(), 0);
                if (sent > 0) {
                    st.writeBuf.erase(0, static_cast<size_t>(sent));
                    st.lastActivity = std::time(NULL);
                }
                if (sent <= 0 || st.writeBuf.empty()) {
                    removeFd(fds, clients, i);
                    i--;
                    continue;
                }
            }

            // POLLHUP apos send pode indicar peer fechou; limpamos.
            if (revents & POLLHUP) {
                removeFd(fds, clients, i);
                i--;
                continue;
            }
        }

        // Varredura de connections ociosas (subject: "nunca travar
        // indefinidamente"). Cliente que conectou e nao envia request
        // completa em CLIENT_TIMEOUT_S segundos eh derrubado.
        time_t now = std::time(NULL);
        for (size_t i = 0; i < fds.size(); i++) {
            size_t dummy;
            if (isServerFd(serverFds, fds[i].fd, dummy))
                continue;
            std::map<int, ClientState>::iterator it = clients.find(fds[i].fd);
            if (it == clients.end())
                continue;
            if (now - it->second.lastActivity >= CLIENT_TIMEOUT_S) {
                removeFd(fds, clients, i);
                i--;
            }
        }
    }

    for (size_t i = 0; i < fds.size(); i++)
        close(fds[i].fd);
}
