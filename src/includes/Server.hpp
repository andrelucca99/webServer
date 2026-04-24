/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 19:11:27 by andre             #+#    #+#             */
/*   Updated: 2026/04/18 11:53:34 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ServerConfig.hpp"

class Server {
private:
    ServerConfig config;

public:
    Server();
    Server(const ServerConfig& config);
    ~Server();

    void run();
};
