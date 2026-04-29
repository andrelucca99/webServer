/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 19:07:21 by andre             #+#    #+#             */
/*   Updated: 2026/04/01 19:07:25 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <vector>
#include <map>
#include "RouteConfig.hpp"

struct ServerConfig {
    int                         port;
    std::string                 host;
    std::string                 root;
    size_t                      client_max_body_size;
    std::map<int, std::string>  error_pages;
    std::vector<RouteConfig>    routes;

    ServerConfig() : port(0), client_max_body_size(1024 * 1024) {}
};