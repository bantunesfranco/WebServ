#include "webServ.hpp"

char	*getProgramPath(const Uri& uri, char *program)
{
	std::string	path = uri.getPath();
	program[0] = '.';
	for (int i = 0; i < static_cast<int>(path.size()); i++)
		program[i + 1] = path[i];
	program[path.size() + 1] = '\0';
	return (program);
}

void	execChild(const HttpRequest& req, CGI &cgi, int oldFd)
{
	char	program[req.uri.getPath().size() + 2];
	char	*argv[] = {program, NULL};
	char	**env = req.getEnvArray();
	getProgramPath(req.uri, program);

	// std::cout << "cgi fd = " << cgi.getFdIn() << std::endl;
	if (dup2(cgi.getFdOut(), STDOUT_FILENO) == -1 || dup2(oldFd, STDIN_FILENO) == -1)
	{
		cgi.closeFds();
		std::exit(1);
	}
	// close(cgi.getFdIn());
	cgi.closeFds();
	close(oldFd);
	execve(argv[0], argv, env);
	std::exit(1);
}

int	writeBody(const HttpRequest& req, int *fd)
{
	
	std::string	body = req.getBody() + "\0";
	while (1)
	{
		size_t	size = body.length() > BUFFER_SIZE ? BUFFER_SIZE : body.length();
		if (write(fd[1], body.c_str(), size) == -1)
		{
			std::cerr << strerror(errno) << std::endl;
			close(fd[0]);
			close(fd[1]);
			return (-1);
		}
		if (body.length() > BUFFER_SIZE)
			body = body.substr(size, body.npos);
		else
			break;
	}
	close(fd[1]);
	return (fd[0]);
}

int cgiHandler(const HttpRequest& req, connection *conn, int epollFd)
{
	int			fd[2];

	if (pipe(fd) == -1)
		return (-1);
	
	CGI	cgi(epollFd, conn);
	if (cgi.getStatus() == 1)
	{
		cgi.closeFds();
		return (1);
	}

	int	pid = fork();
	if (pid == -1)
	{
		cgi.closeFds();
		return (1);
	}
	else if (pid == 0) {
		execChild(req, cgi, fd[0]);
	}
	else
	{
		conn->cgiPID = pid;
		close(cgi.getFdOut());
		if (writeBody(req, fd) == -1)
		{
			close(fd[0]);
			return (1);
		}
		close(fd[0]);
	}
	return (0);
}