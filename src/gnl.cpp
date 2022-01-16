# include <unistd.h>
# include <stdlib.h>
# define BUFFER_SIZE 65536


int	check_init_errors(int fd, char **line, char **buf)
{
	if (fd < 0 || read(fd, &buf[0], 0) || !line)
		return (1);
	*buf = (char *)malloc(BUFFER_SIZE);
	if (*buf == NULL)
		return (1);
	return (0);
}

void	check_read_errors(int res, char **line, char *buf)
{
	if (res == -1)
		free(buf);
	else
		*line = buf;
}

int	read_cycle(int fd, char *buf)
{
	int	i;
	int	res;

	i = 0;
	while (1)
	{
		if (i == BUFFER_SIZE)
			return (-1);
		res = read(fd, &buf[i], 1);
		if (res <= 0 || buf[i] == '\n')
			break ;
		i++;
	}
	buf[i] = '\0';
	return (res);
}

int	get_next_line(int fd, char **line)
{
	char	*buf;
	int		res;

	buf = 0;
	if (check_init_errors(fd, line, &buf))
		return (-1);
	res = read_cycle(fd, buf);
	check_read_errors(res, line, buf);
	return (res);
}