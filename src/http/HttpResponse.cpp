/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 12:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/04/29 00:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpResponse.hpp"
#include <sstream>

HttpResponse::HttpResponse()
	: status(200), contentType("text/html") {}

HttpResponse::HttpResponse(const HttpResponse& other)
	: status(other.status),
	  body(other.body),
	  contentType(other.contentType),
	  location(other.location) {}

HttpResponse& HttpResponse::operator=(const HttpResponse& other) {
	if (this != &other) {
		status      = other.status;
		body        = other.body;
		contentType = other.contentType;
		location    = other.location;
	}
	return *this;
}

HttpResponse::~HttpResponse() {}

std::string HttpResponse::build() const {
	std::ostringstream oss;

	oss << "HTTP/1.1 " << status << " " << reasonPhraseFor(status) << "\r\n";
	if (!location.empty())
		oss << "Location: " << location << "\r\n";
	oss << "Content-Type: " << contentType << "\r\n";
	oss << "Content-Length: " << body.size() << "\r\n";
	oss << "Connection: close\r\n";
	oss << "\r\n";
	oss << body;

	return oss.str();
}

std::string HttpResponse::reasonPhraseFor(int code) {
	switch (code) {
		case 200: return "OK";
		case 201: return "Created";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
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
