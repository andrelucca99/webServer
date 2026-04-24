/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:12:36 by andre             #+#    #+#             */
/*   Updated: 2026/04/24 17:08:43 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpRequest.hpp"
#include <sstream>
#include <algorithm>

void parseRequest(HttpRequest& req, const std::string& raw) {
  req = HttpRequest();

  std::istringstream stream(raw);
  std::string line;

  // ===== FIRST LINE =====
  if (std::getline(stream, line)) {
    if (!line.empty() && line[line.size() - 1] == '\r')
      line.erase(line.size() - 1);

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

  // ===== HEADERS =====
  while (std::getline(stream, line)) {
    if (line == "\r" || line.empty())
      break;

    if (!line.empty() && line[line.size() - 1] == '\r')
      line.erase(line.size() - 1);

    size_t pos = line.find(':');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);

      // lowercase header key
      std::transform(key.begin(), key.end(), key.begin(), ::tolower);

      // trim início
      while (!value.empty() && (value[0] == ' ' || value[0] == '\t'))
        value.erase(0, 1);

      // trim final
      while (!value.empty() && (value[value.size() - 1] == ' ' || value[value.size() - 1] == '\r'))
        value.erase(value.size() - 1);

      req.headers[key] = value;
    }
  }

  // ===== BODY =====
  std::string body;
  while (std::getline(stream, line)) {
    body += line + "\n";
  }

  req.body = body;
}
