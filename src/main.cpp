/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   main.cpp                                           :+:    :+:            */
/*                                                     +:+                    */
/*   By: ccaljouw <ccaljouw@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/03 11:16:40 by cariencaljo   #+#    #+#                 */
/*   Updated: 2023/12/08 10:11:07 by cariencaljo   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "webServ.hpp"
#include <signal.h>

int	g_shutdown_flag = 0;

int main(int argc, char **argv) {

	int 				epollFd;
	struct epoll_event	events[MAX_EVENTS];
	std::list<Server>	servers;

	signal(SIGINT, handleSignal);
	// signal(SIGPIPE, handleSignal);

	Config conf(argc, argv);
	if (conf.getError() == true)
		return 1;
	// conf.printServers(); // to remove later, only used for testing purposes
	try {
		if ((epollFd = epoll_create(1)) == -1)
			throw std::runtime_error("epoll_create: " + std::string(strerror(errno)));
		if ((servers = initServers(conf, epollFd)).size() == 0)
			throw std::runtime_error("no succesfull server configuration, terminating programm");
	}
	catch(const std::runtime_error& e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;
		return 1;
	}
	while (!g_shutdown_flag) {
		int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, 0);
		
		for (int i = 0; i < numEvents; i++) {
			connection *conn = static_cast<connection *>(events[i].data.ptr);
			// std::cout << "con: " << conn->fd << " event: " << events[i].events << " state: " << conn->state << std::endl;
			checkTimeout(conn);
			if (events[i].events & EPOLLIN && conn->state == LISTENING)
				newConnection(epollFd, conn->fd, conn->server);
			if (events[i].events & EPOLLIN && conn->state == READING)
				readData(conn);
			if (conn->state == HANDLING)
				handleRequest(epollFd, conn);
			if (events[i].events & EPOLLHUP && conn->state == IN_CGI)
				readCGI(epollFd, conn);
			if (events[i].events & EPOLLOUT && conn->state == WRITING)
				writeData(conn);
			if (events[i].events & EPOLLERR || conn->state == CLOSING) {
				closeCGIpipe(epollFd, conn);
				closeConnection(epollFd, conn);
			}
		}
	}
	std::cout << CYAN << "\nclean up" << RESET << std::endl;
	int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, 0);
	for (int i = 0; i < numEvents; i++) {
		connection *conn = static_cast<connection *>(events[i].data.ptr);
		if (events[i].events && conn->state != LISTENING) {
			closeCGIpipe(epollFd, conn);
			closeConnection(epollFd, conn);
		}
	}
	close (epollFd);
    return 0;
}
