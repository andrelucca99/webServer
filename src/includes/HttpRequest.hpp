/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>     +#+  +:+       +#+       */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 12:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/04/06 12:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <map>

struct HttpRequest {
    std::string method;                              // GET, POST, DELETE, etc
    std::string uri;                                 // /index.html ou /index.html?name=value
    std::string path;                                // /index.html (sem query string)
    std::string query_string;                        // name=value (sem ?)
    std::string http_version;                        // HTTP/1.1 ou HTTP/1.0
    std::map<std::string, std::string> headers;      // Host, Content-Type, Content-Length, etc
    std::string body;                                // POST/PUT data (vazio em GET/DELETE)

    HttpRequest() {}
};
