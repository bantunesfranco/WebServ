/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Header.cpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: ccaljouw <ccaljouw@student.42.fr>            +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/13 09:46:19 by carlo         #+#    #+#                 */
/*   Updated: 2023/12/08 14:14:35 by carlo         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "Header.hpp"

std::string		removeWhitespaces(std::string str);

Header::Header() : _headerBlock(), _headerMap(), _headerVector() {}

Header::Header(const std::string& headerBlock) : _headerBlock(headerBlock), _headerMap(), _headerVector()	{
	
	std::istringstream HeaderStream(headerBlock);
	std::string headerLine;

	while(std::getline(HeaderStream, headerLine)) {
		std::size_t columPos = headerLine.find(":");
		if (columPos != std::string::npos)
		{
			std::string key = headerLine.substr(0,columPos);
			std::string value = headerLine.substr(columPos + 1);

			//header keys are case insensitive
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
				
			value = removeWhitespaces(value);
			_headerMap[key] = value;
		}
	}
	setHeaderVector();
}

Header::Header(const Header& origin) {
	*this = origin;
}

const Header& Header::operator=(const Header& rhs) {
	if (this != &rhs)
	{
		_headerBlock				= rhs._headerBlock;
		_headerMap.clear();
		_headerMap					= rhs._headerMap;
		_headerVector.clear();
		_headerVector				= rhs._headerVector;
	}
	return *this;
}

Header::~Header(void) {
	_headerMap.clear();
	for (char* str : _headerVector)
		free(str);
	_headerVector.clear();
}


// ========== Getters ===========
std::map<std::string, std::string>	Header::getHeaders(void) const		{	return _headerMap;		}
std::vector<char*>					Header::getHeaderVector(void) const	{	return _headerVector;	}


std::string	Header::getHeaderValue(std::string key) const {
	for (const auto& headerPair : _headerMap) {
		if (headerPair.first == key)
			return headerPair.second;
	}
	return "";
}

 
bool	Header::isHeader(std::string key) const {
	auto it = _headerMap.find(key);
	if (it != _headerMap.end())
		return true;
	return false;
}


// ========== Setters ===========
void	Header::addHeader(const std::string& key, const std::string& value) {
	if(!value.empty())
		_headerMap[key] = value;
}


void	Header::setHeader(const std::string& key, const std::string& value) {
	for (auto& headerPair : _headerMap) {
		if (key == headerPair.first) {
			headerPair.second = value;
			return;
		}
	}
	_headerMap[key] = value;	
}


//========== Other ===============
void	Header::setHeaderVector(void) {
	
	// make c_string array from headers. first a vector of c_strings, then make pairs and capitalize
	for (const auto& pair : _headerMap) {
		std::string key = pair.first;
		std::string value = pair.second;
		
		for (char& c : key) {	
			c = toupper(static_cast<unsigned char>(c)); 
			if (c == '-')
				c = '_';
		}
		for (char& c : value) {	
			if (c == ',')
				c = ';';
		}
		std::string line = key + "=" + value;
		_headerVector.push_back(strdup(line.c_str()));
	}
	_headerVector.push_back(nullptr);
}


void	Header::printHeaders(void) {
	std::cout << BLUE << "Headers:\n" << RESET << std::endl;
	for (const auto& pair : _headerMap) {
		std::cout << BLUE << "key: " << pair.first << " | value: " << pair.second << RESET << std::endl;
	}
}

std::string	Header::serializeHeaders(void) {
	std::string	temp;
	for(const auto& pair : _headerMap)
		temp += pair.first + ": " + pair.second + LINE_END;
	return temp;
}
