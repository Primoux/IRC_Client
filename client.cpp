#include <ncurses.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <cstring>
#include <netdb.h>
#include <algorithm>

static WINDOW *g_msg;
static WINDOW *g_bar;
static WINDOW *g_inp;

static void msg_push(const std::string &s)
{
	waddstr(g_msg, s.c_str());
	wrefresh(g_msg);
}

static void bar_draw()
{
	werase(g_bar);
	wattron(g_bar, A_REVERSE);
	wprintw(g_bar, " [1]PRIVMSG  [2]JOIN  [3]PART  [4]NICK  [5]QUIT  [6]Raw ");
	wattroff(g_bar, A_REVERSE);
	wrefresh(g_bar);
}

static void inp_draw(const std::string &label, const std::string &buf)
{
	werase(g_inp);
	mvwprintw(g_inp, 0, 0, "%s", label.c_str());
	mvwprintw(g_inp, 1, 0, "> %s", buf.c_str());
	wmove(g_inp, 1, 2 + (int)buf.size());
	wrefresh(g_inp);
}

static void send_cmd(int fd, const std::string &cmd)
{
	send(fd, cmd.c_str(), cmd.size(), 0);
}

int main(int argc, char *argv[])
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
		return 1;
	}

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) { perror("socket"); return 1; }

	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int st = getaddrinfo(argv[1], argv[2], &hints, &res);
	if (st != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(st));
		return 1;
	}
	if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("connect"); freeaddrinfo(res); return 1;
	}
	freeaddrinfo(res);

	initscr();
	noecho();
	cbreak();

	int rows, cols;
	getmaxyx(stdscr, rows, cols);

	g_msg = newwin(rows - 3, cols, 0, 0);
	scrollok(g_msg, TRUE);
	idlok(g_msg, TRUE);
	wrefresh(g_msg);

	g_bar = newwin(1, cols, rows - 3, 0);
	g_inp = newwin(2, cols, rows - 2, 0);
	keypad(g_inp, TRUE);

	bar_draw();

	send_cmd(fd, "PASS pass\r\n");
	send_cmd(fd, "NICK enchevrax\r\n");
	send_cmd(fd, "USER enchevrax 0 * :RealName\r\n");

	const std::vector<std::string> plabels[] = {
		{},
		{"Target", "Message"},
		{"Channel"},
		{"Channel"},
		{"New nick"},
		{"Quit message"},
		{"Commande brute"},
	};

	enum { MENU, PARAM } mode = MENU;
	int choice = 0;
	std::vector<std::string> params;
	std::string buf;

	char net[512];
	int nfds = std::max(fd, STDIN_FILENO) + 1;

	while (true) {
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		FD_SET(STDIN_FILENO, &rfds);

		if (select(nfds, &rfds, NULL, NULL, NULL) < 0)
			break;

		if (FD_ISSET(fd, &rfds)) {
			int n = recv(fd, net, sizeof(net) - 1, 0);
			if (n <= 0) break;
			net[n] = '\0';
			msg_push(net);
		}

		if (FD_ISSET(STDIN_FILENO, &rfds)) {
			int ch = wgetch(g_inp);
			if (ch != ERR) {
				if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
					if (!buf.empty()) buf.pop_back();
				} else if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
					if (mode == MENU) {
						if (buf.size() == 1 && buf[0] >= '1' && buf[0] <= '6') {
							choice = buf[0] - '0';
							mode = PARAM;
							params.clear();
						}
					} else {
						params.push_back(buf);
						if (params.size() == plabels[choice].size()) {
							std::string cmd;
							switch (choice) {
								case 1: cmd = "PRIVMSG " + params[0] + " :" + params[1]; break;
								case 2: cmd = "JOIN " + params[0]; break;
								case 3: cmd = "PART " + params[0]; break;
								case 4: cmd = "NICK " + params[0]; break;
								case 5: cmd = "QUIT :" + params[0]; break;
								case 6: cmd = params[0]; break;
							}
							send_cmd(fd, cmd + "\r\n");
							mode = MENU;
							choice = 0;
							params.clear();
						}
					}
					buf.clear();
				} else if (ch >= 32 && ch < 127) {
					buf += (char)ch;
				}
			}
		}

		std::string label = (mode == MENU)
			? "Choix (1-6) :"
			: plabels[choice][params.size()] + " :";
		inp_draw(label, buf);
	}

	endwin();
	close(fd);
	return 0;
}
