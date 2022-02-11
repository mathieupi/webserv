#pragma once

#include "utils.hpp"
#include "Request.hpp"

class	Route {
	public:
	std::string	pattern;
	std::string index;
	std::string	root;
	std::string	cgi;

	Route(std::string _pattern) : pattern(_pattern), index("index.html"), root("./www"), cgi("") {}
	~Route() {}

	bool match(Request &req) const
	{ return (startwith(req.url, pattern)); }
};