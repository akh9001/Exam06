#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <sys/_types/_fd_def.h>
#include <sys/_types/_socklen_t.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

struct client
{
	int id;
	int fd;
	char *buf;
};

struct client clients[100000];
int id;
int	max, socket_fd;
fd_set	master;
fd_set	rd = {};
fd_set	wr = {};

int extract_message(char **buf, char **msg) {
  char *newbuf;
  int i;

  *msg = 0;
  if (*buf == 0)
    return (0);
  i = 0;
  while ((*buf)[i]) {
    if ((*buf)[i] == '\n') {
      newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
      if (newbuf == 0)
        return (-1);
      strcpy(newbuf, *buf + i + 1);
      *msg = *buf;
      (*msg)[i + 1] = 0;
      *buf = newbuf;
      return (1);
    }
    i++;
  }
  return (0);
}

char *str_join(char *buf, char *add) {
  char *newbuf;
  int len;

  if (buf == 0)
    len = 0;
  else
    len = strlen(buf);
  newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
  if (newbuf == 0)
    return (0);
  newbuf[0] = 0;
  if (buf != 0)
    strcat(newbuf, buf);
  free(buf);
  strcat(newbuf, add);
  return (newbuf);
}

void	ft_exit(char *error)
{
	write(2, error, strlen(error));
	exit(1);
}

void	broadcast(char *message, int sender)
{
	for (int i = 3; i <= max; i++)
	{
		if (i == sender || i == socket_fd || !FD_ISSET(i, &wr))
			continue;
		send(i, message, strlen(message), 0);
	}
}

int	main(int ac, char **av)
{
	int buf_size = 100000;
	int len, index, clt_fd;
	socklen_t add_len;
	char *line = NULL;
	char message[buf_size + 1];
	struct sockaddr_in serv_addr, clt_addr;

	if (ac != 2)
		ft_exit("Wrong number of arguments\n");
	add_len = sizeof(serv_addr);
	bzero(&serv_addr, add_len);
	bzero(&clt_addr, add_len);
	// assign IP, PORT
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(2130706433); // 127.0.0.1
	serv_addr.sin_port = htons(atoi(av[1]));
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		ft_exit("Fatal error\n");
	if (bind(socket_fd, (const struct sockaddr *)&serv_addr, (socklen_t)sizeof(serv_addr)) == -1)
		ft_exit("Fatal error\n");
	if (listen(socket_fd, 128) == -1)
		ft_exit("Fatal error\n");
	max = socket_fd;
	FD_SET(socket_fd, &master);
	while (1)
	{
		rd = master;
		wr = master;
		if (select(FD_SETSIZE, &rd, &wr, NULL, NULL) == -1)
			continue;
		for (int i = 3; i <= max; i++)
		{
			bzero(message, buf_size + 1);
			index = i - (socket_fd + 1);
			if (FD_ISSET(i, &rd))
			{
				if (i == socket_fd)
				{
					if ((clt_fd = accept(socket_fd, (struct sockaddr *)&clt_addr, &add_len)) == -1)
						continue;
					index = clt_fd - (socket_fd + 1);
					FD_SET(clt_fd, &master);
					clients[index].id = id++;
					clients[index].fd = clt_fd;
					clients[index].buf = NULL;
					max = clt_fd > max ? clt_fd : max;
					sprintf(message, "server: client %d just arrived\n", clients[index].id);
					broadcast(message, clt_fd);
				}
				else {
					while((len = recv(i, message, buf_size, 0)) > 0)
					{
						clients[index].buf = str_join(clients[index].buf, message);
						if (clients[index].buf == 0)
							ft_exit("Fatal error\n");
						if (len < buf_size)
							break;
						bzero(message, buf_size + 1);
					}
					if (len == 0)
					{
						close(i);
						free(clients[index].buf);
						clients[index].buf = NULL;
						clients[index].fd = 0;
						FD_CLR(i, &master);
						FD_CLR(i, &rd);
						FD_CLR(i, &wr);
						sprintf(message, "server: client %d just left\n", clients[index].id);
						broadcast(message, i);
						continue;
					}
					while ((len = extract_message(&clients[index].buf, &line)))
					{
						sprintf(message, "client %d: %s", clients[index].id, line);
						broadcast(message, i);
						free(line);
					}
					if (len == -1)
						ft_exit("Fatal error\n");
				}
			}
		}
	}
}