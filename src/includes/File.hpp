/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   File.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alucas-e <alucas-e@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 11:21:06 by alucas-e          #+#    #+#             */
/*   Updated: 2026/04/27 05:57:20 by alucas-e         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <string>

std::string readFile(const std::string& path);

bool writeFile(const std::string& path, const std::string& content);
