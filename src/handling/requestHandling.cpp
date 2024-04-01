/* ************************************************************************** */
/*                                                                            */
/*                                                        ::::::::            */
/*   requestHandling.cpp                                :+:    :+:            */
/*                                                     +:+                    */
/*   By: bfranco <bfranco@student.codam.nl>           +#+                     */
/*                                                   +#+                      */
/*   Created: 2023/11/03 23:45:10 by cariencaljo   #+#    #+#                 */
/*   Updated: 2023/12/11 17:23:21 by cwesseli      ########   odam.nl         */
/*                                                                            */
/* ************************************************************************** */

#include "webServ.hpp"
#include <fstream>
// #include <functional>

using RequestHandler = std::function<void(int, connection*, HttpRequest&)>;

std::string	getHandler(connection *conn, HttpRequest& request) {
	std::string	method		= request.getMethod();
	std::string host 		= request.getHostname() ;
	std::string location 	= request.uri.getPath();
	std::string executable	= request.uri.getExecutable();

	// //***testprint
	// std::cout << "method: " << method << " host: " << host << " location: " << location << std::endl;
	
	if (!conn->server->get_redirect(host, location).empty())
		return "REDIRECT";
	std::set<std::string> allowedMethods = conn->server->get_allowedMethods(host, location);
	if (allowedMethods.find(method) == allowedMethods.end())
		throw HttpRequest::parsingException(405, "Method not Allowed");
	if (request.uri.isDir()) {
		// std::cout << "going to handleDIR" << std::endl;
		return "DIRLISTING"; 
	}
	std::string root = conn->server->get_rootFolder(host, location);
	// std::cout << "was: " << request.uri.getPath() << std::endl;
	// request.uri.setPath(root + request.uri.getPath());
	// std::cout << "now: " << request.uri.getPath() << std::endl;
	std::vector<std::string>ext = executableExtensions;

	if (executable == "cgi-bin" || (executable.empty() &&  std::find(ext.begin(), ext.end(), request.uri.getExtension()) != ext.end()))
	{
		// std::cout << "going to handleCGI" << std::endl;
		return "CGI";
	}
	if (method == "GET") {
		// std::cout << "going to handleGET" << std::endl;
		return "GET";
	}
	else if (method == "POST") {
		// std::cout << "going to handlePOST" << std::endl;
		return "POST";
	}
	else if (method == "DELETE")
		return "DELETE";
	else
		throw HttpRequest::parsingException(501, "Method not implemented"); //testen
	return "";
}

void	handleRequest(int epollFd, connection *conn) {

	// std::cout << "in handleRequest:" << "\tfd = " << conn->fd << std::endl; //for testing
	const std::map<std::string, RequestHandler> methodHandlers = {
		{"GET", handleGET},
		{"POST", handlePOST},
		{"DELETE", handleDELETE},
		{"REDIRECT", handleRedirect},
		{"DIRLISTING", handleDIR},
		{"CGI", handleCGI},
	};
	try {
		HttpRequest request(conn->request, conn->server);
		if (request.getRequestStatus() != 200)
			throw HttpRequest::parsingException(request.getRequestStatus(), "Parsing error");
		auto it = methodHandlers.find(getHandler(conn, request));
		if (it != methodHandlers.end()) {
			it->second(epollFd, conn, request);
		}
		else
			throw HttpRequest::parsingException(405, "Check what is happening, should not get here");

	} catch (const HttpRequest::parsingException& exception) {
		std::cerr << RED << "Error: " + std::to_string(exception.getErrorCode()) << " " << exception.what() << RESET << std::endl;
		setErrorResponse(conn, exception.getErrorCode());
	}
	// std::cout << "end of handleRequest" << std::endl;
}

void	handleRedirect(int epollFd, connection *conn, HttpRequest& request) {
	(void)epollFd;
	std::string host			= request.uri.getHost();
	std::string location		= request.uri.getPath();
	std::map<int, std::string> redirect_location_header = conn->server->get_redirect(host, location);
	// //**testprint**
	// std::cout << BLUE << "\ntarget is redirected to: " << redirect_location_header.begin()->second << RESET << std::endl;
	request.headers->addHeader("location", redirect_location_header.begin()->second);
	HttpResponse response(request);
	response.setStatusCode(redirect_location_header.begin()->first);
	response.setResponse(conn);
}

void	handleDIR(int epollFd, connection *conn, HttpRequest& request) {

	//**testprint**
	// std::cout << BLUE << "\ntarget is a directory and its dirlisting = " << dirListing << RESET << std::endl;
	std::string cookieValue = checkAndSetCookie(conn, request);
	std::string host			= request.uri.getHost();
	std::string location		= request.uri.getPath();
	std::string root 			= conn->server->get_rootFolder(host, location);
	std::string index			= conn->server->get_index(host, location);
	bool dirListing				= conn->server->get_dirListing(host, location);
	std::string contentType		= request.uri.getMime(request.uri.getExtension());
	
	if (dirListing == true) {
		HttpResponse response(request);
		request.uri.setPath("/cgi-bin/index.py");
		request.uri.setQuery("folder="+root);
		handleCGI(epollFd, conn, request);
	}
	else if (!index.empty()) {
		// std::string bodyPath = root + "/text/html/" + index;
		std::string bodyPath = root + index;
		
		//**testprint**
		// std::cout << BLUE << "index found: " << index << " | setting body to :" << bodyPath <<  RESET << std::endl;
		
		HttpResponse response(request);
		response.reSetBody(bodyPath, false);
		response.headers->addHeader("Content-type", contentType);
		response.headers->setHeader("Set-Cookie", cookieValue);
		response.setResponse(conn);
	}
	else
		throw HttpRequest::parsingException(404, "Path not found");
}

