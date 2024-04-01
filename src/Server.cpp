/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Server.cpp                                   :+:    :+:            */
/*                                                     +:+                    */
/*   By: cariencaljouw <cariencaljouw@student.co      +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/03 11:11:47 by cariencaljo   #+#    #+#                 */
/*   Updated: 2023/11/03 17:32:38 by cariencaljo   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include <Server.hpp>
#include "webServ.hpp" //fix

// ============= con-/destructors ================

Server::Server() : _fd(0), _conn(nullptr) {}

Server::Server( Server const & src ) { 
	_fd = src.get_FD();
	_serverAddr = src.get_serverAddr();
	_knownClientIds = src.get_knownClientIds();
	_timeout = src.get_timeout();
	_maxNrOfRequests = src.get_maxNrOfRequests();
	_settings = src.get_settings();
	_conn = src.get_connection();
}

Server::~Server() {	
	if (_fd)
		close(_fd); 
	if (_conn)
		delete _conn;
}

// ============= Methods ================

int	Server::assign_name()
{
	int enable = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)))
		return 1;
	if (bind(_fd, reinterpret_cast<struct sockaddr*>(&_serverAddr), sizeof(_serverAddr)) == -1) {
		return 1;
	}
	return 0;
}

int	Server::set_to_listen(int backlog)
{
	if (listen(_fd, backlog) == -1) {
		return 1;
	}
	return 0;
}

int Server::initServer(struct ServerSettings *settings, int epollFd, double timeout, int maxNrOfRequests)
{
	try {
		_timeout = timeout,
		_maxNrOfRequests = maxNrOfRequests;
		_settings.push_back(settings);
		_serverAddr.sin_family = AF_INET;
		_serverAddr.sin_addr.s_addr = INADDR_ANY;
		_serverAddr.sin_port = htons(settings->_port);
	
		if ((_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK , 0)) == -1) 
			throw ServerException(settings->_serverName + " socket: ");
		if (assign_name())
			throw ServerException(settings->_serverName + " bind: ");
		if (set_to_listen(100))
			throw ServerException(settings->_serverName + " listen: ");
		if (register_server(epollFd, this->_fd, this))
			throw ServerException(settings->_serverName + " register: ");
	}
	catch (const ServerException& e) {
		std::cerr << RED << e.what() << RESET << std::endl;
		return 1;
	}
	std::cout << GREEN << "Server: " << settings->_serverName << ", listening on port "  << settings->_port << RESET << std::endl;
	return 0;
}

void	Server::addClientId(std::string newCookieValue) {
	_knownClientIds.insert(std::make_pair(newCookieValue, 1));
}

int	Server::checkClientId(std::string id, struct connection* conn) {

	for (auto& pair : _knownClientIds)
	{
		if (pair.first == id)
		{
			if (conn->nr_of_requests == 1) 
				pair.second += 1;
			return  1;
		}
	}
	return 0;
}

void	Server::addSubDomain(struct ServerSettings *settings) {	_settings.push_back(settings);	}

// ============= Getters ================
uint16_t							Server::get_port(void) const {	return	_settings.front()->_port; }
int									Server::get_FD() const { return _fd; }
struct sockaddr_in					Server::get_serverAddr() const { return _serverAddr; }
std::map<std::string, int>			Server::get_knownClientIds() const { return _knownClientIds; }	
double								Server::get_timeout() const { return _timeout; }
int									Server::get_maxNrOfRequests() const { return _maxNrOfRequests; }
std::list<struct ServerSettings *>	Server::get_settings() const { return _settings; }
struct connection*					Server::get_connection() const { return _conn; }

struct ServerSettings*		Server::get_hostSettings(std::string host) const {
	struct ServerSettings *hostSettings = _settings.front();
	for (auto& setting : _settings) {
		if (setting->_serverName == host)
			hostSettings = setting;
	}
	return hostSettings;
}

std::string	Server::inherrit_rootFolder(struct ServerSettings *hostSettings, std::string location) const {
	std::string find = findNextBestMatch(location);
	while (!find.empty())
	{
		// find location
		for (auto& loc : hostSettings->_locations) {
			if (loc->_locationId == find) {
				if (!loc->_rootFolder.empty())
					return (loc->_rootFolder);
				break;
			}
		}
		// if no rootfolder set in location blocks
		if (find == "/")
			break;

		// find next bext match to check for location
		find = findNextBestMatch(find);
	}
	return hostSettings->_rootFolder;
}

std::string	Server::inherrit_uploadDir(struct ServerSettings *hostSettings, std::string location) const {
	std::string find = findNextBestMatch(location);
	while (!find.empty())
	{
		// find location
		for (auto& loc : hostSettings->_locations) {
			if (loc->_locationId == find) {
				if (!loc->_uploadDir.empty())
					return (loc->_uploadDir);
				break;
			}
		}
		// if no rootfolder set in location blocks
		if (find == "/")
			break;

		// find next bext match to check for location
		find = findNextBestMatch(find);
	}
	return hostSettings->_uploadDir;
}


std::string	Server::inherrit_index(struct ServerSettings *hostSettings, std::string location) const {
	std::string find = findNextBestMatch(location);
	while (!find.empty())
	{
		// find location
		for (auto& loc : hostSettings->_locations) {
			if (loc->_locationId == find) {
				if (!loc->_index.empty())
					return (loc->_index);
				break;
			}
		}
		// if no rootfolder set in location blocks
		if (find == "/")
			break;

		// find next bext match to check for location
		find = findNextBestMatch(find);
	}
	return hostSettings->_index;
}

size_t	Server::inherrit_maxBodySize(struct ServerSettings *hostSettings, std::string location) const {
	std::string find = findNextBestMatch(location);
	while (!find.empty())
	{
		// find location
		for (auto& loc : hostSettings->_locations) {
			if (loc->_locationId == find) {
				if (loc->_maxBodySize)
					return (loc->_maxBodySize);
				break;
			}
		}
		// if no rootfolder set in location blocks
		if (find == "/")
			break;

		// find next bext match to check for location
		find = findNextBestMatch(find);
	}
	return hostSettings->_maxBodySize;
}

std::set<std::string> Server::inherrit_allowedMethods(struct ServerSettings *hostSettings, std::string location) const {
	std::string find = findNextBestMatch(location);
	while (!find.empty())
	{
		// find location
		for (auto& loc : hostSettings->_locations) {
			if (loc->_locationId == find) {
				if (!loc->_allowedMethods.empty())
					return (loc->_allowedMethods);
				break;
			}
		}
		// if no rootfolder set in location blocks
		if (find == "/") {
			break;
		}
		// find next bext match to check for location
		find = findNextBestMatch(find);
	}
	std::set<std::string> empty;
	return empty;
}

std::string	Server::findNextBestMatch(std::string location) const {
	size_t pos = location.rfind('/');
	if (pos == 0 || pos > location.length())
		location = "/";
	else
		location.resize(pos);
	return location;
}

struct LocationSettings*	Server::get_locationSettings(std::string host, std::string location) const {
	// std::cout << "getting location for host: " << host << " and location: " << location << std::endl;
	struct ServerSettings		*hostSettings = get_hostSettings(host);
	struct LocationSettings		*locationSettings = nullptr;
	std::string					toFind = location;

	while (!toFind.empty())
	{
		// find location
		for (auto& loc : hostSettings->_locations) {
			if (loc->_locationId == toFind) {
				locationSettings = loc;
				break;
			}
			else {
				size_t pos = toFind.find('.');
				if (pos != toFind.npos) {
					if (loc->_locationId.substr(1) == toFind.substr(pos)) {
						locationSettings = loc;
						break;
					}
				}
			}
		}

		// check for missing values that should be inherited
		if (locationSettings) {
			// std::cout << "\nlocation: " << location << std::endl;
			// std::cout << "id: " << locationSettings->_locationId << "\nmethods: " << locationSettings->_allowedMethods.size() << std::endl;
			// std::cout << "root: " << locationSettings->_rootFolder << "\nuploadDir: " << locationSettings->_uploadDir << std::endl;
			// std::cout << "index: " << locationSettings->_index << "\bbodySize: " << locationSettings->_maxBodySize << std::endl;
			if (locationSettings->_rootFolder.empty())
				locationSettings->_rootFolder = inherrit_rootFolder(hostSettings, toFind);
			if (locationSettings->_uploadDir.empty())
				locationSettings->_uploadDir = inherrit_uploadDir(hostSettings, toFind);
			if (locationSettings->_index.empty())
				locationSettings->_index = inherrit_index(hostSettings, toFind);
			if (!locationSettings->_maxBodySize)
				locationSettings->_maxBodySize = inherrit_maxBodySize(hostSettings, toFind);
			if (locationSettings->_allowedMethods.empty())
				locationSettings->_allowedMethods = inherrit_allowedMethods(hostSettings, toFind);
			// std::cout << "\nid: " << locationSettings->_locationId << "\nmethods: " << locationSettings->_allowedMethods.size() << std::endl;
			// std::cout << "root: " << locationSettings->_rootFolder << "\nuploadDir: " << locationSettings->_uploadDir << std::endl;
			// std::cout << "index: " << locationSettings->_index << "\bbodySize: " << locationSettings->_maxBodySize << std::endl;
			break;
		}
		// find next bext match to check for location
		toFind = findNextBestMatch(toFind);
	}
	if (!locationSettings)
		std::cerr << RED << "Error\nincorrect configuration " << hostSettings->_serverName << RESET << std::endl;
	return locationSettings;
}

std::string	Server::get_serverName(std::string host) const { 
	return get_hostSettings(host)->_serverName;
}

std::string	Server::get_rootFolder(std::string host) const { 
	return get_hostSettings(host)->_rootFolder;
}

std::string Server::get_uploadDir(std::string host) const {
	return get_hostSettings(host)->_uploadDir;
}

std::map<int, std::string>*	Server::get_errorPages(std::string host) const {
	return get_hostSettings(host)->_errorPages;
}

size_t	Server::get_maxBodySize(std::string host) const {
	return get_hostSettings(host)->_maxBodySize; 
}

std::string	Server::get_index(std::string host, std::string location) const {
	struct LocationSettings* settings = get_locationSettings(host, location);
	if (settings)
		return settings->_index;
	return "";
}

bool Server::get_dirListing(std::string host, std::string location) const {
	struct LocationSettings* settings = get_locationSettings(host, location);
	if (settings && settings->_locationId == location)
		return settings->_dirListing;
	else
		return false;
}

std::map<int, std::string>	Server::get_redirect(std::string host, std::string location) const {
	struct LocationSettings* settings = get_locationSettings(host, location);
	if (settings && settings->_locationId == location)
		return settings->_redirect;
	else {
		// std::cout << "sending empty redirect" << std::endl;
		std::map<int, std::string> map;
		return map;
	}
}

std::string	Server::get_rootFolder(std::string host, std::string location) const {
	struct LocationSettings* settings = get_locationSettings(host, location);
	if (settings)
		return settings->_rootFolder;
	return "";
}

size_t	Server::get_maxBodySize(std::string host, std::string location) const {
	struct LocationSettings* settings = get_locationSettings(host, location);
	if (settings)
		return settings->_maxBodySize;
	return 0;
}

std::string	Server::get_uploadDir(std::string host, std::string location) const {
	struct LocationSettings* settings = get_locationSettings(host, location);
	if (settings)
		return settings->_uploadDir;
	return "";
}

std::set<std::string> Server::get_allowedMethods(std::string host, std::string location) const {
	struct LocationSettings* settings = get_locationSettings(host, location);
	if (settings)
		return settings->_allowedMethods;
	else {
		std::set<std::string> empty;
		return empty;
	}
}

// ============= setters ================
void	Server::set_connection(struct connection *conn) {
	_conn = conn;
}

/* ************************************************************************** */

