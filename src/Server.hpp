#pragma once

#include "Route.hpp"
#include "Request.hpp"
#include "mime.hpp"
#include "utils.hpp"

#define E404() { \
	server->info("E404"); \
	if (send(new_sock, "404", 3, 0) == -1) \
		server->perr("send 404"); \
	close(new_sock); \
	continue ; \
}

bool sendf(int new_sock, const std::string &path)
{
	struct stat	info;

	if (stat(path.c_str(), &info) == -1 || info.st_mode & S_IFDIR)
		return (false);
	std::string header = "HTTP/1.1 200 OK\r\nContent-length: " + atos(info.st_size) + "\r\nContent-Type: " + mime(path) + "\r\n\r\n";
	send(new_sock, header.c_str(), header.size(), 0);
	int fd = open(path.c_str(), O_RDONLY);
	#ifdef __APPLE__
		struct sf_hdtr	hdtr = { NULL, 0, NULL, 0 };
		off_t len = 0;
		sendfile(new_sock, fd, 0, &len, &hdtr, 0);
	#else
		long int off = 0;
		while (sendfile(new_sock, fd, &off, SENDFILE_BUF))
			;
	#endif
	close(fd);
	return (true);
}

class	Server {
	public:
	int							port;
	in_addr_t					host;
	std::vector<std::string>	name;
	std::map<int, std::string>	error;
	size_t						body_size;
	std::vector<Route>			routes;

	Server() : port(8080), host(INADDR_ANY), body_size(1024) {}
	~Server() {}

	/*** SETTERS ***/
	void	setPort(const std::string &_port)
	{ strisdigit(_port) ? port = atoi(_port.c_str()) : throw "invalid unumber"; }

	void	setHost(const std::string &_host)
	{ host = inet_addr(_host.c_str()); }

	void	setName(const std::string &_name)
	{ name.push_back(_name); }

	void	setError(const std::string &_code, const std::string &_url)
	{ strisdigit(_code) ? error[atoi(_code.c_str())] = _url : throw "invalid unumber"; }

	void	setBodySize(const std::string &_body_size)
	{ strisdigit(_body_size) ? body_size = atoi(_body_size.c_str()) : throw "invalid unumber"; }

	/*** DEBUG ***/
	void	info(const std::string &msg) const
	{ println(1, GRE + msg + " " ORA + atos(port)); }

	void	perr(const std::string &msg) const
	{ println(2, RED "error: " ORA + atos(port) + RED " " + msg + ": " + std::strerror(errno)); }

	void	debug()
	{
		std::cout << ENDL;
		std::cout << "port " << port << ENDL;
		std::cout << "host " << (host & 255) << "." << (host >> 8 & 255) << "." << (host >> 16 & 255) << "." << (host >> 24) << ENDL;
		for (size_t i = 0; i < name.size(); i++)
			std::cout << "name " << name[i] << ENDL;
		for (std::map<int, std::string>::iterator it = error.begin(); it != error.end(); it++)
			std::cout << "error " << it->first << " " << it->second << ENDL;
		std::cout << "body_size " << body_size << ENDL;
		for (size_t i = 0; i < routes.size(); i++)
			routes[i].debug();
	}

	/*** START ***/
	static void	*start(const Server *server)
	{
		server->info("starting ...");

		int					sock, new_sock;
		socklen_t			addrlen;
		struct sockaddr_in	addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = server->host;
		addr.sin_port = htons(server->port);

		/*** SETUP ***/
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0
			|| bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0
			|| listen(sock, 10) < 0)
		{
			server->perr("cannot start");
			return (NULL);
		}

		server->info("started");

		while (1) {
			try {
				/*** ACCEPT ***/
				if ((new_sock = accept(sock, (struct sockaddr *) &addr, &addrlen)) < 0)
				{
					server->perr(ENDL "cannot accept client");
					continue ;
				}

				/*** PARSE ***/
				Request request(new_sock, server->body_size);
				server->info(ENDL RED + request.type + GRE " " + request.url + BLU " " + request.protocol);

				/*** FIND ROUTE ***/
				const Route *route = NULL;
				for (size_t i = 0; i < server->routes.size(); i++)
				{
					if (server->routes[i].match(request))
					{
						route = &server->routes[i];
						server->info("match: " + server->routes[i].pattern);
						break ;
					}
				}

				/*** SEND ***/
				if (route)
				{
					std::string path = route->root + "/" + request.url.substr(route->pattern.size());
					if (endwith(path, "/"))
						path += route->index;

					server->info("sending file: " + path);
					if (!sendf(new_sock, path)) E404();
				}
				else
					E404();

				/*** CLOSE ***/
				close(new_sock);
			}
			catch (const std::exception &e)
			{
				server->perr(e.what());
			}
		}
		return (NULL);
	}
};