/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteConfig.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 19:06:25 by andre             #+#    #+#             */
/*   Updated: 2026/04/01 19:06:36 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include <vector>

struct RouteConfig {
    std::string path;
    std::vector<std::string> methods;
    std::string index;
    bool autoindex;

    RouteConfig() : autoindex(false) {}
};