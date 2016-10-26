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
#include <string.h>

#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>

#include <net/ethernet.h>	//L2 protocols
//#include <net/if_arp.h>
//#include <netinet/if_ether.h>



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
	}
}

void fnSendData(int socketfd) {
	//target address
	struct	sockaddr_ll socket_addr;
	//byffer for send frame
	void *pvSndBuf = (void *)malloc(ETH_FRAME_LEN);

	//pointer to the ethernet header
	unsigned char *etherhead = pvSndBuf;

	//pointer to the ethernet data
	unsigned char *data = pvSndBuf + 14;

	//another pointer to ethernet header
	struct ethhdr *eh = (struct ethhdr *)etherhead;

	int	Send_result;

	//our mac address
	unsigned char src_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	//other host mac addr
	unsigned char dst_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	socket_addr.sll_family = PF_PACKET;
	socket_addr.sll_protocol = htons(ETH_P_ALL);
	//interface index
	socket_addr.sll_ifindex = 0;
	//ARP HARDWARE IDENTIFIER IS ETHERNET
	socket_addr.sll_hatype = ARPHRD_ETHER;

	socket_addr.sll_pkttype = PACKET_OTHERHOST;
	//address length
	socket_addr.sll_halen = ETH_ALEN;

	//MAC BEGIN
	socket_addr.sll_addr[0] = 0x00;
	socket_addr.sll_addr[1] = 0x00;
	socket_addr.sll_addr[2] = 0x00;
	socket_addr.sll_addr[3] = 0x00;
	socket_addr.sll_addr[4] = 0x00;
	socket_addr.sll_addr[5] = 0x00;
	//MAC END
	socket_addr.sll_addr[0] = 0x00;		//not used
	socket_addr.sll_addr[0] = 0x00;		//not used

	//Set the frame header
	memcpy((void *)pvSndBuf, (void *)dst_mac, ETH_ALEN);
	memcpy((void *)pvSndBuf + ETH_ALEN, (void *)src_mac, ETH_ALEN);
	eh->h_proto = 0;
	//fill frame with some data
	data[0] = 'a';

	Send_result = sendto(/*socket*/0, pvSndBuf, ETH_FRAME_LEN, 0,
			(struct sockaddr *)&socket_addr, sizeof(socket_addr));

	if (Send_result == -1) {
		puts("error sendto");
	}
}