void	handleCGI(int epollFd, connection *conn, HttpRequest& request) {
	std::string host			= request.uri.getHost();
	std::string location		= request.uri.getPath();
	//**testprint**
	// std::cout << "\nin CGI\n" << std::endl;

	// check content-length
	size_t	maxContentLength		= conn->server->get_maxBodySize(host, location);
	size_t	actualContentLength		= request.getBody().size();
	size_t	headerContentLength		= 0;
	if (request.headers->isHeader("content-length")) {
		headerContentLength = std::stoi(request.headers->getHeaderValue("content-length"));
		
		//**test print
		// request.headers->printHeaders();
		// std::cout << "header content len: " << headerContentLength << "\n";
		// std::cout << "actual content len: " << actualContentLength << "\n";
		// std::cout << "max content len: " 	<< maxContentLength << "\n";
	}
	if (headerContentLength > maxContentLength) {
		throw HttpRequest::parsingException(413, "Content Too Large");
	}
	if (headerContentLength != actualContentLength)
		throw HttpRequest::parsingException(400, "Bad request");
	
	// === set environ vars === 
	request.addEnvironVar("REQUEST_METHOD", request.getMethod());
	request.addEnvironVar("QUERY_STRING", request.getQueryString());
	request.addEnvironVar("REMOTE_HOST", host);
	request.addEnvironVar("PATH_INFO", conn->server->get_rootFolder(host, location));
	request.addEnvironVar("UPLOAD_DIR", conn->server->get_uploadDir(host, location));

		
	//call cgi handler
	std::string root = conn->server->get_rootFolder(host, location);
	if (request.uri.getPath().find("cgi-bin") == std::string::npos)
 		request.uri.setPath("/cgi-bin" + request.uri.getPath());

	if (cgiHandler(request, conn, epollFd) == 1 ) {
		setErrorResponse(conn, 500);	
		closeCGIpipe(epollFd, conn);
	}
	else {
		conn->state = IN_CGI;
	}
}

void	handleGET(int epollfd, connection *conn, HttpRequest& request) {
	
	//**testprint**
	// std::cout << "\nin GET handler\n" << std::endl;
	(void)epollfd;
	std::string	 cookieValue	= checkAndSetCookie(conn, request);
	std::string contentType		= request.uri.getMime(request.uri.getExtension());
	if (!contentType.empty()) {
		
		std::string fullPath;
		std::string host			= request.getHostname();
		std::string location		= request.uri.getPath();
		
		//check if target exists
		std::string pathToCheck = request.uri.getPath();
		if (!pathToCheck.empty() && pathToCheck[0] == '/')
        	pathToCheck.erase(0, 1);
		if (validPath(pathToCheck))
			fullPath = pathToCheck; // does not work for cookies

		else {
			//check cookie.png
			location = replaceCookiePng(location, cookieValue);
			// fullPath = conn->server->get_rootFolder(host, location) + "/" + contentType + location;
			fullPath = conn->server->get_rootFolder(host, location) + "/" + location;
		}
			
		std::ifstream f(fullPath);
		if (f.good())
			request.uri.setPath(fullPath);
		else
			throw HttpRequest::parsingException(404, "Path not found");

		HttpResponse response(request);
		response.reSetBody(request.uri.getPath(), request.uri.getIsBinary());
		response.headers->addHeader("Content-type", contentType);
		response.headers->setHeader("Set-Cookie", cookieValue);
		response.setResponse(conn);
	} 
	else
		throw HttpRequest::parsingException(404, "Page not found in request handler");
	// std::cout << "\nend of GET handler\n" << std::endl;
}

void	handlePOST(int epollfd, connection *conn, HttpRequest& request) {
	//**testprint**
	// std::cout << "\nin POST handler\n" << std::endl;
	
	(void) epollfd;
	(void) conn;
	(void) request;
	throw HttpRequest::parsingException(500, "configuration error");
	
	// HttpResponse response(request);
	// response.reSetBody("data/text/html/upload.html", false);
	// response.setResponse(conn);
}

void	handleDELETE(int epollfd, connection *conn, HttpRequest& request) {
	//**testprint**
	// std::cout << "\nin DELETE handler\n" << std::endl;
	
	(void) epollfd;
	(void) conn;
	(void) request;
	throw HttpRequest::parsingException(500, "configuration error");

	// HttpResponse response(request);
	// response.reSetBody("data/text/html/418.html", false);
	// response.setResponse(conn);
}