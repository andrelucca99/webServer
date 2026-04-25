/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:01:45 by andre             #+#    #+#             */
/*   Updated: 2026/04/25 10:15:33 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Router.hpp"
#include "../includes/File.hpp"
#include <sstream>
#include <iostream>

#include <limits.h>
#include <stdlib.h>

// protege contra path traversal
static bool isPathInsideRoot(const std::string& root, const std::string& path) {
    char resolvedRoot[PATH_MAX];
    char resolvedPath[PATH_MAX];

    if (!realpath(root.c_str(), resolvedRoot))
      return false;

    if (!realpath(path.c_str(), resolvedPath))
      return true;

    std::string rootStr(resolvedRoot);
    std::string pathStr(resolvedPath);

    return pathStr.find(rootStr) == 0;
}

// pega extensão do arquivo
static std::string getExtension(const std::string& path) {
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
      return "";
    return path.substr(dot);
}

HttpResponse Router::handleRequest(const HttpRequest& request, const ServerConfig& config) {
  HttpResponse res;

  std::string path = request.path;

  std::cout << "[ROUTER] path: " << request.path << std::endl;

  // default route
  if (path == "/")
    path = "/index.html";

  std::string fullPath = config.root + path;

  if (!isPathInsideRoot(config.root, fullPath)) {
    res.status = 403;
    res.body = "<h1>403 Forbidden</h1>";
    res.contentType = "text/html";
    return res;
  }

  std::string content = readFile(fullPath);

  // melhor verificação
  if (!content.empty()) {
    res.status = 200;
    res.body = content;

    std::string ext = getExtension(fullPath);
    res.contentType = HttpResponse::mimeTypeFor(ext);
  } else {
    res.status = 404;
    res.body = "<h1>404 Not Found</h1>";
    res.contentType = "text/html";
  }

  return res;
}
