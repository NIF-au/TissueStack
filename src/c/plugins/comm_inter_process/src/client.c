#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>

int		init_sock_comm_client(char *path)
{
  struct sockaddr_un address;
  int		socket_fd;
  int		nbytes;

  socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(socket_fd < 0)
    {
      printf("socket() failed\n");
      return 1;
    }
  memset(&address, 0, sizeof(struct sockaddr_un));
  address.sun_family = AF_UNIX;
  snprintf(address.sun_path, 108, path);
  if(connect(socket_fd, (struct sockaddr *) &address,
	     sizeof(struct sockaddr_un)) != 0)
    {
      printf("connect() failed\n");
      return 1;
    }
  return (socket_fd);
}

int main(int argc, char **argv) {
	int fd = init_sock_comm_client("/tmp/tissue_stack_communication");
	char * test = "load minc_info /usr/local/plugins/TissueStackMincInfo.so";
	write(fd, test, strlen(test));
	sleep(2);
	test = "start minc_info";
	write(fd, test, strlen(test));
	char * buffer = malloc(1024 * sizeof(buffer));
	size_t size = 0;
	while ((size = read(fd, buffer, 1024))) {
		printf("%s\n", buffer);
		printf("%i\n", (int) size);
		if (size == 0) {
			break;
		}
	}
	close(fd);
}
