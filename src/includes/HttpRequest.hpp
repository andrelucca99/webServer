/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alucas-e <alucas-e@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/06 12:10:25 by jtertuli          #+#    #+#             */
/*   Updated: 2026/05/03 08:52:13 by alucas-e         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <map>

struct HttpRequest {
    std::string method;                              
    std::string uri;                                 
    std::string path;                                
    std::string query_string;                        
    std::string http_version;                        
    std::map<std::string, std::string> headers;      
    std::string body;                                
    bool isMultipart;
    std::string boundary;

    HttpRequest() : isMultipart(false) {}
};
