/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alucas-e <alucas-e@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 19:09:03 by alucas-e          #+#    #+#             */
/*   Updated: 2026/04/01 19:16:17 by alucas-e         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <vector>
#include <string>
#include "Config.hpp"

class ConfigParser {
    private:
        std::vector<std::string> tokens;
        size_t pos;

        std::string current();
        void advance();
        void expect(const std::string& token);

        ServerConfig parseServer();
        RouteConfig parseRoute();

    public:
        Config parse(const std::string& filename);
};