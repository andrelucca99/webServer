/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpError.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 10:18:47 by jtertuli          #+#    #+#             */
/*   Updated: 2026/05/14 17:02:55 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/HttpError.hpp"
#include "../includes/File.hpp"
#include "../includes/HttpResponse.hpp"
#include <sstream>

std::string httpErrorBody(int status, const ServerConfig& config) {
    std::map<int, std::string>::const_iterator it = config.error_pages.find(status);
    if (it != config.error_pages.end()) {
        std::string ep   = it->second;
        std::string path = config.root;
        if (!path.empty() && path[path.size() - 1] != '/') path += "/";
        if (!ep.empty() && ep[0] == '/') ep.erase(0, 1);
        std::string content = readFile(path + ep);
        if (!content.empty()) return content;
    }
    std::ostringstream oss;
    oss << "<h1>" << status << " " << HttpResponse::reasonPhraseFor(status) << "</h1>";
    return oss.str();
}
