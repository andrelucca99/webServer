/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpError.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jtertuli <jtertuli@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 00:00:00 by jtertuli          #+#    #+#             */
/*   Updated: 2026/05/12 00:00:00 by jtertuli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>
#include "ServerConfig.hpp"

// Retorna o body HTML para um status de erro. Se houver entry em
// config.error_pages, tenta servir o arquivo. Caso o arquivo nao exista
// ou nao haja error_page configurada, devolve um fallback inline.
std::string httpErrorBody(int status, const ServerConfig& config);
