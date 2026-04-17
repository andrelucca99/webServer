/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:12:36 by andre             #+#    #+#             */
/*   Updated: 2026/04/17 17:28:11 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpRequest.hpp"
#include <sstream>

void parseRequest(HttpRequest& req, const std::string& raw) {
  std::istringstream stream(raw);
  std::string line;

  if (std::getline(stream, line)) {
    std::istringstream firstLine(line);
    firstLine >> req.method >> req.uri >> req.http_version;

    size_t pos = req.uri.find('?');
    if (pos != std::string::npos) {
      req.path = req.uri.substr(0, pos);
      req.query_string = req.uri.substr(pos + 1);
    } else {
      req.path = req.uri;
    }
  }

  while (std::getline(stream, line)) {
    if (line == "\r" || line.empty())
      break;

    size_t pos = line.find(':');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);

      if (!value.empty() && value[0] == ' ')
        value.erase(0, 1);

      if (!value.empty() && value[value.size() - 1] == '\r')
        value.erase(value.size() - 1);

      req.headers[key] = value;
    }
  }

  std::string body;
  while (std::getline(stream, line)) {
    body += line;
  }

  req.body = body;
}
