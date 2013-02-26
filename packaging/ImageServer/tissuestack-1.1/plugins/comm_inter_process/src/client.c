/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
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
