#pragma once

#include "Server.hpp"
#include "utils.hpp"

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
			if (ln == "server")
			{
				Server server;
				while (scope(filename, &idx, f, ln))
				{
					std::vector<std::string> sargv = split(ln);
					if (sargv.size() != 2)
						err(filename, idx, "invalid args count");
					if (sargv[0] == "match")
					{
						Route route(sargv[1]);
						while (scope(filename, &idx, f, ln))
						{
							std::vector<std::string> margv = split(ln);
							if (margv.size() != 2)
								err(filename, idx, "invalid args count");
							else if (margv[0] == "index")
								route.index = margv[1];
							else if (margv[0] == "root")
								route.root = margv[1];
							else if (margv[0] == "cgi")
								route.cgi = margv[1];
							else
								err(filename, idx, "invalid property");
						}
						server.routes.push_back(route);
					}
					else if (sargv[0] == "port")
						server.port = atoi(sargv[1].c_str());
					else if (sargv[0] == "host")
						server.host = inet_addr(sargv[1].c_str());
					else if (sargv[0] == "name")
						server.name = sargv[1];
					else
						err(filename, idx, "invalid property");
				}
				servers.push_back(server);
			}
			else
				err(filename, idx, "not allowed kwd");
		}
		f.close();
	}
	~Config() {}
};