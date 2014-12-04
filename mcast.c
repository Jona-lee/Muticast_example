#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int log_debug;

#define MSGBUFSIZE 256

static void usage(void)
{
	printf("Usage: mcast [option] [argv]\n");
	printf("    -v : verbose\n");
	printf("    -h : help\n");
	printf("    -c : client say hello to group\n");
	printf("    -s : listener recveive from group\n");
	printf("    -g [group]: ip group for client and server\n");
	printf("    -p [port] : ip port for client and server\n");
}

static int add_multigroup(int fd, char *group)
{
	struct ip_mreq mreq;
	
	memset((void*)&mreq, 0, sizeof(mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(group);
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	if (setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
		&mreq, sizeof(mreq)) < 0) {
		perror("setsockopt:IP_ADD_MEMBERSHIP");
		return -1;
	}
	return 0;
}

static int start_client(char *group, int port)
{
	int fd, ret;

	struct sockaddr_in addr;
	char *message = "Hello, World!";

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(group);
	addr.sin_port = htons(port);

	while (1) {
		if (sendto(fd, message, strlen(message), 0, 
			(struct sockaddr *) &addr, sizeof(addr)) < 0) {
			perror("sendto");
			ret = -1;
			break;
		}
		sleep(1);
	}
	close(fd);

	return ret;
}

static int start_server(char *group, int port)
{
	int fd;
	struct sockaddr_in addr;
	int nbytes, addrlen;
	char msgbuf[MSGBUFSIZE];
	u_int reuseflag = 1;
	int ret = 0;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseflag, sizeof(reuseflag));
	if (ret < 0) {
		perror("SO_REUSEADDR");
		close(fd);
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(fd);
		return -1;
	}

	ret = add_multigroup(fd, group);
	if (ret) {
		printf("join a multicast group %s failed!\n", group);
		close(fd);
		return ret;
	}

	while (1) {
		addrlen = sizeof(addr);
		nbytes = recvfrom(fd, msgbuf, MSGBUFSIZE, 0,
							(struct sockaddr *)&addr, &addrlen);
		if (nbytes < 0) {
			perror("recvfrom");
			ret = -1;
			break;
		}
		msgbuf[nbytes] = '\0';
		printf("recv: %s\n", msgbuf);
	}
	close(fd);

	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int c;
	int mode = 0;
	int port = -1;
	char *group = NULL;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"help",  no_argument,          0, 'h' },
			{"verbose", no_argument,        0, 'v' },
			{"client",  no_argument,        0, 'c' },
			{"server",  no_argument,        0, 's' },
			{"group",   required_argument,  0, 'g' },
			{"port",    required_argument,  0, 'p' },
			{    0,             0,          0,  0  }
		};

		c = getopt_long(argc, argv, "hvcsg:p:",
			long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'v':
			log_debug = 1;
			break;
		case 'c':
			mode = 0;
			break;
		case 's':
			mode = 1;
			break;
		case 'g':
			group = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'h':
			usage();
			return 0;
		default:
			break;
		}
	}

	if (port < 0 || !group) {
		usage();
		return -1;
	}

	switch (mode) {
	case 0:
		printf("multicast test:client..\n");
		ret = start_client(group, port);
		break;
	case 1:
		printf("multicast test:server..\n");
		ret = start_server(group, port);
		break;
	default:
		break;
	}

	return ret;
}
