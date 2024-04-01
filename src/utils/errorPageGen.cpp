/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   errorPageGen.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: ccaljouw <ccaljouw@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/24 08:20:32 by carlo         #+#    #+#                 */
/*   Updated: 2023/12/08 21:30:38 by cariencaljo   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "webServ.hpp"
#include <sys/stat.h>


// Success 2xx /  Error 4xx, 5xx / Redirection 3xx
std::map<int, std::string> errorPages = {
 	{ 400, "Bad request"},					// The request had bad syntax or was inherently impossible to be satisfied. 
 	// { 401, "Unauthorized"},				// The parameter to this message gives a specification of authorization schemes which are acceptable. The client should retry the request with a suitable Authorization header. 
 	{ 403, "Forbidden"},					// The request is for something forbidden. Authorization will not help. 
	{ 404, "Not found"},					//  The server has not found anything matching the URI given 
	{ 405, "Method Not Allowed"},
	{ 408, "Request Timeout"},
	{ 413, "Content Too Large"},
	{ 422, "Unprocessable Entity"}, 		// Indicates that the server understands the content type of the request entity, and the syntax of the request entity is correct, but it was unable to process the contained instructions. 
	{ 429, "Too Many Requests"},
	{ 500, "Internal Error"},				// The server encountered an unexpected condition which prevented it from fulfilling the request. 
	{ 501, "Not implemented"},				// The server does not support the facility required. 
	// { 502, "Service temporary overloaded"},
	{ 504, "Gateway timeout"},
	{ 505, "version not supported"}
};


//checks is a directory exists
bool	directoryExists(const std::string& path) {
	struct stat info;
	return stat(path.c_str(), &info) == 0 && (info.st_mode & S_IFDIR) != 0;
}


std::string	generateErrorPage(int e) {
	// std::cout << "\nin error page generator\n" << std::endl;

	std::string error = std::to_string(e);
	std::string title = "Undefined Error";
	for (const auto& pair : errorPages)
		if (pair.first == e) {
			title = pair.second;
			// std::cout << "found matching eror page" << " " <<  e << " " << title << std::endl;	
		}

//set page content
std::string htmlContent = R"(
<!DOCTYPE html>
<html>
<head>
	<link rel="stylesheet" href="/90s_styles.css">
	<title>)" + error + " " + title + R"(</title>
</head>
<body>
	<nav class="left-menu">
			<ul>
			<li><a href="/index.html"><img class="small" src="/cookie.png"></a></li>
			<li><a href="/upload.html">Upload</a></li>
			<li><a href="/delete.html">Delete</a></li>
			<li><a href="/other.html">Cookies</a></li>
			<li><a href="/sockets.html">Sockets</a></li>
			<li><a href="/epoll.html">Epoll</a></li>
			</ul>
	</nav>
	<div class="container">
		<h1>)" + error + " " + title + R"(</h1>
	</div>
</body>
</html>
)";

	//create tmp dir
	std::string pathToTmp = "tmp";
	if (!directoryExists(pathToTmp)) {
		if (mkdir(pathToTmp.c_str(), 0777) == -1) 
			std::cerr << RED << "Error making dir" << RESET << std::endl;
	}

	//write to html page
	std::string path = pathToTmp + "/" + error + ".html";
	std::ofstream errorPageStream(path);
	if (errorPageStream.is_open()) {
		errorPageStream << htmlContent;
		errorPageStream.close();
	}
	else {
		std::cerr << RED <<  "unable to generate error HTML file" << RESET <<  std::endl;
		return "";
	}
	return path;
}

std::string	getErrorPage(connection *conn, int error) {
	std::string		errorHtmlPath;
	Uri				uri;
	std::string		host = uri.getHost();
	std::map<int, std::string> *providedErrorPages = conn->server->get_errorPages(host);
	if (providedErrorPages->size() != 0) {
		for (const auto& pair : *providedErrorPages) {
			if (pair.first == error) {
				// std::cout << RED << "directed to error page set in config with nr: " << error <<  RESET << std::endl;
				errorHtmlPath = "data/text/html" + pair.second; // can this be handled better?
				break;
			}
			else
				errorHtmlPath = generateErrorPage(error);
		}
	}
	else
		errorHtmlPath = generateErrorPage(error);
	return errorHtmlPath;
}


