/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Config.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/09 15:17:36 by bfranco       #+#    #+#                 */
/*   Updated: 2023/12/08 10:03:29 by cariencaljo   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"

Config::Config(int argc, char** argv) : _error(false), _lineNr(1)
{
	if (argc == 1) {
		_filename = std::string("./config/default.conf");
	}
	else if (argc >= 2) {
		_filename = std::string(argv[1]);
		if (argc > 2) {
			std::cerr << CYAN << "\nToo many arguments, Ignoring all but first" << RESET << std::endl;
		}
	}
	std::cout << CYAN << "Config: " << _filename << " loading...." << RESET << std::endl;
	try
	{
		_servers = std::list<struct ServerSettings*>();
		_readConfigFile();
		for (const auto server : _servers)
			_checkMandatoryParameters(server);
	}
	catch (const std::exception& e)
	{
		std::cerr << RED << e.what() << RESET << std::endl;
		_error = true;
	}

}

Config::~Config()
{
	for (auto& server : _servers) {
		for (auto& location : server->_locations) {
			delete location;
		}
		if (!server->_locations.empty())
			server->_locations.clear();
		if (server->_errorPages != nullptr) {
			server->_errorPages->clear();
			delete server->_errorPages;
		}
		delete server;
	}
	_servers.clear();
}

std::list<struct ServerSettings*>	Config::getServers() const	{	return (_servers);	}
bool								Config::getError() const	{	return (_error);	}

void	Config::_readConfigFile()
{
	std::ifstream	configFile(_filename.c_str());
	std::string		line;
	configBlock		currentBlock = NONE;
	void			*currentBlockPtr = nullptr;

	// check if file is valid and open
	if (!configFile.is_open())
	{
		std::cerr << RED << "Config file: " << RESET;
		std::flush(std::cerr);
		throw NoSuchFileException();
	}

	// read file line by line
	while (std::getline(configFile, line))
	{
		line.erase(0, line.find_first_not_of(" \t\n\v\f\r"));
		line.erase(line.find_last_not_of("; \t\n\v\f\r") + 1);
		if (line.empty() || line[0] == '#')
		{
			_lineNr++;
			continue;
		}

		if (currentBlock == NONE && line.find("server {") != std::string::npos)
		{
			currentBlock = SERVER;
			currentBlockPtr = initServerBlock();
			_servers.push_back(static_cast<struct ServerSettings *>(currentBlockPtr));
		}
		else if (currentBlock == SERVER && line.find("location /") != std::string::npos && line.find(" {") != std::string::npos)
		{
			currentBlock = LOCATION;
			currentBlockPtr = initLocationBlock(line);
		}
		else if (currentBlock == SERVER && line.find("errorpages {") != std::string::npos)
		{
			currentBlock = ERROR_PAGE;
			currentBlockPtr = new std::map<int, std::string>;
		}
		else if (line.find("}") != std::string::npos)
		{
			if (_handleBlockEnd(&currentBlock, currentBlockPtr) == 1) {
				if (currentBlockPtr != nullptr)
					deleteBlock(currentBlock, currentBlockPtr);
				throw InvalidParameterException(_lineNr);
			}
		}
		else
		{
			switch (_handleBlockContent(line, currentBlock, currentBlockPtr))
			{
				case 1:
					if (currentBlockPtr != nullptr)
						deleteBlock(currentBlock, currentBlockPtr);
					throw InvalidParameterException(_lineNr);
					break;
				case 2:
					if (currentBlockPtr != nullptr)
						deleteBlock(currentBlock, currentBlockPtr);
					std::cerr << RED << "Line " << _lineNr << ": " << RESET;
					std::flush(std::cerr);
					throw NoSuchFileException();
					break;
			}
		}
		_lineNr++;
	}
}

int	Config::_handleBlockEnd(configBlock *currentBlock, void *currentBlockPtr)
{
	switch (*currentBlock) {
		case SERVER:
			*currentBlock = NONE;
			break;
		case LOCATION:
			_servers.back()->_locations.push_back(static_cast<struct LocationSettings *>(currentBlockPtr));
			*currentBlock = SERVER;
			break;
		case ERROR_PAGE:
			_servers.back()->_errorPages = static_cast<std::map<int, std::string>*>(currentBlockPtr);
			*currentBlock = SERVER;
			break;
		default:
			return 1;
	}
	return 0;
}

int	Config::_handleBlockContent(std::string line, configBlock currentBlock, void *currentBlockPtr)
{
	switch (currentBlock) {
		case SERVER:
			return parseServer(line, static_cast<struct ServerSettings *>(currentBlockPtr));
		case LOCATION:
			return parseLocation(line, static_cast<struct LocationSettings *>(currentBlockPtr));
		case ERROR_PAGE:
			return parseErrorPage(line, static_cast<std::map<int, std::string> *>(currentBlockPtr));
		default:
			return 1;
	}
	return 0;
}

void	Config::_checkMandatoryParameters(const struct ServerSettings *server)
{
	bool	hasRootLocation = false;

	if (server->_serverName.empty())
		throw MissingParameterException("server_name");
	if (server->_rootFolder.empty())
		throw MissingParameterException("root folder");
	if (server->_uploadDir.empty())
		throw MissingParameterException("upload_dir");
	if (server->_index.empty())
		throw MissingParameterException("server index");
	if (server->_port == 0)
		throw MissingParameterException("listen port");
	if (server->_maxBodySize == 0)
		throw MissingParameterException("client_max_body_size");

	for (const auto location : server->_locations)
	{
		if (location->_locationId.empty())
			throw MissingParameterException("location name");
		if (location->_locationId == "/")
			hasRootLocation = true;
	}

	if (!hasRootLocation)
		throw MissingParameterException("root location");
}

void	Config::printServers() const
{
	std::cout << "\nnb servers = " << _servers.size() << "\n" << std::endl;

	for (const auto server : _servers) {
		std::cout << BLUE << "Server: " << server->_serverName << RESET << std::endl;
		std::cout << "index: " << server->_index <<  std::endl;
		std::cout << "max body size: " << server->_maxBodySize <<  std::endl;
		std::cout << "port: " << server->_port << std::endl;
		std::cout << "root folder: " << server->_rootFolder <<  std::endl;
		std::cout << "upload dir: " << server->_uploadDir <<  std::endl;
		std::cout << "\nlocation size: " << server->_locations.size() << "\n" << std::endl;

		for (const auto location : server->_locations) {
			std::cout << PURPLE << "Location ID: " << location->_locationId << RESET << std::endl;
			std::cout << "dirListing: " << (location->_dirListing ? "on":"off") << std::endl;
			std::cout << "index: " << location->_index << std::endl;
			std::cout << "max body size: " << location->_maxBodySize << std::endl;
			std::cout << "root folder: " << location->_rootFolder << std::endl;
			for (const auto& redirect : location->_redirect)
				std::cout << "redirect: " << redirect.first << " " << redirect.second << std::endl;
			for (const auto& method : location->_allowedMethods)
				std::cout << "allow " << method << std::endl;
			std::cout << std::endl;
		}

		for (const auto& errorPage : *server->_errorPages)
			std::cout << "error page: " << errorPage.first << " " << errorPage.second << std::endl;
		
		std::cout << std::endl;
	}
}
