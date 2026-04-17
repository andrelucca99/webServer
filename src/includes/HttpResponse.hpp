/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 12:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/04/17 16:58:13 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <map>

class HttpResponse {
	private:
		std::string							_http_version;
		int									_status_code;
		std::string							_reason_phrase;
		std::map<std::string, std::string>	_headers;
		std::string							_body;

	public:
		HttpResponse();
		HttpResponse(const HttpResponse& other);
		HttpResponse& operator=(const HttpResponse& other);
		~HttpResponse();

		static std::string reasonPhraseFor(int code);
		static std::string mimeTypeFor(const std::string& extension);

		int status;
    std::string body;
    std::string contentType;

		std::string build() const;
};
