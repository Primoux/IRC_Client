#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <netdb.h>


void send_cmd(int fd, const std::string& cmd)
{
	send(fd, cmd.c_str(), cmd.size(), 0);
}

int main(int argc, char* argv[])
{
	(void)argc;
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return 1;
	}

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
	int	status = getaddrinfo(argv[1], argv[2], &hints, &res);
	if (status != 0) {
		std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
		return 1;
	}

	if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("connect");
		return 1;
	}

	std::cout << "Connected!" << std::endl;

	send_cmd(fd, "PASS pass\r\n");
	send_cmd(fd, "NICK enchevrax\r\n");
	send_cmd(fd, "USER enchevrax 0 * :RealName\r\n");
	send_cmd(fd, "JOIN #lol password\r\n");
	send_cmd(fd, "JOIN #lol password\r\n");
	send_cmd(fd, "PART #lol\r\n");
	char buf[512];
	while (true)
	{
		int n = recv(fd, buf, sizeof(buf) - 1, 0);
		if (n <= 0) break;
		buf[n] = '\0';
		std::cout << buf;
	}
	close(fd);
	return 0;
}
