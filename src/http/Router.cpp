/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alucas-e <alucas-e@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:01:45 by alucas-e          #+#    #+#             */
/*   Updated: 2026/05/03 09:39:07 by alucas-e         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Router.hpp"
#include "../includes/CgiHandler.hpp"
#include "../includes/File.hpp"
#include "../includes/HttpError.hpp"
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <map>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

static std::string sanitizeFilename(const std::string& filename) {
    std::string clean;

    for (size_t i = 0; i < filename.size(); i++) {
        char c = filename[i];

        if (c == '/' || c == '\\')
            continue;

        clean += c;
    }

    if (clean.empty())
        clean = "upload.bin";

    return clean;
}

static std::string generateAutoindex(const std::string& dirPath, const std::string& uriPath) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) return "";

    std::ostringstream html;
    html << "<!DOCTYPE html><html><head><title>Index of " << uriPath << "</title></head>"
         << "<body><h1>Index of " << uriPath << "</h1><hr><pre>";

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        if (name == ".") continue;
        html << "<a href=\"" << name << "\">" << name << "</a>\n";
    }
    closedir(dir);

    html << "</pre><hr></body></html>";
    return html.str();
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

    if (route && route->redirect_code != 0) {
        res.status = route->redirect_code;
        res.location = route->redirect_url;
        res.body = "";
        res.contentType = "text/html";
        return res;
    }

    if (route && !isMethodAllowed(*route, request.method)) {
        res.status = 405;
        res.body = httpErrorBody(405, config);
        res.contentType = "text/html";
        return res;
    }

    if (config.client_max_body_size > 0 && request.body.size() > config.client_max_body_size) {
        res.status = 413;
        res.body = httpErrorBody(413, config);
        res.contentType = "text/html";
        return res;
    }

    if (!path.empty() && path[0] == '/')
        path.erase(0, 1);

    if (path.find("..") != std::string::npos) {
        res.status = 403;
        res.body = httpErrorBody(403, config);
        res.contentType = "text/html";
        return res;
    }

    std::string baseRoot = (route && !route->root.empty()) ? route->root : config.root;
    std::string fullPath = baseRoot;
    if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/')
        fullPath += "/";
    if (!path.empty())
        fullPath += path;

    if (route) {
        std::string interpreter = CgiHandler::matchCgi(*route, request.path);
        if (!interpreter.empty()) {
            CgiHandler cgi(request, *route, config, fullPath, interpreter);
            return cgi.execute();
        }
    }

    if (request.method == "DELETE") {
        struct stat st;
        if (stat(fullPath.c_str(), &st) != 0) {
            res.status = 404;
            res.body   = httpErrorBody(404, config);
        } else if (S_ISDIR(st.st_mode)) {
            res.status = 403;
            res.body   = httpErrorBody(403, config);
        } else if (access(fullPath.c_str(), W_OK) != 0) {
            res.status = 403;
            res.body   = httpErrorBody(403, config);
        } else if (std::remove(fullPath.c_str()) == 0) {
            res.status = 204;
            res.body   = "";
        } else {
            res.status = 403;
            res.body   = httpErrorBody(403, config);
        }
        res.contentType = "text/html";
        return res;
    }

    if (request.method == "POST") {

        if (!request.isMultipart) {
            res.status = 400;
            res.body = httpErrorBody(400, config);
            res.contentType = "text/html";
            return res;
        }

        if (request.boundary.empty()) {
            res.status = 400;
            res.body = httpErrorBody(400, config);
            res.contentType = "text/html";
            return res;
        }

        const std::string& body = request.body;
        const std::string& boundary = request.boundary;

        size_t pos = 0;
        bool saved = false;

        while (true) {
            size_t partStart = body.find(boundary, pos);
            if (partStart == std::string::npos)
                break;

            partStart += boundary.length();

            if (body.substr(partStart, 2) == "--")
                break;

            if (body.substr(partStart, 2) == "\r\n")
                partStart += 2;

            size_t headersEnd = body.find("\r\n\r\n", partStart);
            if (headersEnd == std::string::npos)
                break;

            std::string partHeaders = body.substr(partStart, headersEnd - partStart);

            size_t filenamePos = partHeaders.find("filename=\"");
            if (filenamePos == std::string::npos) {
                pos = headersEnd;
                continue;
            }

            filenamePos += 10;
            size_t filenameEnd = partHeaders.find("\"", filenamePos);

            std::string filename = partHeaders.substr(filenamePos, filenameEnd - filenamePos);
            filename = sanitizeFilename(filename);

            size_t dataStart = headersEnd + 4;
            size_t nextBoundary = body.find(boundary, dataStart);

            if (nextBoundary == std::string::npos)
                break;

            size_t dataEnd = nextBoundary;

            if (body.substr(dataEnd - 2, 2) == "\r\n")
                dataEnd -= 2;

            std::string fileData = body.substr(dataStart, dataEnd - dataStart);

            std::string uploadDir = (route && !route->upload_store.empty())
                ? route->upload_store
                : config.root;
            if (!uploadDir.empty() && uploadDir[uploadDir.size() - 1] == '/')
                uploadDir.erase(uploadDir.size() - 1);
            std::string uploadPath = uploadDir + "/" + filename;

            if (!writeFile(uploadPath, fileData)) {
                res.status = 500;
                res.body = httpErrorBody(500, config);
                res.contentType = "text/html";
                return res;
            }

            saved = true;
            pos = nextBoundary;
        }

        if (!saved) {
            res.status = 400;
            res.body = httpErrorBody(400, config);
            res.contentType = "text/html";
            return res;
        }

        res.status = 201;
        res.body = "<h1>File Uploaded</h1>";
        res.contentType = "text/html";
        return res;
    }

    if (request.method == "GET") {
        struct stat st;
        if (stat(fullPath.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            std::string idx = (route && !route->index.empty()) ? route->index : "index.html";
            std::string idxPath = fullPath;
            if (!idxPath.empty() && idxPath[idxPath.size() - 1] != '/') idxPath += "/";
            idxPath += idx;

            std::string content = readFile(idxPath);
            if (!content.empty()) {
                res.status = 200;
                res.body = content;
                res.contentType = "text/html";
                return res;
            }

            if (route && route->autoindex) {
                res.status = 200;
                res.body = generateAutoindex(fullPath, request.path);
                res.contentType = "text/html";
                return res;
            }

            res.status = 403;
            res.body = httpErrorBody(403, config);
            res.contentType = "text/html";
            return res;
        }

        std::string content = readFile(fullPath);
        if (!content.empty()) {
            res.status = 200;
            res.body = content;
            size_t dot = fullPath.find_last_of('.');
            std::string ext = (dot != std::string::npos) ? fullPath.substr(dot) : "";
            res.contentType = HttpResponse::mimeTypeFor(ext);
        } else {
            res.status = 404;
            res.body = httpErrorBody(404, config);
            res.contentType = "text/html";
        }
        return res;
    }

    res.status = 405;
    res.body = httpErrorBody(405, config);
    res.contentType = "text/html";
    return res;
}
