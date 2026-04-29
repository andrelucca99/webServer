/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:01:45 by andre             #+#    #+#             */
/*   Updated: 2026/04/27 06:36:37 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Router.hpp"
#include "../includes/File.hpp"
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <map>

static std::string errorBody(int status, const ServerConfig& config) {
    std::map<int, std::string>::const_iterator it = config.error_pages.find(status);
    if (it != config.error_pages.end()) {
        std::string ep = it->second;
        std::string path = config.root;
        if (!path.empty() && path[path.size() - 1] != '/') path += "/";
        if (!ep.empty() && ep[0] == '/') ep.erase(0, 1);
        std::string content = readFile(path + ep);
        if (!content.empty()) return content;
    }
    std::ostringstream oss;
    oss << "<h1>" << status << " " << HttpResponse::reasonPhraseFor(status) << "</h1>";
    return oss.str();
}

static std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '%' && i + 2 < str.length()) {
            std::string hex = str.substr(i + 1, 2);
            char ch = static_cast<char>(strtol(hex.c_str(), NULL, 16));
            result += ch;
            i += 2;
        } else {
            result += str[i];
        }
    }
    return result;
}

static const RouteConfig* findRoute(const std::vector<RouteConfig>& routes, const std::string& path) {
    const RouteConfig* best = NULL;
    size_t bestLen = 0;
    for (size_t i = 0; i < routes.size(); i++) {
        const std::string& rpath = routes[i].path;
        if (path.size() >= rpath.size() && path.substr(0, rpath.size()) == rpath && rpath.size() >= bestLen) {
            bestLen = rpath.size();
            best = &routes[i];
        }
    }
    return best;
}

static bool isMethodAllowed(const RouteConfig& route, const std::string& method) {
    if (route.methods.empty())
        return true;
    for (size_t i = 0; i < route.methods.size(); i++) {
        if (route.methods[i] == method)
            return true;
    }
    return false;
}

HttpResponse Router::handleRequest(const HttpRequest& request, const ServerConfig& config) {
    HttpResponse res;

    std::cout << "[" << request.method << "] " << request.path << std::endl;

    std::string path = urlDecode(request.path);

    const RouteConfig* route = findRoute(config.routes, path);

    if (route && !isMethodAllowed(*route, request.method)) {
        res.status = 405;
        res.body = errorBody(405, config);
        res.contentType = "text/html";
        return res;
    }

    if (config.client_max_body_size > 0 && request.body.size() > config.client_max_body_size) {
        res.status = 413;
        res.body = errorBody(413, config);
        res.contentType = "text/html";
        return res;
    }

    if (!path.empty() && path[0] == '/')
        path.erase(0, 1);

    if (path.find("..") != std::string::npos) {
        res.status = 403;
        res.body = errorBody(403, config);
        res.contentType = "text/html";
        return res;
    }

    std::string fullPath = config.root;
    if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/')
        fullPath += "/";
    if (!path.empty())
        fullPath += path;

    if (request.method == "DELETE") {
        if (std::remove(fullPath.c_str()) == 0) {
            res.status = 204;
            res.body = "";
        } else {
            res.status = 404;
            res.body = errorBody(404, config);
        }
        res.contentType = "text/html";
        return res;
    }

    if (request.method == "POST") {
        if (writeFile(fullPath, request.body)) {
            res.status = 201;
            res.body = "<h1>201 Created</h1>";
        } else {
            res.status = 500;
            res.body = errorBody(500, config);
        }
        res.contentType = "text/html";
        return res;
    }

    if (request.method == "GET") {
        std::string content = readFile(fullPath);
        if (!content.empty()) {
            res.status = 200;
            res.body = content;
            size_t dot = fullPath.find_last_of('.');
            std::string ext = (dot != std::string::npos) ? fullPath.substr(dot) : "";
            res.contentType = HttpResponse::mimeTypeFor(ext);
        } else {
            res.status = 404;
            res.body = errorBody(404, config);
            res.contentType = "text/html";
        }
        return res;
    }

    res.status = 405;
    res.body = errorBody(405, config);
    res.contentType = "text/html";
    return res;
}
