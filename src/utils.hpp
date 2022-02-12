#pragma once

#include <iostream>
#include <string>
#include <cstdio>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <fcntl.h>

#ifdef __APPLE__
# include <sys/socket.h>
#else
# include <sys/sendfile.h>
#endif

#define DEFAULT_CONF "default.conf"
#define REQ_BUF 2048
#define SENDFILE_BUF 2048

#define EOC "\033[0m"    //reset
#define ENDL EOC "\n"    //reset + endl
#define RED "\033[1;91m" //red
#define ORA "\033[1;33m" //orange
#define BLU "\033[1;94m" //blue
#define GRE "\033[0;92m" //green
#define GRA "\033[0;90m" //grey

std::string	&trim(std::string &s)
{ return (s.erase(0, s.find_first_not_of(" \t")).erase(s.find_last_not_of(" \t") + 1)); }

bool	startwith(std::string s, std::string start)
{ return (s.substr(0, start.size()) == start); }
bool	endwith(std::string s, std::string end)
{
	if (end.size() > s.size()) return (false);
	return (s.substr(s.size() - end.size()) == end);
}

std::vector<std::string>	split(std::string &s)
{
	std::vector<std::string> split;
	std::stringstream ss(s);
	std::string word;
	while (ss >> word)
		split.push_back(word);
	return (split);
}

template <typename T>
std::string atos(const T &t)
{
	std::ostringstream ss;
	ss << t;
	return (ss.str());
}

void	println(int fd, const std::string &s)
{
	std::string o = s + ENDL;
	write(fd, o.c_str(), o.size());
}

/* Function to write format error in the config file */
void	err(const char *filename, const int idx, const std::string &msg)
{
	println(2, std::string(RED "error: ") + filename + ":" + atos(idx) + ": " + msg);
	exit(1);
}
/* Function to write system error such as open ... */
void	perr(const std::string &msg, const std::string &reason = std::strerror(errno))
{
	println(2, std::string(RED "error: ") + msg + ": " + reason);
	exit(1);
}

bool	getline(int *idx, std::ifstream &f, std::string &ln)
{
	while (++(*idx))
		if (!std::getline(f, ln))
			return (false);
		else if (trim(ln) != "" && !startwith(ln, "//"))
			break ;
	return (true);
}

bool	scope(const char *filename, int *idx, std::ifstream &f, std::string &ln)
{
	if (!getline(idx, f, ln)) err(filename, *idx, "unexpected eof");
	return (ln != ";");
}