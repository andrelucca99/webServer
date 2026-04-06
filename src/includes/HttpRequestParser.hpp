/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>     +#+  +:+       +#+       */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 12:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/04/06 12:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include "HttpRequest.hpp"

class HttpRequestParser {
private:
    void parseRequestLine(const std::string& line, HttpRequest& req);
    void parseUri(const std::string& uri, HttpRequest& req);
    void parseHeaders(const std::string& raw_headers, HttpRequest& req);
    void parseBody(const std::string& raw_body, HttpRequest& req);

public:
    HttpRequest parse(const std::string& raw);
};
