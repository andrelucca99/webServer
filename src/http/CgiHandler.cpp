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
#include <cctype>
#include <map>
#include <sstream>

static std::string headerToEnvKey(const std::string& name) {
    std::string out = "HTTP_";
    for (size_t i = 0; i < name.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(name[i]);
        out += (c == '-') ? '_' : static_cast<char>(std::toupper(c));
    }
    return out;
}

static std::string itos(long long n) {
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

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

std::map<std::string, std::string> CgiHandler::_buildEnv() const {
    std::map<std::string, std::string> env;

    env["GATEWAY_INTERFACE"] = "CGI/1.1";
    env["SERVER_PROTOCOL"]   = _request.http_version.empty() ? "HTTP/1.1"
                                                             : _request.http_version;
    env["SERVER_NAME"]       = _server.host;
    env["SERVER_PORT"]       = itos(_server.port);
    env["REQUEST_METHOD"]    = _request.method;
    env["SCRIPT_NAME"]       = _request.path;
    env["SCRIPT_FILENAME"]   = _scriptPath;
    env["PATH_INFO"]         = _request.path;
    env["QUERY_STRING"]      = _request.query_string;
    env["REDIRECT_STATUS"]   = "200";

    if (!_request.body.empty())
        env["CONTENT_LENGTH"] = itos(static_cast<long long>(_request.body.size()));

    typedef std::map<std::string, std::string>::const_iterator It;
    It ct = _request.headers.find("content-type");
    if (ct != _request.headers.end())
        env["CONTENT_TYPE"] = ct->second;

    for (It it = _request.headers.begin(); it != _request.headers.end(); ++it) {
        env[headerToEnvKey(it->first)] = it->second;
    }

    return env;
}

HttpResponse CgiHandler::execute() {
    std::map<std::string, std::string> env = _buildEnv();
    (void)env;

    HttpResponse res;
    res.status = 501;
    res.body = "<h1>501 CGI not implemented yet</h1>";
    res.contentType = "text/html";
    return res;
}
