#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

struct s_client
{
	int 	id;
	int		fd;
	char	*buf;
};

fd_set	master = {};
fd_set	rd = {};
fd_set	wr = {};
int		id = 0;

void ft_exit(char *err, int len)
{
	write(2, err, len);
	exit(1);
}

void	broadcast(char *message, int len, int fd_client, struct s_client clients[], int max)
{
	for (int i = 0; i < max; i++)
	{
		if (clients[i].fd == fd_client || clients[i].fd == 0 || !FD_ISSET(clients[i].fd, &wr))
			continue;
		send(clients[i].fd, message, len, 0);
	}
}

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
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

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

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

int main(int ac, char *av[])
{
	struct s_client		clients[10000];
	struct sockaddr_in	serv_addr;
	struct sockaddr_in	clt_addr;
	socklen_t 			len_add = sizeof(clt_addr);
	int buf_size = 1024;
	int socket_id;
	int max;
	int fd;
	int len;
	char msg[buf_size];
	char *line;

	if (ac != 2)
		ft_exit("Wrong number of arguments\n", 26);

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(2130706433);
	serv_addr.sin_port = htons(atoi(av[1]));

	socket_id = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_id == -1)
		ft_exit("Fatal error\n", 12);
	max = socket_id;
	FD_ZERO(&master);
	FD_SET(socket_id, &master);

	if (bind(socket_id, ((const struct sockaddr *)&serv_addr), sizeof(serv_addr)) == -1)
		ft_exit("Fatal error\n", 12);
	if (listen(socket_id, 128) == -1)
		ft_exit("Fatal error\n", 12);
	signal(SIGPIPE, SIG_IGN);
	while(1)
	{
		rd = master;
		wr = master;
		if (select(FD_SETSIZE, &rd, &wr, NULL, NULL) != -1)
		{
			for (int i = 3; i <= max; i++)
			{
				bzero(msg, buf_size);
				if (FD_ISSET(i, &rd))
				{
					if (i == socket_id)
					{
						fd = accept(socket_id, (struct sockaddr *)&clt_addr, &len_add);
						if (fd < 0)
							continue;
							//* Here i create a new client
						printf("New client connected\n");
						FD_SET(fd, &master);
						clients[fd - (socket_id + 1)].id = id++;
						clients[fd - (socket_id + 1)].fd = fd;
						clients[fd - (socket_id + 1)].buf = NULL;
						max = fd > max ? fd : max;
						 // * Here i notify connected clients that he is connected!
						len = sprintf(msg,"server: client %d just arrived\n", clients[fd - (socket_id + 1)].id);
						broadcast(msg, len, fd, clients, max - socket_id);
					}
					else
					{
						while ((len = recv(i, msg, buf_size, 0)) > 0)
						{
							clients[i - (socket_id + 1)].buf = str_join(clients[i - (socket_id +1)].buf, msg);
							if (len < buf_size)
								break;
							bzero(msg, buf_size);
						}
						if (len == 0)
						{
							close(i);
							FD_CLR(i, &master);
							clients[i - (socket_id + 1)].fd = 0;
							free(clients[i - (socket_id + 1)].buf);
							clients[i - (socket_id + 1)].buf = NULL;
							len = sprintf(msg, "server: client %d just left\n", clients[i - (socket_id + 1)].id);
							broadcast(msg,len, i, clients, max);
						}
						else
						{
							while (extract_message(&clients[i - (socket_id + 1)].buf, &line) > 0)
							{
								len = sprintf(msg, "client %d: %s", clients[i - (socket_id + 1)].id, line);
								broadcast(msg, len, i, clients, max);
								free(line);
							}
						}
					}
				}
			}
		}
	}
}