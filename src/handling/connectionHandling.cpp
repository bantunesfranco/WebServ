/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   connectionHandling.cpp                             :+:    :+:            */
/*                                                     +:+                    */
/*   By: ccaljouw <ccaljouw@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/03 23:45:10 by cariencaljo   #+#    #+#                 */
/*   Updated: 2023/12/08 23:42:15 by cariencaljo   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "webServ.hpp"
#include <arpa/inet.h>
#include <fstream>

void	newConnection(int epollFd, int serverFd, Server *server) 
{
	int	fd;
	
	// std::cout << "new connection request" << std::endl; // for testing
	try {
		if ((fd = accept(serverFd, nullptr, nullptr)) == -1)
			throw std::runtime_error("client accept: " + std::string(strerror(errno)));
		if (register_client(epollFd,  fd, server))
			throw std::runtime_error("register client: " + std::string(strerror(errno)));
		// std::cout << "connection setup on fd: " << fd << std::endl;
	}
	catch (std::runtime_error& e) {
		std::cerr << RED << "Error\n" << e.what() << RESET << std::endl;
	}
}

void readData(connection *conn) 
{
    char 	buffer[BUFFER_SIZE];
    ssize_t bytesRead;

	// std::cout << "readData" << "\tfd = " << conn->fd << std::endl; //for testing
	if (conn->nr_of_requests == conn->server->get_maxNrOfRequests()) {
		std::cout << RED << "Too many requests on open socket, closing connection" << RESET << std::endl;
		setErrorResponse(conn, 429);
		return;
	}
	if ((bytesRead = recv(conn->fd, buffer, sizeof(buffer), MSG_DONTWAIT)) > 0) {
		// std::cout << YELLOW << buffer << RESET << std::endl; //for testing
		std::time(&conn->time_last_request);
		conn->request.append(buffer, static_cast<long unsigned int>(bytesRead));
	}
	switch(bytesRead)
	{
		case -1:
			setErrorResponse(conn, 500);
			break;
		case 0:
			break;
		case BUFFER_SIZE:
			break;
		default:
			conn->nr_of_requests += 1;
			conn->state = HANDLING;
	}
}

void readCGI(int epollFd, connection *conn)
{
	char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
	
	// std::cout << "readCGI" << "\tcgiFd = " << conn->cgiFd << std::endl; //for testing
    if ((bytesRead = read(conn->cgiFd, buffer, BUFFER_SIZE)) > 0) {
		conn->response.append(buffer, static_cast<long unsigned int>(bytesRead));
    }
	switch(bytesRead)
	{
		case -1:
			std::cerr << RED << "Error\nproblem reading CGI" << RESET << std::endl;
			closeCGIpipe(epollFd, conn);
			setErrorResponse(conn, 500);
			break;
		case 0:
			if (conn->response.empty()) {
				std::cerr << RED << "Error\nnothing to read from CGI" << RESET << std::endl;
				closeCGIpipe(epollFd, conn);
				setErrorResponse(conn, 500);		
			}
			break;
		case BUFFER_SIZE:
			break;
		default:
			closeCGIpipe(epollFd, conn);
			conn->state = WRITING;
	}
	conn->request.clear();
}

void writeData(connection *conn) 
{
	int			len;
	
	// std::cout << "writeData" << "\tfd = " << conn->fd << std::endl; //for testing
	len = std::min(static_cast<int>(conn->response.length()), BUFFER_SIZE);
    len = send(conn->fd, conn->response.c_str(), len, MSG_NOSIGNAL);
	if (len == -1) {
		std::cerr << RED << "send error: Connection closed by peer" << RESET << std::endl;
		conn->state = CLOSING;
	}
	else if (len < static_cast<int>(conn->response.size())) {
		conn->response = conn->response.substr(len, conn->response.npos);
		conn->state = WRITING;
	}
	else {
		conn->response.clear();
		if (conn->close_after_response)
			conn->state = CLOSING;
		else
			conn->state = READING;
		// std::cout << "Response sent, nr of requests: " << conn->nr_of_requests << " state now: " << conn->state << std::endl; //for testing
	}
}

void	closeCGIpipe(int epollFd, connection *conn) {
	if (conn->cgiFd) {
		epoll_ctl(epollFd, EPOLL_CTL_DEL, conn->cgiFd, nullptr);
		if (close (conn->cgiFd))
			std::cout << RED << "failed to close CGIfd " << conn->fd << ": " << std::string(strerror(errno)) << RESET << std::endl;
		// else
		// 	std::cout << CYAN << "Connection on CGIfd " << conn->cgiFd << " closed" << RESET << std::endl;
		conn->cgiFd = 0;
	}
}

void	closeConnection(int epollFd, connection *conn) {
	epoll_ctl(epollFd, EPOLL_CTL_DEL, conn->fd, nullptr);
	if (close(conn->fd))
		std::cout << RED << "failed to close fd " << conn->fd << ": " << std::string(strerror(errno)) << RESET << std::endl;
	// else
	// 	std::cout << CYAN << "Connection on fd " << conn->fd << " closed" << RESET << std::endl;
	delete conn;
}

