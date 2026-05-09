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

    private:
        const HttpRequest&  _request;
        const RouteConfig&  _route;
        const ServerConfig& _server;
        std::string         _scriptPath;
        std::string         _interpreter;

        CgiHandler(const CgiHandler&);
        CgiHandler& operator=(const CgiHandler&);
};
