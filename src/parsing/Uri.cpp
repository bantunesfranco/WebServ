/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Uri.cpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/03 12:17:27 by carlo         #+#    #+#                 */
/*   Updated: 2023/12/11 17:09:35 by cwesseli      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Uri.hpp"
#include "HttpRequest.hpp" //only used for exeption

Uri::Uri() : _scheme(), _authority(), _path(), _extension(), _isBinary(false), _query(), _queryMap(), _fragment(), _userinfo(), _host(), _port() {}


// regex teken directly from RFC 2396 : ^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))? added R for ignoring escape characterss
Uri::Uri(const std::string& uri) : _isBinary(false) {
	
	std::regex uriRegex(R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)");
	std::smatch matches;

	if(std::regex_match(uri, matches, uriRegex)) {
		_scheme 	= matches[2].str();
		_authority 	= matches[4].str();
		_path 		= matches[5].str();
		_query 		= matches[7].str();
		_fragment 	= matches[9].str();

		if (_authority.empty() && !_path.empty())
			_authority = _path;
	} 
	else 
		throw HttpRequest::parsingException(400, "bad request");

	// std::cout << BLUE << "URI components: '" << "'\n\tsceme: '" << _scheme << "'\n\tauthority: '" << _authority << "'\n\tpath: '" << _path << "'\n\tquery: '" << _query << "'\n\tfragment" << _fragment << "\n" << RESET << std::endl;
	splitAuthority();
	
	//scheme and host are case insensitive and as such are normalized here
	std::transform(_scheme.begin(), _scheme.end(), _scheme.begin(), ::tolower);
	std::transform(_host.begin(), _host.end(), _host.begin(), ::tolower);

	setExtension();
	mapQueries();
}

Uri::Uri(const Uri& origin) { 
	*this = origin;
}

const Uri& Uri::operator=(const Uri& rhs) {
	if (this != &rhs) {
		_scheme			= rhs._scheme;
		_authority		= rhs._authority;
		_path			= rhs._path;
		_query			= rhs._query;
		_fragment		= rhs._fragment;
		_extension		= rhs._extension;
		_isBinary		= rhs._isBinary;
		_queryMap.clear();
		_queryMap		= rhs._queryMap;
		_userinfo		= rhs._userinfo;
		_host			= rhs._host;
		_port			= rhs._port;
	}
	return *this;
}

Uri::~Uri() {
	_queryMap.clear();
}


//==============================

void	Uri::splitAuthority() {

	//check for optional userinfo	
	size_t atPos	= _authority.find("@");
	if (atPos != std::string::npos) {
		_userinfo	= _authority.substr(0, atPos);
		_host		= _authority.substr(atPos + 1);
	}
	else
		_host = _authority;

	// todo: check for optional port and validity
	size_t columPos	= _host.find(":");
	if (columPos != std::string::npos) {
		std::string portString  = _host.substr(columPos + 1);
		_host = _host.substr(0, columPos);

		try {
			_port = std::stoi(portString);
		}
		//handle invalid port
		catch (const std::invalid_argument& e) {
			std::cerr << RED << "Invalid Port Number" << e.what() << std::endl;
		}
		//handle out of range port
    	catch (const std::out_of_range& e) {
            std::cerr << RED << "Port number out of range: " << e.what() << std::endl;
        }
	}
}


std::string	Uri::serializeUri() {
	
	std::string serializedUri;

	if (!_scheme.empty())
		serializedUri += _scheme + ":";

	if (!_authority.empty())
		serializedUri += "//" + _authority;

	if (!_path.empty()) {
		if (_path[0] != '/')
			serializedUri += "/";
		serializedUri += _path;
	}
	
	if (!_query.empty())
		serializedUri += "?" + _query;
	
	if (!_fragment.empty())
		serializedUri += "#" + _fragment;
		
	return serializedUri;
}

//=========== getters =================

bool		Uri::getIsBinary(void) const						{	return _isBinary;	}
int			Uri::getPort() const								{	return _port;		}
std::string	Uri::getScheme() const								{	return _scheme;		}
std::string	Uri::getAuthority() const							{	return _authority;	}
std::string	Uri::getPath() const								{	return _path; 		}
std::string	Uri::getExtension() const							{	return _extension;	}
std::string	Uri::getQuery() const								{	return _query;		}
std::string	Uri::getFragment() const							{	return _fragment;	}
std::string	Uri::getUserInfo() const							{	return _userinfo;	}
std::string	Uri::getHost() const								{	return _host;		}
std::map<std::string, std::string> Uri::getQueryMap(void) const	{	return _queryMap;	}


std::string	Uri::getExecutable(void) const {
	if (_path.empty())
		return "";
	
	std::string temp = _path;
	if (temp[0] == '/')
		temp = temp.substr(1);
	size_t slash = temp.find("/");
	if (slash != std::string::npos)
		return temp.substr(0, slash);
	
	return "";
}


std::string	Uri::getPathInfo(void) const {
	if (_path.empty())
		return "";

	std::string temp = _path;
	if (temp[0] == '/')
		temp = temp.substr(1);
	size_t slash = temp.find("/");
	if (slash != std::string::npos)
		return  temp.substr(slash + 1);
	
	return "";
}


bool	Uri::isDir() {
	if (_path.empty())
		return false;
	return _path.back() == '/';
}


//=========== setters =================
void Uri::mapQueries() {
	
	std::istringstream qurieStream(_query);
	
	std::string token;
	while(getline(qurieStream, token, '&'))
	{
		size_t equalPosition = token.find('=');
		if (equalPosition != std::string::npos)
		{
			std::string key = token.substr(0, equalPosition);
			std::string value = token.substr(equalPosition + 1); //todo: URL-decode key and value is needed
			_queryMap[key] = value;
		}
		else
			_queryMap[token] = "";
	}
}


std::string	Uri::getMime(std::string extension) const
{
	for (char& c : extension)
		c = std::tolower(c);
	for (const auto& pair : acceptedExtensions )
	{
		if (extension == pair.first)
			return pair.second;
	}
	return ".txt";
}


void	Uri::setExtension()
{
	size_t periodPos = _path.rfind('.');
	if (periodPos != std::string::npos)
		_extension = _path.substr(periodPos);
	else
		_extension = "";
	
	for (const auto& ext : binaryExtensions) {
		if (ext == _extension)
			_isBinary =  true;	
	}
}


void	Uri::setPath(std::string path) {
	_path = path;
}


void Uri::setQuery(std::string query) {
	_query = query;
	mapQueries();
}


void Uri::setFragment(std::string fragment) {
	_fragment = fragment;
}

bool	Uri::isValidExtension() {
	std::string actualExtension = getExtension();
	for (const auto& pair : acceptedExtensions)
		if (pair.first == actualExtension)
			return true;
	return false;
}
