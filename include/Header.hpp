/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Header.hpp                                         :+:    :+:            */
/*                                                     +:+                    */
/*   By: carlo <carlo@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/13 09:46:19 by carlo         #+#    #+#                 */
/*   Updated: 2023/12/05 06:59:40 by cariencaljo   ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef HEADER_H
# define HEADER_H

#include "defines.hpp"
#include <sstream>
#include <algorithm>

class Header {

public:
	Header(void);
	Header(const std::string& headerBlock);
	Header(const Header& origin);
	const Header& operator=(const Header& rhs);
	~Header(void);

// ========== Getters ===========
	std::map<std::string, std::string>	getHeaders(void) const;
	std::vector<char*>					getHeaderVector(void) const;
	std::string							getHeaderValue(std::string key) const;
	bool								isHeader(std::string key) const;

// ========== Setters ===========
	void								addHeader(const std::string& key, const std::string& value);
	void								setHeader(const std::string& key, const std::string& value);

//========== Other ===============
	void								setHeaderVector(void);
	void								printHeaders(void);
	std::string							serializeHeaders(void);

private:
	std::string							_headerBlock;
	std::map<std::string, std::string>	_headerMap;
	std::vector<char*>					_headerVector;
};

#endif
