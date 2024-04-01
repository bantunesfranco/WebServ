/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGI.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: ccaljouw <ccaljouw@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/06 12:51:38 by bfranco       #+#    #+#                 */
/*   Updated: 2023/12/02 22:24:57 by cariencaljo   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI(int epollFd, connection *conn) : _epollFd(epollFd), _status(0), _fdIn(-1), _fdOut(-1)
{
	int	fd[2];
	try
	{
		if (pipe(fd) == -1)
			throw std::runtime_error("CGI pipe: " + std::string(strerror(errno)));
		else
		{
			_fdIn = fd[0];
			_fdOut = fd[1];
			if (register_CGI(_epollFd, _fdIn, conn))
				throw std::runtime_error("registerCGI: " + std::string(strerror(errno)));
		}
	}
	catch(const std::runtime_error& e)
	{
		_status = 1;
		std::cerr << "\033[31;1mError\n" << e.what() << "\n\033[0m";
	}
}

void	CGI::closeFds()
{
	if (_fdIn != -1)
	{
		close(_fdIn);
		_fdIn = -1;
	}
	if (_fdOut != -1)
	{
		close(_fdOut);
		_fdOut = -1;
	}
}

CGI::~CGI() {};
int		CGI::getStatus() const		{ return (_status); }
int		CGI::getFdIn() const		{ return (_fdIn); }
int		CGI::getFdOut() const		{ return (_fdOut); }

