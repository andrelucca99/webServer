/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequestParser.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 12:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/05/02 08:41:20 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <map>
#include "HttpRequest.hpp"

enum ParseStatus {
    PARSE_OK,
    PARSE_BAD_REQUEST,
    PARSE_HTTP_VERSION
};

class HttpRequestParser {
    private:
        bool isValidNumber(const std::string& str);

        ParseStatus parseRequestLine(const std::string& line, HttpRequest& req);
        ParseStatus parseHeaders(const std::string& raw_headers, HttpRequest& req);
        void parseUri(const std::string& uri, HttpRequest& req);
        void parseBody(const std::string& raw_body, HttpRequest& req);

    public:
        HttpRequestParser();
        HttpRequestParser(const HttpRequestParser& other);
        HttpRequestParser& operator=(const HttpRequestParser& other);
        ~HttpRequestParser();

        ParseStatus parse(const std::string& raw, HttpRequest& req);
};
