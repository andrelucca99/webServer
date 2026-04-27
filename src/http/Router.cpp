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
#include <iostream>

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

// Router principal
HttpResponse Router::handleRequest(const HttpRequest& request, const ServerConfig& config) {
  HttpResponse res;

  std::cout << "[PATH RAW]: " << request.path << std::endl;

  // decode path
  std::string path = urlDecode(request.path);

  // default route
  if (path == "/" || path.empty())
    path = "/index.html";

  // remove leading '/'
  if (!path.empty() && path[0] == '/')
    path.erase(0, 1);

  // proteção
  if (path.find("..") != std::string::npos) {
    res.status = 403;
    res.body = "<h1>403 Forbidden</h1>";
    res.contentType = "text/html";
    return res;
  }

  // montar caminho
  std::string fullPath = config.root;
  if (!fullPath.empty() && fullPath[fullPath.size() - 1] != '/')
    fullPath += "/";
  fullPath += path;

  // POST
  if (request.method == "POST") {
    if (writeFile(fullPath, request.body)) {
      res.status = 201;
      res.body = "<h1>201 Created</h1>";
    } else {
      res.status = 500;
      res.body = "<h1>500 Internal Server Error</h1>";
    }

    res.contentType = "text/html";
    return res;
  }

  // GET
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
      res.body = "<h1>404 Not Found</h1>";
      res.contentType = "text/html";
    }

    return res;
  }

  // fallback
  res.status = 405;
  res.body = "<h1>405 Method Not Allowed</h1>";
  res.contentType = "text/html";

  return res;
}
