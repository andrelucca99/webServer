/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/17 17:01:45 by andre             #+#    #+#             */
/*   Updated: 2026/04/17 17:02:43 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Router.hpp"

HttpResponse Router::handleRequest(const HttpRequest& request) {
  HttpResponse res;

  if (request.path == "/") {
    res.status = 200;
    res.body = "Bem-vindo ao Webserv!";
    res.contentType = "text/plain";
  }
  else if (request.path == "/about") {
    res.status = 200;
    res.body = "Pagina About";
    res.contentType = "text/plain";
  }
  else {
    res.status = 404;
    res.body = "404 Not Found";
    res.contentType = "text/plain";
  }

  return res;
}
