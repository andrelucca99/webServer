/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 19:07:24 by andre             #+#    #+#             */
/*   Updated: 2026/04/15 19:09:03 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

class Socket {
  private:
    int _server_fd;
    int _port;
    std::string _host;

  public:
    Socket(int port, const std::string& host);
    ~Socket();

    void init();
    void startListening();
    int acceptConnection();
};
