/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 00:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/05/09 00:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/CgiHandler.hpp"

CgiHandler::CgiHandler(const HttpRequest&  request,
                       const RouteConfig&  route,
                       const ServerConfig& server,
                       const std::string&  scriptPath,
                       const std::string&  interpreter)
    : _request(request),
      _route(route),
      _server(server),
      _scriptPath(scriptPath),
      _interpreter(interpreter) {}

CgiHandler::~CgiHandler() {}

HttpResponse CgiHandler::execute() {
    HttpResponse res;
    res.status = 501;
    res.body = "<h1>501 CGI not implemented yet</h1>";
    res.contentType = "text/html";
    return res;
}
