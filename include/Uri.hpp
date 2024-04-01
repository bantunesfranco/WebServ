/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   Uri.hpp                                            :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/03 12:12:15 by carlo         #+#    #+#                 */
/*   Updated: 2023/12/11 13:34:25 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef URI_H
# define URI_H

#include "defines.hpp"
#include <regex>

class Uri {

	public:
		Uri(void);
		Uri(const std::string& uri);
		Uri(const Uri& origin);
		const Uri& operator=(const Uri& rhs);
		~Uri(void);

		std::string		serializeUri(void);

		// ========== Getters ===========
		std::string							getScheme(void) const;
		std::string							getAuthority(void) const;
		std::string							getPath(void) const;
		std::string							getExtension(void) const;
		bool								getIsBinary(void) const;
		std::string							getQuery(void) const;
		std::string							getFragment(void) const;
		std::string							getUserInfo(void) const;
		std::string							getHost(void) const;
		int									getPort(void) const;
		std::map<std::string, std::string>	getQueryMap(void) const;
		std::string							getMime(std::string key) const;
		std::string							getExecutable(void) const;
		std::string							getPathInfo(void) const;
		bool								isDir(void);

		// ========== Setters ===========
		void								mapQueries(void);
		void								setExtension(void);
		void								setQuery(std::string query);
		void								setFragment(std::string fragment);
		void								setPath(std::string path);

		// ========== other ==========
		bool								isValidExtension(void);

	private:
		void								splitAuthority(void);
		
		std::string							_scheme;
		std::string							_authority;
		std::string							_path;
		std::string							_extension;
		bool								_isBinary;
		std::string							_query;
		std::map<std::string, std::string>	_queryMap;
		std::string							_fragment;
		std::string							_userinfo;
		std::string							_host;
		int									_port;
};

#endif
