/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tokenizer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alucas-e <alucas-e@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 19:08:15 by alucas-e          #+#    #+#             */
/*   Updated: 2026/04/01 19:08:20 by alucas-e         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include <vector>
#include <string>

class Tokenizer {
    public:
        std::vector<std::string> tokenize(const std::string& content);
};