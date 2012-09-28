#include "client.h"

// IMPORTANT: use this variable to avoid seg faults that are caused by logging when called from JNI
// when 1 => we know we have come from JNI and then we don't use the LOG macros
short i_am_jni = 0;

int		init_sock_comm_client(char *path)
{
  struct sockaddr_un address;
  int		socket_fd;

  socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(socket_fd < 0)
    {
	if (!i_am_jni) ERROR("socket() failed");
      return 0;
    }
  memset(&address, 0, sizeof(struct sockaddr_un));
  address.sun_family = AF_UNIX;
  snprintf(address.sun_path, 108, "%s", path);
  if(connect(socket_fd, (struct sockaddr *) &address,
	     sizeof(struct sockaddr_un)) != 0)
    {
		if (!i_am_jni)  ERROR("connect() failed");
      return 0;
    }
  return (socket_fd);
}
