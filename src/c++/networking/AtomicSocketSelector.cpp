#include "networking.h"




/*
	fd_set master;
	// temp file descriptor list for select()
	fd_set read_fds;
	// client address
	struct sockaddr_in clientaddr;
	// maximum file descriptor number
	int fdmax;
	// listening socket descriptor
	int listener;
	// newly accept()ed socket descriptor
	int newfd;
	// buffer for client data
	char buf[1024];
	int nbytes;
	// for setsockopt() SO_REUSEADDR, below
	int addrlen;
	int i, j;
	// clear the master and temp sets
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	// listen
	if (listen(listener, 10) == -1) {
		perror("Server-listen() error lol!");
		exit(1);
	}
	printf("Server-listen() is OK...\n");

	// add the listener to the master set
	FD_SET(listener, &master);
	// keep track of the biggest file descriptor
	fdmax = listener; // so far, it's this one

	// loop
	for (;;) {
		// copy it
		read_fds = master;

		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("Server-select() error lol!");
			exit(1);
		}
		printf("Server-select() is OK...\n");

		//run through the existing connections looking for data to be read
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // we got one...
				if (i == listener) {
					// handle new connections
					addrlen = sizeof(clientaddr);
					if ((newfd = accept(listener,
							(struct sockaddr *) &clientaddr, &addrlen)) == -1) {
						perror("Server-accept() error lol!");
					} else {
						printf("Server-accept() is OK...\n");

						FD_SET(newfd, &master); // add to master set
						if (newfd > fdmax) { // keep track of the maximum
							fdmax = newfd;
						}
						printf("%s: New connection from %s on socket %d\n",
								argv[0], inet_ntoa(clientaddr.sin_addr), newfd);
					}
				} else {
					// handle data from a client
					if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
						// got error or connection closed by client
						if (nbytes == 0)
							// connection closed
							printf("%s: socket %d hung up\n", argv[0], i);

						else
							perror("recv() error lol!");

						// close it...
						close(i);
						// remove from master set
						FD_CLR(i, &master);
					} else {
						// we got some data from a client
						for (j = 0; j <= fdmax; j++) {
							// send to everyone!
							if (FD_ISSET(j, &master)) {
								// except the listener and ourselves
								if (j != listener && j != i) {
									if (send(j, buf, nbytes, 0) == -1)
										perror("send() error lol!");
								}
							}
						}
					}
				}
			}
		}
	}
*/
