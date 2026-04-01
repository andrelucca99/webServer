/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 19:04:04 by andre             #+#    #+#             */
/*   Updated: 2026/04/01 19:16:38 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./includes/ConfigParser.hpp"
#include <iostream>

int main() {
    try {
        ConfigParser parser;
        Config config = parser.parse("config.conf");

        for (size_t i = 0; i < config.servers.size(); i++) {
            std::cout << "Server:\n";
            std::cout << "Port: " << config.servers[i].port << "\n";
            std::cout << "Host: " << config.servers[i].host << "\n";
            std::cout << "Root: " << config.servers[i].root << "\n";

            for (size_t j = 0; j < config.servers[i].routes.size(); j++) {
                std::cout << "  Route: " << config.servers[i].routes[j].path << "\n";
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}