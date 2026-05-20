/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/09 00:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/05/09 00:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <map>
#include <string>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "RouteConfig.hpp"
#include "ServerConfig.hpp"

class CgiHandler {
    public:
        CgiHandler(const HttpRequest&  request,
                   const RouteConfig&  route,
                   const ServerConfig& server,
                   const std::string&  scriptPath,
                   const std::string&  interpreter);
        ~CgiHandler();

        HttpResponse execute();

        static std::string matchCgi(const RouteConfig& route,
                                    const std::string& path);

    private:
        const HttpRequest&  _request;
        const RouteConfig&  _route;
        const ServerConfig& _server;
        std::string         _scriptPath;   
        std::string         _interpreter;
        std::string         _scriptName;   
        std::string         _pathInfo;     

        std::map<std::string, std::string> _buildEnv() const;
        HttpResponse                       _errorResponse(int status) const;
        bool                               _parseOutput(const std::string& raw,
                                                        HttpResponse&      res) const;

        CgiHandler(const CgiHandler&);
        CgiHandler& operator=(const CgiHandler&);
};
