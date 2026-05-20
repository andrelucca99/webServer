/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alucas-e <alucas-e@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 19:11:27 by alucas-e          #+#    #+#             */
/*   Updated: 2026/04/18 11:53:34 by alucas-e         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Config.hpp"

class Server {
private:
    Config _config;

public:
    Server(const Config& config);
    ~Server();

    void run();
};
