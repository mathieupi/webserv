#pragma once

#include "Server.hpp"
#include "utils.hpp"

bool need(std::vector<std::string> argv, size_t argc)
{
	if (argv.size() != argc) throw "invalid argument count";
	return (1);
}

class Config {
	public:
	std::vector<Server> servers;

	Config(const char *filename)
	{
		std::ifstream		f;
		std::string			ln;
		int					idx = 0;
		struct stat			info;

		if (!stat(filename, &info) && info.st_mode & S_IFDIR)
			perr(std::string(filename), "is a directory");
		f.open(filename);
		if (f.fail())
			perr("cannot open " + std::string(filename));
		while (getline(&idx, f, ln))
		{
			try {
				if (ln == "server")
				{
					Server server;
					while (scope(filename, &idx, f, ln))
					{
						std::vector<std::string> sargv = split(ln);
						if (sargv[0] == "match" && need(sargv, 2))
						{
							Route route(sargv[1]);
							while (scope(filename, &idx, f, ln))
							{
								std::vector<std::string> margv = split(ln);
								if (margv[0] == "autoindex" && need(margv, 2))
									route.setAutoIndex(margv[1]);
								else if (margv[0] == "index" && need(margv, 2))
									route.setIndex(margv[1]);
								else if (margv[0] == "root" && need(margv, 2))
									route.setRoot(margv[1]);
								else if (margv[0] == "cgi" && need(margv, 3))
									route.setCgi(margv[1], margv[2]);
								else if (margv[0] == "method" && need(margv, 2))
									route.setMethod(margv[1]);
								else if (margv[0] == "redirect" && need(margv, 3))
									route.setRedirect(margv[1], margv[2]);
								else
									throw "invalid property";
							}
							server.routes.push_back(route);
						}
						else if (sargv[0] == "port" && need(sargv, 2))
							server.setPort(sargv[1]);
						else if (sargv[0] == "host" && need(sargv, 2))
							server.setHost(sargv[1]);
						else if (sargv[0] == "name" && need(sargv, 2))
							server.setName(sargv[1]);
						else if (sargv[0] == "error" && need(sargv, 3))
							server.setError(sargv[1], sargv[2]);
						else if (sargv[0] == "body_size" && need(sargv, 2))
							server.setBodySize(sargv[1]);
						else
							throw "invalid property";
					}
					servers.push_back(server);
				}
				else
					err(filename, idx, "not allowed kwd");
			}
			catch (char const *e)
			{ err(filename, idx, std::string(e)); }
		}
		f.close();
	}
	~Config() {}
};