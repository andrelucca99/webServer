/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   File.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alucas-e <alucas-e@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 11:20:12 by alucas-e          #+#    #+#             */
/*   Updated: 2026/04/27 05:57:55 by alucas-e         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fstream>
#include <sstream>

std::string readFile(const std::string& path) {
    std::ifstream file(path.c_str());
    if (!file.is_open())
        return "";

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path.c_str(), std::ios::out | std::ios::binary);
    if (!file.is_open())
        return false;

    file << content;
    return true;
}
