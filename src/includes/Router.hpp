/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alucas-e <alucas-e@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:00:09 by alucas-e          #+#    #+#             */
/*   Updated: 2026/04/18 11:22:42 by alucas-e         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Config.hpp"

class Router {
  public:
    static HttpResponse handleRequest(const HttpRequest& request, const ServerConfig& config);
};
