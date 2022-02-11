#include "Config.hpp"

int	main(int argc, char **argv)
{
	/*** PARSING ***/
	Config conf(argc >= 2 ? argv[1] : DEFAULT_CONF);

	/*** DEBUG ***/
	for (size_t j = 0; j < conf.servers.size(); j++)
	{
		const Server &s = conf.servers[j];
		std::cout << std::endl << BLU "on" EOC "(" ORA << s.port << EOC ", " GRE << s.host << EOC ", " GRE << s.name << EOC ")" << std::endl;
		for (size_t i = 0; i < s.routes.size(); i++)
		{
			const Route &r = s.routes[i];
			std::cout << "   - " BLU "route" EOC "(" GRE << r.pattern << EOC ") . { " RED "index" EOC ": " GRE << r.index << EOC ", " RED "root" EOC ": " GRE << r.root << EOC ", " RED "cgi" EOC ": " GRE << (r.cgi == "" ? ORA "None" : r.cgi) << EOC " }" << std::endl;
		}
	}
	std::cout << std::endl;

	/*** LAUNCH ***/
	pthread_t *threads = new pthread_t[conf.servers.size()];
	for (size_t i = 0; i < conf.servers.size(); i++)
		pthread_create(threads + i, NULL, (void* (*)(void *))Server::start, (void *)&conf.servers[i]);

	/*** JOIN ***/
	for (size_t i = 0; i < conf.servers.size(); i++)
		pthread_join(threads[i], NULL);

	/*** FREE ***/
	delete [] threads;
}