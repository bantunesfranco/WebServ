/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Config.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/09 14:02:40 by bfranco       #+#    #+#                 */
/*   Updated: 2023/12/09 21:33:16 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "defines.hpp"
#include <unistd.h>
#include <algorithm>
#include <fstream>

enum configBlock
{
	NONE,
	SERVER,
	LOCATION,
	ERROR_PAGE
};

class Config
{
	public:
		Config(int argc, char **argv);
		~Config();
		Config( const Config& src );
		Config&	operator=( Config const & rhs );

	// ============= exception ================
		class NoSuchFileException : public std::exception
		{
			virtual const char *	what() const throw() {
				return ("Invalid path or wrong permissions");
			}
		};

		class InvalidParameterException : public std::exception
		{
			std::string	_msg;
			public:
				InvalidParameterException(int lineNr)
				{
					_msg = std::string("Line ") + std::to_string(lineNr) + std::string(": Invalid parameter");
				}
				virtual const char *	what() const throw() {
					return (_msg.c_str());
				}
		};

		class MissingParameterException : public std::exception
		{
			std::string	_msg = "Missing mandatory parameter";
			public:
				MissingParameterException(std::string parameter)
				{
					_msg += (std::string(": ") + parameter);
				}
				virtual const char *	what() const throw() {
					return (_msg.c_str());
				}
		};

	// ============= getters ================
		std::list<struct ServerSettings*>	getServers() const;
		std::map<int, std::string>*			getErrorPages() const;
		bool								getError() const;

	// ============= testing ================
		void								printServers() const;
	
	private:
		std::string							_filename;
		std::list<struct ServerSettings*>	_servers;
		bool								_error;
		unsigned int						_lineNr;

	// ============= methods =================
		void								_readConfigFile();
		int									_handleBlockEnd(configBlock *currentBlock, void *currentBlockPtr);
		int									_handleBlockContent(std::string line, configBlock currentBlock, void *currentBlockPtr);
		void								_checkMandatoryParameters(const struct ServerSettings *server);
};

// ============= utils =================
int											parseServer(std::string line, struct ServerSettings *server);
int											parseLocation(std::string line, struct LocationSettings *location);
int											parseErrorPage(std::string line, std::map<int, std::string> *errorPages);
void										deleteBlock(const configBlock& currentBlock, void *currentBlockPtr);
void										*initServerBlock();
void										*initLocationBlock(std::string line);

#endif
