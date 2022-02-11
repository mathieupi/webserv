#pragma once

#include "Route.hpp"
#include "Request.hpp"
#include "utils.hpp"

#define E404() { \
	server->info("E404"); \
	if (send(new_sock, "404", 3, 0) == -1) \
		server->perr("send 404"); \
	close(new_sock); \
	continue ; \
}

void sendf(int new_sock, int fd)
{
	long int off = 0;

	#ifdef __APPLE__
		struct sf_hdtr	hdtr = { NULL, 0, NULL, 0 };
		while (sendfile(new_sock, fd, &off, SENDFILE_BUF, &hdtr, 0))
			;
	#else
		while (sendfile(new_sock, fd, &off, SENDFILE_BUF))
			;
	#endif
	close(fd);
}

class	Server {
	public:
	std::vector<Route>	routes;
	int					port;
	in_addr_t			host;
	std::string			name;

	Server() : port(8080), host(INADDR_ANY), name("") {}
	~Server() {}


	void info(const std::string &msg) const
	{ println(1, std::stringstream() << GRE << msg << " " RED << name << EOC ":" ORA << port); }

	void perr(const std::string &msg) const
	{ println(2, std::stringstream() << RED "error: " << name << ":" ORA << port << RED " " << msg << ": " << std::strerror(errno)); }

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
				Request request(new_sock);
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
					int	fd = open(path.c_str(), O_RDONLY);
					if (fd == -1) E404();
					sendf(new_sock, fd);
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