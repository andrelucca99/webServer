/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 19:04:04 by andre             #+#    #+#             */
/*   Updated: 2026/04/25 09:42:16 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./includes/Server.hpp"
#include "./includes/ConfigParser.hpp"
#include <iostream>

int main() {
    ConfigParser parser;
    Config config;

    try {
        config = parser.parse("config.conf");
    } catch (const std::exception& e) {
        std::cerr << "Config error: " << e.what() << std::endl;
        return 1;
    }

    if (config.servers.empty()) {
        std::cerr << "No server block found in config.conf" << std::endl;
        return 1;
    }

    Server server(config);
    server.run();
    return 0;
}
