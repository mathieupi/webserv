#pragma once

#include "utils.hpp"

class Request {
	public:
	std::string	plain;

	std::string	request;
	std::string	type;
	std::string	url;
	std::string	protocol;

	Request(int sock)
	{
		char	buf[REQ_BUF + 1] = {0};
		int		ret = recv(sock, buf, REQ_BUF, 0);
		if (ret == -1) throw std::runtime_error("cannot recv");

		plain = std::string(buf);

		std::stringstream	ss(plain);
		std::getline(ss, request);
		std::vector<std::string>	req = split(request);
		if (req.size() != 3) throw std::runtime_error("invalid request");
		type = req[0];
		url = req[1];
		protocol = req[2];
	}
	~Request() {}
};