std::list<Server> initServers(const Config& config, int epollFd)
{
	std::list<Server> 					servers;
	std::map<uint16_t, std::string> 	serverMap;
	std::set<uint16_t>					ports;
	std::list<struct ServerSettings *>	settings = config.getServers();

	try {
		for (auto setting : settings)
		{
			std::set<uint16_t>::iterator p = ports.find(setting->_port);
			if (p == ports.end()) {
				ports.insert(setting->_port);
				serverMap.insert(std::make_pair(setting->_port, setting->_serverName));
				servers.push_back(Server());
				if (servers.back().initServer(setting, epollFd, TIMEOUT, MAX_NR_REQUESTS) == 1)
					servers.pop_back();
				(void)epollFd;
			}
			else {
				for (auto& server : servers) {
					for (auto& pair : serverMap) {
						if (pair.first == setting->_port && pair.second == setting->_serverName)
							throw std::runtime_error(setting->_serverName + ":" + std::to_string(setting->_port) + " allready configured");
					}
					if (server.get_port() == setting->_port) {
						server.addSubDomain(setting);
						std::cout << GREEN << setting->_serverName << ", listening on port "  \
															<< setting->_port << RESET << std::endl;
						break ;
					}
				}
			}
		}
	}
	catch (const std::runtime_error& e) {
		std::cerr << RED << "Error\n" << e.what() << RESET << std::endl;
	}
	return (servers);
}
