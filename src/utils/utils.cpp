/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   utils.cpp                                          :+:    :+:            */
/*                                                     +:+                    */
/*   By: ccaljouw <ccaljouw@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/22 21:57:55 by carlo         #+#    #+#                 */
/*   Updated: 2023/12/11 17:25:05 by cwesseli      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "webServ.hpp"
#include <signal.h>


std::string getTimeStamp() {
	//get current time
    std::time_t now = std::time(nullptr);
	
	//store current time in struct as gmt time
	std::tm* timeInfoGmt = std::gmtime(&now);
    
	// Define the format for the output string
    char buffer[40];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeInfoGmt);
    return buffer;
}

void	setErrorResponse(connection *conn, int error)
{
	HttpRequest		request(conn->server);
	HttpResponse	response(request);

	// check for error pages set in config
	std::string errorHtmlPath = getErrorPage(conn, error);
	std::ifstream f(errorHtmlPath);
	if (f.good())
		response.reSetBody(errorHtmlPath, false);
	f.close();
	response.setStatusCode(error);
	response.setResponse(conn);
}


// Some browsers (eg Firefox), check form timeout based on header keep-alive with timeout.
// others (eg safari) do not close the connection themselves
int	checkTimeout(connection *conn)
{
	double timeout;

	if (conn->state == IN_CGI || conn->state == WRITING || conn->state == HANDLING)
		timeout = CGI_TIMEOUT;
	else
		timeout = conn->server->get_timeout();
	if (std::difftime(std::time(nullptr), conn->time_last_request) >= timeout) {
		switch(conn->state)
		{
			case READING:
				// std::cerr << CYAN << "request timeout" << RESET << std::endl;
				setErrorResponse(conn, 408);
				return 1;
			case IN_CGI: 
			//	std::cerr << CYAN << "timeout in cgi" << RESET << std::endl;
				if (conn->cgiPID) {
					close(conn->cgiFd);
					conn->cgiFd = 0;
					kill(conn->cgiPID, SIGTERM);
				}
				setErrorResponse(conn, 504);
				return 1;
			case HANDLING:
				//std::cerr << CYAN << "timout in processing" << RESET << std::endl;
				setErrorResponse(conn, 500);
				return 1;
			case WRITING:
				// std::cerr << CYAN << "stuck in writing" << RESET << std::endl;
				return 1;
			default:
				return 0;
		}
	}
	return 0;
}

std::string	removeWhitespaces(std::string str) {
	size_t	index;
	index = str.find_first_not_of(WHITE_SPACE);
	if (index != std::string::npos)
	 	str	= str.substr(index);
	index = str.find_last_not_of(WHITE_SPACE);
	if (index != std::string::npos)
	 	str	= str.substr(0, index + 1);
	return str;
}

std::string replaceCookiePng(std::string location, std::string cookieValue) {
	if (location == "/cookie.png") {
		std::string trigger = cookieValue.substr(cookieValue.rfind("=") + 1) ;
		location = "/" + trigger + ".png";	
	}
	return location;
}

void	handleSignal(int signal) {
	// if (signal == SIGPIPE)
	// 	std::cout << CYAN << "Ignoring SIGPIPE signal" << std::endl;
	// else
		g_shutdown_flag = signal;
}

//checks is a path exists
bool	validPath(std::string path) {

	if (!path.empty() && path[0] == '/')
        	path.erase(0, 1);
    struct stat info;
    return (stat(path.c_str(), &info) == 0);
}
