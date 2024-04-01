/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   HttpResponse.cpp                                    :+:    :+:            */
/*                                                     +:+                    */
/*   By: carlo <carlo@student.codam.nl>               +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/01 14:21:11 by carlo         #+#    #+#                 */
/*   Updated: 2023/11/03 16:09:43 by carlo         ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "webServ.hpp" //fix

std::string getTimeStamp();

// HttpResponse::HttpResponse() :  headers(), _protocol(HTTP_PROTOCOL), _statusCode(), _body() {
// 	headers = new Header();
// 	fillStandardHeaders();
// }

HttpResponse::HttpResponse(const HttpRequest& request) {
	headers				= request.headers;
	_protocol 			= HTTP_PROTOCOL; //todo from config
	_statusCode			= request.getRequestStatus();
	_body				= request.getBody();
	fillStandardHeaders();
}


HttpResponse::HttpResponse(const HttpResponse& origin) {
	*this = origin;
}


const HttpResponse& HttpResponse::operator=(const HttpResponse& rhs) {
	if (this != &rhs) {
		headers			= rhs.headers;
		_protocol		= rhs._protocol;
		_statusCode		= rhs._statusCode;
		_body			= rhs._body;
	}
	headers->setHeader("Last-Modified", getTimeStamp());
	return *this;
}


HttpResponse::~HttpResponse() {}


// ============= Getters ================
int				HttpResponse::getStatusCode()	const {	return _statusCode;	}
std::string		HttpResponse::getBody() 		const {	return _body;		}



// ============= Setters ================
void			HttpResponse::setProtocol(const std::string& protocol)	{ 
	_protocol 	= 	protocol;

	headers->setHeader("Last-Modified", getTimeStamp());
}


void			HttpResponse::setStatusCode(int status) {
	_statusCode	= status;

	headers->setHeader("Last-Modified", getTimeStamp());
}


void			HttpResponse::reSetBody(const std::string& filePath, bool isBinary)	{
	std::ifstream	inputFile;
	int				length;

	if (!_body.empty())
		_body.clear();
	if (isBinary)
		inputFile.open(filePath, std::ifstream::binary);
	else {
		inputFile.open(filePath);
		// std::cout << "input file opened: " << filePath << std::endl;
	}
	if (inputFile.good()) {
		inputFile.seekg(0, inputFile.end);
		length = inputFile.tellg();
		inputFile.seekg(0, inputFile.beg);

		char buffer[length];
		inputFile.read(buffer, length);
		_body.append(buffer, length);
		inputFile.close();	
	}
	else {
		throw HttpRequest::parsingException(500, "Internal Server error");
	}
	
	headers->setHeader("Content-Length", std::to_string(length));
	headers->setHeader("Last-Modified", getTimeStamp());
}


void		HttpResponse::fillStandardHeaders() {
	headers->setHeader("Date", getTimeStamp());
	headers->setHeader("Last-Modified", getTimeStamp());
	headers->setHeader("Content-Length", std::to_string(_body.length()));
}

void		HttpResponse::setResponse(connection *conn){
	std::set<int> setClose{408, 429, 500, 504};
	
	if (setClose.find(getStatusCode()) != setClose.end()) {
		conn->close_after_response = 1;
		headers->setHeader("Connection", "close");		
	}
	conn->response = serializeResponse();
	conn->request.clear();
	conn->state = WRITING;
}

std::string	HttpResponse::serializeResponse() {
	std::string serializedResponse;
	
	std::string reason = "empty";
	for (auto& pair : HttpResponseCodes) {
		if (pair.first == _statusCode)
			reason = pair.second;
	}

	serializedResponse += _protocol + " " + std::to_string(_statusCode) + " " + reason + LINE_END + headers->serializeHeaders() + LINE_END + _body + LINE_END;
	return serializedResponse;
}

std::map<int, std::string> HttpResponseCodes = {
	{ 200, "OK"}, 					// The request was fulfilled. 
	{ 201, "Created"},				// Following a POST command, this indicates success, but the textual part of the response line indicates the URI by which the newly created document should be known. 
	{ 202, "Accepted"},				// The request has been accepted for processing, but the processing has not been completed.
	{ 204, "No response"},			// Server has received the request but there is no information to send back, and the client should stay in the same document view. 
 	{ 400, "Bad request"},			// The request had bad syntax or was inherently impossible to be satisfied. 
 	{ 401, "Unauthorized"},			// The parameter to this message gives a specification of authorization schemes which are acceptable. The client should retry the request with a suitable Authorization header. 
 	{ 403, "Forbidden"},			// The request is for something forbidden. Authorization will not help. 
	{ 404, "Not found"},			//  The server has not found anything matching the URI given 
	{ 405, "Method Not Allowed"},
	{ 422, "Unprocessable Entity"}, // Indicates that the server understands the content type of the request entity, and the syntax of the request entity is correct, but it was unable to process the contained instructions. 
	{ 500, "Internal Error"},		// The server encountered an unexpected condition which prevented it from fulfilling the request. 
	{ 501, "Not implemented"},		// The server does not support the facility required. 
	{ 502, "Service temporary overloaded"},
	{ 503, "Gateway timeout"},
	{ 505, "version not supported"}
};
