/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:01:45 by andre             #+#    #+#             */
/*   Updated: 2026/04/24 17:07:47 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Router.hpp"
#include "../includes/File.hpp"

HttpResponse Router::handleRequest(const HttpRequest& request, const ServerConfig& config) {
  HttpResponse res;

  std::string path = request.path;

  if (path == "/")
    path = "/index.html";

  std::string fullPath = config.root + path;

  std::string content = readFile(fullPath);

  if (!content.empty()) {
    res.status = 200;
    res.body = content;
    res.contentType = "text/html";
  } else {
    res.status = 404;
    res.body = "<h1>404 Not Found</h1>";
    res.contentType = "text/html";
  }

  return res;
}
