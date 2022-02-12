#pragma once

#include "Route.hpp"
#include "Request.hpp"
#include "mime.hpp"
#include "utils.hpp"

#define E(code) { \
	server->info("E" # code); \
	if (send(new_sock, # code, 3, 0) == -1) \
		server->perr("send " # code); \
	close(new_sock); \
	continue ; \
}

void sendf(int new_sock, const std::string &path, struct stat &info)
{
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
}

class	Server {
	public:
	int								port;
	in_addr_t						host;
	std::set<std::string>			name;
	std::map<int, std::string>		error;
	size_t							body_size;
	std::map<std::string, Route>	routes;

	Server() : body_size(1024) {}
	~Server() {}

	/*** SETTERS ***/
	void	setPort(const std::string &_port)
	{ strisdigit(_port) ? port = atoi(_port.c_str()) : throw "invalid unumber"; }

	void	setHost(const std::string &_host)
	{ host = inet_addr(_host == "localhost" ? "127.0.0.1" : _host.c_str()); }

	void	setName(const std::string &_name)
	{ name.insert(_name); }

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
		for (std::set<std::string>::iterator it = name.begin(); it != name.end(); it++)
			std::cout << "name " << *it << ENDL;
		for (std::map<int, std::string>::iterator it = error.begin(); it != error.end(); it++)
			std::cout << "error " << it->first << " " << it->second << ENDL;
		std::cout << "body_size " << body_size << ENDL;
		for (std::map<std::string, Route>::iterator it = routes.begin(); it != routes.end(); it++)
		{
			std::cout << "match " << it->first << ENDL;
			it->second.debug();
		}
	}

	const std::string	match(std::string url)
	{
		(void) url;
		/*std::cout << RED << url << ENDL << std::endl;
		if (routes.count(url))
		{
			std::cout << "here " << *(split(url, "/").end() - 1) << std::endl;
			if (!routes[url].root.empty() && routes[url].root != url)
				return (match(routes[url].root));
			else if (routes[url].index)
				return (routes[url].root + "/" + *(split(url, "/").end() - 1));
			else
				return (routes[url].root);
		}
		else if (url.empty())
			return ("404");
		else
		{
			std::cout << url << std::endl;
			std::cout << "-> " << url.substr(0, url.find_last_of('/')) << std::endl;
			//exit(1);
			//std::cout << "here1" << std::endl;
			return (match(url.substr(0, url.find_last_of('/'))));
		}*/
		
		return ("404");
	}

	/*** START ***/
	#define SERVER_ERROR(msg) { \
		server->perr(msg); \
		return (NULL); \
	} 

	static void	*start(Server *server)
	{
		server->info("starting ...");

		int					sock, new_sock;
		struct sockaddr_in	addr;
		socklen_t			addrlen = sizeof(addr);
		int	opt = 1;

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = server->host;
		addr.sin_port = htons(server->port);

		/*** SETUP ***/
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			SERVER_ERROR("cannot create socket");
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
			SERVER_ERROR("cannot bind");
		if (listen(sock, 10) < 0)
			SERVER_ERROR("cannot listen");

		server->info("started");

		while (1) {
			try {
				/*** ACCEPT ***/
				if ((new_sock = accept(sock, (struct sockaddr *) &addr, &addrlen)) < 0)
				{
					server->perr(ENDL "cannot accept client");
					continue ;
				}

				std::cout << "parsing request" << ENDL;

				/*** PARSE ***/
				Request request(new_sock, server->body_size);
				server->info(ENDL RED + request.type + GRE " " + request.url + BLU " " + request.protocol);

				std::cout << RED "will match" << ENDL << std::endl;

				/*** SEND ***/
				const std::string path = server->match(split(request.url, "?")[0]);

				std::cout << RED "route found: " << path << ENDL << std::flush;

				/*** CGI ***/
				//if (path == cgi)
				//	cgi();
				//else
				//	senf();

				server->info("sending file: " + path);

				struct stat	info;
				if (stat(path.c_str(), &info) == -1)
					E(404)
				else if (info.st_mode & S_IFDIR)
				{
					// if (autoindex)
					//else
					E(403);
				}
				else
					sendf(new_sock, path, info);

				/*** CLOSE ***/
				close(new_sock);
			}
			catch (const std::exception &e)
			{
				// E500
				E(500);
				server->perr(e.what());
			}
		}
		return (NULL);
	}
};