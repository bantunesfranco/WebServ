/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpRequest.hpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/03 10:27:35 by carlo         #+#    #+#                 */
/*   Updated: 2023/12/09 21:25:28 by bfranco       ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef HTTPREQUEST_H
# define HTTPREQUEST_H

#include "Server.hpp"
#include "Header.hpp"
#include "Uri.hpp"
#include <sstream>
#include <algorithm>

class HttpRequest {

	public:
		HttpRequest() = delete;
		HttpRequest(const Server *server);
		HttpRequest(const std::string& request, const Server *server);
		HttpRequest(const HttpRequest& origin);
		const HttpRequest& operator=(const HttpRequest& rhs);
		~HttpRequest(void);

		Uri										uri;
		Header*									headers;

	// ============= Getters ================
		std::string								getHostname(void) const;
		std::string								getMethod(void) const;
		std::string								getProtocol(void) const;
		std::string								getBody(void) const;
		std::string								getUri(void);
		int										getRequestStatus(void) const;
		std::string								getQueryString(void) const;
		char**									getEnvArray(void) const;

	// ============= Setters ================
		void									setMethod(const std::string& method);
		void									setProtocol(const std::string& protocol);
		void									setBody(const std::string& body);
		void									setUri(const std::string& str);
		void									addEnvironVar(const std::string& key, const std::string& value);
		void									setRequestStatus(int value);
		void									fillStandardHeaders(void);

	// ============= exception ================
		class parsingException : public std::exception {
			public:
				parsingException(int errorCode, const std::string& message) : _errorCode(errorCode), _message(message) {}
				const char*	what() const noexcept override		{	return _message.c_str();	}
				int			getErrorCode() const 				{	return _errorCode;			}

			private:
				int								_errorCode;
				std::string						_message;
		};
	
	private:
		std::string								_method;
		std::string								_protocol;
		std::map<std::string, std::string>		_environVars;
		std::string								_body;
		int										_requestStatus;
		const Server*							_server;
		std::string								_hostname;
		// const struct LocationSettings*			_settings;
};

#endif
