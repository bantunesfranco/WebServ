/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   CGI.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/06 12:06:07 by bfranco       #+#    #+#                 */
/*   Updated: 2023/12/02 22:25:05 by cariencaljo   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef CGI_HPP
#define CGI_HPP

#include "defines.hpp"
#include <unistd.h>

class CGI
{
	public:
		CGI() = delete;
		CGI(const CGI& other) = delete;
		CGI& operator=(const CGI& other) = delete;
		
		~CGI();
		CGI(int epollFd, connection *conn);
		int		getStatus() const;
		int		getFdIn() const;
		int		getFdOut() const;
		void	closeFds();
		
	private:
		int		_epollFd;
		int		_status;
		int		_fdIn;
		int		_fdOut;
};

int		register_CGI(int epollFd, int cgiFd, connection *conn);
#endif
