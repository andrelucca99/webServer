/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>     +#+  +:+       +#+       */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 12:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/04/10 12:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse()
	: _http_version("HTTP/1.1"), _status_code(200), _reason_phrase("OK") {}

HttpResponse::HttpResponse(const HttpResponse& other)
	: _http_version(other._http_version),
	  _status_code(other._status_code),
	  _reason_phrase(other._reason_phrase),
	  _headers(other._headers),
	  _body(other._body) {}

HttpResponse& HttpResponse::operator=(const HttpResponse& other) {
	if (this != &other) {
		_http_version  = other._http_version;
		_status_code   = other._status_code;
		_reason_phrase = other._reason_phrase;
		_headers       = other._headers;
		_body          = other._body;
	}
	return *this;
}

HttpResponse::~HttpResponse() {}
