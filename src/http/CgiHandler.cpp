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
#include <map>

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

std::string CgiHandler::matchCgi(const RouteConfig& route, const std::string& path) {
    typedef std::map<std::string, std::string>::const_iterator It;
    for (It it = route.cgi_extensions.begin(); it != route.cgi_extensions.end(); ++it) {
        const std::string& ext = it->first;
        if (ext.empty() || ext.size() > path.size())
            continue;
        size_t pos = path.rfind(ext);
        if (pos == std::string::npos)
            continue;
        size_t after = pos + ext.size();
        if (after == path.size() || path[after] == '/')
            return it->second;
    }
    return "";
}

HttpResponse CgiHandler::execute() {
    HttpResponse res;
    res.status = 501;
    res.body = "<h1>501 CGI not implemented yet</h1>";
    res.contentType = "text/html";
    return res;
}
