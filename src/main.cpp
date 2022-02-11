/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bledda <bledda@student.42nice.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/02/10 11:22:10 by bledda            #+#    #+#             */
/*   Updated: 2022/02/11 02:42:33 by bledda           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/config.hpp"

int main(int ac, char **av)
{
	std::vector<t_config> config;

	if (ac >= 2)
	{
		webserv::log("Additional arguments will be ignored");
		config = config_file(av[1]);
	}
	else
		webserv::log("Using default config");

	return (0);
}
