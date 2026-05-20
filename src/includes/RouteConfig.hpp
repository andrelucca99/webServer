/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteConfig.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alucas-e <alucas-e@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 19:06:25 by alucas-e          #+#    #+#             */
/*   Updated: 2026/04/01 19:06:36 by alucas-e         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <map>
#include <string>
#include <vector>

struct RouteConfig {
    std::string                        path;
    std::vector<std::string>           methods;
    std::string                        index;
    bool                               autoindex;
    int                                redirect_code;
    std::string                        redirect_url;
    std::string                        upload_store;
    std::string                        root;
    std::map<std::string, std::string> cgi_extensions;

    RouteConfig() : autoindex(false), redirect_code(0) {}
};