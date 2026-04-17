/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 19:11:27 by andre             #+#    #+#             */
/*   Updated: 2026/04/15 19:13:27 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Socket.hpp"
#include "ServerConfig.hpp"

class Server {
public:
    Server(const ServerConfig& config);
    void run();

private:
    ServerConfig _config;
    Socket _socket;
};
