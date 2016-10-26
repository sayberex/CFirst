/*
 ============================================================================
 Name        : CFirst.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>

#include <net/ethernet.h>	//L2 protocols

//http://forum.vingrad.ru/forum/topic-372076/kw-packet-raw-socket-sendto-recvfrom.html
//http://ru.manpages.org/
//https://habrahabr.ru/post/227729/
//http://linuxdoc.ru/packet.html

//6byte DST_MAC + 6byte SRC_MAC + 2btesETH TYPE + 1500byte USER DATA + 4byte FSC = 1518(Maximal ethernet frame length)
//#define	MAX_FRAME_LEN	1518
#define	MAX_FRAME_LEN	1518

void *pvRcvBuf;
int   iRcvBufLen;

/*
 * Important notice
 * Warning: only root can create raw sockets otherwise
 * socket return -1 always
 */

int fnCreateSocket(int *socketfd);
void fnReceiveData(int socketfd);

int socketfd = 0;		//Socket file descriptor

int main(void) {

	pvRcvBuf = (void *)malloc(MAX_FRAME_LEN);

	if (fnCreateSocket(&socketfd)) {
		do {
			fnReceiveData(socketfd);
		} while (1);
	}

	//int ch = getchar();
	//putchar(ch);

	puts("Terminated..");
	return EXIT_SUCCESS;
}

int fnCreateSocket(int *socketfd) {
	int hSock;
	/*
	 * ru.manpages.org/socket/2
	 * PF_PACKET - Protocol family (PF_INET PF_INET6 )
	 * SOCK_RAW  - Socket Type
	 * ETH_P_ALL - Protocol ID
	 *
	 * PF_PACKET - low level packet
	 * SOCK_RAW	 - Socket Type RAW(direct access to network interface)
	 * ETH_P_ALL - receive all ethernet frames
	 */

	hSock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (hSock == -1) {
		puts("Raw socket not created\n");
		return 0;
	}
	else {
		printf("Raw socket created hSocket = %d\n", hSock);
		*socketfd = hSock;
		return 1;
	}
}


void fnReceiveData(int socketfd) {
	iRcvBufLen = 0;
	iRcvBufLen = recvfrom(socketfd, pvRcvBuf, ETH_FRAME_LEN, 0, NULL, NULL);
	if (iRcvBufLen != -1) {
		printf("Length = %d\n", iRcvBufLen);
		//printf("Length = %d\n", iRcvBufLen);
	}
}
