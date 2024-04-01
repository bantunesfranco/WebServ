/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   registerEvents.cpp                                 :+:    :+:            */
/*                                                     +:+                    */
/*   By: ccaljouw <ccaljouw@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/03 23:48:35 by cariencaljo   #+#    #+#                 */
/*   Updated: 2023/12/08 09:42:54 by cariencaljo   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "webServ.hpp"

int	register_server(int epollFd, int fd, Server *server)
{
	struct epoll_event 	event;
	connection			*conn;
	int					tries = 0;
	
	conn = new connection;
	conn->fd = fd;
	conn->cgiFd = 0;
	conn->cgiPID = 0;
	conn->state = LISTENING;
	conn->request = "";
	conn->response = "";
	conn->server = server;
	conn->time_last_request = std::time(nullptr);
	conn->nr_of_requests = 0;
    event.events = EPOLLIN;
    event.data.ptr = conn;
	conn->close_after_response = 0;
	server->set_connection(conn);
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1 && tries < 3)
		return 1;
	return 0;
}

int	register_client(int epollFd, int fd, Server *server)
{
	struct epoll_event 	event;
	connection			*conn;

	conn = new connection;
	conn->fd = fd;
	conn->cgiFd = 0;
	conn->cgiPID = 0;
	conn->state = READING;
	conn->request = "";
	conn->response = "";
	conn->server = server;
	conn->time_last_request = std::time(nullptr);
	event.events = EPOLLIN | EPOLLOUT;
    event.data.ptr = conn;
	conn->nr_of_requests = 0;
	conn->close_after_response = 0;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
	{
		std::cerr << RED << "Error/nNot able to register new connection to epoll list" << RESET <<std::endl;
		setErrorResponse(conn, 500);
		while (conn->state == WRITING)
			writeData(conn);
		close(conn->fd);
		delete conn;
		return 1;
	}
	return 0;
}

int	register_CGI(int epollFd, int cgiFd, connection *conn)
{
	struct epoll_event 	event;
	
	conn->cgiFd = cgiFd;
	event.events = EPOLLHUP;
    event.data.ptr = conn;
	if (epoll_ctl(epollFd, EPOLL_CTL_ADD, cgiFd, &event) == -1)
	{
		std::cerr << RED << "Error/nNot able to register CGI to epoll list" << RESET <<std::endl;
		setErrorResponse(conn, 500);
		close (conn->cgiFd);
		conn->cgiFd = 0;
		return 1;
	}
	// std::cout << GREEN << "cgi registered for " << conn->cgiFd << " state: " << conn->state << RESET <<std::endl;  //testing
	return 0;
}
