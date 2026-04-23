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
#include <sstream>

HttpResponse::HttpResponse()
	: _http_version("HTTP/1.1"), _status_code(200), _reason_phrase(reasonPhraseFor(200)) {}

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

// --- Setters ---

void HttpResponse::setStatus(int code) {
	_status_code   = code;
	_reason_phrase = reasonPhraseFor(code);
}

void HttpResponse::setHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void HttpResponse::setBody(const std::string& content) {
	_body = content;
	std::ostringstream oss;
	oss << _body.size();
	setHeader("Content-Length", oss.str());
}

// --- Helpers estaticos ---

std::string HttpResponse::reasonPhraseFor(int code) {
	switch (code) {
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 400: return "Bad Request";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Method Not Allowed";
		case 413: return "Payload Too Large";
		case 500: return "Internal Server Error";
		case 505: return "HTTP Version Not Supported";
		default:  return "Unknown";
	}
}

std::string HttpResponse::mimeTypeFor(const std::string& extension) {
	if (extension == ".html") return "text/html";
	if (extension == ".css")  return "text/css";
	if (extension == ".js")   return "application/javascript";
	if (extension == ".json") return "application/json";
	if (extension == ".png")  return "image/png";
	if (extension == ".jpg")  return "image/jpeg";
	if (extension == ".jpeg") return "image/jpeg";
	if (extension == ".gif")  return "image/gif";
	if (extension == ".ico")  return "image/x-icon";
	if (extension == ".txt")  return "text/plain";
	return "application/octet-stream";
}
