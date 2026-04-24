/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 19:12:34 by andre             #+#    #+#             */
/*   Updated: 2026/04/24 17:07:25 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/HttpRequest.hpp"
#include "../includes/HttpRequestParser.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>

#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 4096

static std::string getContentType(const std::string &path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".css") != std::string::npos) return "text/css";
    if (path.find(".js") != std::string::npos) return "application/javascript";
    if (path.find(".png") != std::string::npos) return "image/png";
    if (path.find(".jpg") != std::string::npos) return "image/jpeg";
    if (path.find(".ico") != std::string::npos) return "image/x-icon";
    return "text/plain";
}

static std::string buildResponse(int status, const std::string &body, const std::string &contentType) {
    std::stringstream response;

    if (status == 200)
        response << "HTTP/1.1 200 OK\r\n";
    else
        response << "HTTP/1.1 404 Not Found\r\n";

    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << body;

    return response.str();
}

static bool readFile(const std::string &path, std::string &out) {
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;

    std::stringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return true;
}

static std::string resolvePath(const std::string &urlPath) {
    std::string path = "./www" + urlPath;

    if (urlPath == "/")
        path = "./www/index.html";
    else if (path[path.length() - 1] == '/')
        path += "index.html";

    return path;
}

Server::Server() {}
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

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        return;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        return;
    }

    std::cout << "Servidor rodando na porta " << PORT << "...\n";

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

        if (request.method != "GET") {
            std::string response = buildResponse(404, "<h1>404 Not Found</h1>", "text/html");
            send(client_socket, response.c_str(), response.size(), 0);
            close(client_socket);
            continue;
        }

        std::string path = resolvePath(request.path);

        std::string body;
        if (!readFile(path, body)) {
            std::string response = buildResponse(404, "<h1>404 Not Found</h1>", "text/html");
            send(client_socket, response.c_str(), response.size(), 0);
            close(client_socket);
            continue;
        }

        std::string response = buildResponse(200, body, getContentType(path));

        send(client_socket, response.c_str(), response.size(), 0);
        close(client_socket);
    }
}
