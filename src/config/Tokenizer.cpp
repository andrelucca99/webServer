/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Tokenizer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: andre <andre@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/01 19:08:33 by andre             #+#    #+#             */
/*   Updated: 2026/04/01 19:15:47 by andre            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Tokenizer.hpp"
#include <cctype>

std::vector<std::string> Tokenizer::tokenize(const std::string& content) {
    std::vector<std::string> tokens;
    std::string current;

    for (size_t i = 0; i < content.size(); i++) {
        char c = content[i];

        if (isspace(c)) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        }
        else if (c == '{' || c == '}' || c == ';') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            tokens.push_back(std::string(1, c));
        }
        else {
            current += c;
        }
    }

    if (!current.empty())
        tokens.push_back(current);

    return tokens;
}