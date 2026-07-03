#include <cstdio>
#include <pthread.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <netdb.h>
#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <vector>
#include <sys/wait.h>

static void send_cmd(int fd, const std::string &cmd)
{
	send(fd, cmd.c_str(), cmd.size(), 0);
}

int is_alphabetic(const char *str)
{
	while (*str) {
		if (*str < 'A' || (*str > 'Z' && *str < 'a') || *str > 'z')
			return 0;
		str++;
	}
	return 1;
}

bool random_bool(float probability)
{
	
	float randomValue = static_cast<float>(rand() / (float)RAND_MAX);
	std::cout << "Random value: " << randomValue << ", Probability: " << probability << std::endl;
	bool outcome = randomValue < probability;

	std::cout << "Outcome: " << outcome << std::endl;
	return (outcome);
}

std::string random_string()
{
	int urandom = open("/dev/urandom", O_RDONLY);
	char randomstr[9];
	read(urandom, randomstr, 4);

	while (!is_alphabetic(randomstr) || strlen(randomstr) < 4)
		read(urandom, randomstr, 4);

	randomstr[8] = '\0';
	close(urandom);
	return std::string(randomstr);
}

int main(int argc, char *argv[])
{
  /* initialize random seed: */
  srand (time(NULL));

	if (argc < 3) {
		printf("Usage: %s <host> <port>\n", argv[0]);
		return 1;
	}

int pid = getpid();
	for (int i = 0; i < 5; ++i)
		if (pid != 0)
		{
			usleep(rand() % 1000000);
			pid = fork();
		}

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) { perror("socket"); return 1; }

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int st = getaddrinfo(argv[1], argv[2], &hints, &res);
	if (st != 0) {
		printf("getaddrinfo: %s\n", gai_strerror(st));
		return 1;
	}
	if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("connect"); freeaddrinfo(res); return 1;
	}
	freeaddrinfo(res);



	std::vector<std::string> channels ;


	/* IDENTITY */
	
	
	if (pid == 0)
	{
		std::string randomnick = random_string();
		std::cout << "hi my name is " << randomnick << std::endl;
		std::stringstream ss;
		ss << randomnick;

		send_cmd(fd, "PASS pass\r\n");
		send_cmd(fd, "NICK " + ss.str() + "\r\n");
		usleep(100000);
		send_cmd(fd, "USER " + ss.str() + " 0 * :RealName\r\n");
		usleep(500000);
		send_cmd(fd, "JOIN #rallypoint\r\n");
		usleep(100000);
		send_cmd(fd, "PRIVMSG #rallypoint :hello from " + ss.str() + "\r\n");
		usleep(100000);

		while (random_bool(0.999))
		{
			if (random_bool(0.25))
			{
				std::string randomChannel = random_string();
				std::stringstream channelStream;
				channelStream << "#" << randomChannel;
				send_cmd(fd, "JOIN " + channelStream.str() + "\r\n");
				usleep(100000);
			}

			if (!channels.empty() && random_bool(0.25))
			{
				int randomIndex = rand() % channels.size();
				send_cmd(fd, "PART " + channels[randomIndex] + "\r\n");
				channels.erase(channels.begin() + randomIndex);
				usleep(100000);
			}
			if (!channels.empty() && random_bool(0.6))
			{
				int randomIndex = rand() % channels.size();
				send_cmd(fd, "PRIVMSG " + channels[randomIndex] + " :hello from " + ss.str() + "\r\n");
				usleep(100000);
			}
			if (!channels.empty() && random_bool(0.2))
			{
				int randomIndex = rand() % channels.size();
				send_cmd(fd, "TOPIC " + channels[randomIndex] + " :New topic from " + ss.str() + "\r\n");
				usleep(100000);
			}
			else
			{
				send_cmd(fd, "PRIVMSG #rallypoint :hello from " + ss.str() + "\r\n");
				usleep(100000);
			}
		}
		send_cmd(fd, "QUIT :leaving\r\n");
		usleep(100000);
		return 0;
	}
	std::cout << "Process " << getpid() << " exiting." << std::endl;
	while (wait(NULL) > 0)
		std::cout << "child exited" << std::endl;
	return (0);


	
	int nfds = std::max(fd, STDIN_FILENO) + 1;

	while (true) {
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		FD_SET(STDIN_FILENO, &rfds);

		if (select(nfds, &rfds, NULL, NULL, NULL) < 0)
			break;
		}

	close(fd);
	return 0;
}
