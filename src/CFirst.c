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
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>

#include <net/ethernet.h>	//L2 protocols
//#include <net/if.h>
//#include <net/if_arp.h>
//#include <netinet/if_ether.h>



//http://forum.vingrad.ru/forum/topic-372076/kw-packet-raw-socket-sendto-recvfrom.html
//http://ru.manpages.org/
//https://habrahabr.ru/post/227729/
//http://linuxdoc.ru/packet.html

//6byte DST_MAC + 6byte SRC_MAC + 2btesETH TYPE + 1500byte USER DATA + 4byte FSC = 1518(Maximal ethernet frame length)
//#define	MAX_FRAME_LEN	1518
#define	MAX_FRAME_LEN	1518

char	iface_name[] = "eth0";
int		iface_index  = 0;

void *pvRcvBuf;
int   iRcvBufLen;

/*
 * Important notice
 * Warning: only root can create raw sockets otherwise
 * socket return -1 always
 */

int fnCreateSocket(int *socketfd);
void fnReceiveData(int socketfd);
void fnShowIfaces(void);

int socketfd = 0;		//Socket file descriptor

int main(void) {
	//fnShowIfaces();

	//resolve net iface name to index
	if ((iface_index = if_nametoindex(iface_name)) != 0) {
		printf("iface_index = %d\n", iface_index);

	} else {
		printf("Iface %s not found", iface_name);
	}

	/*pvRcvBuf = (void *)malloc(MAX_FRAME_LEN);

	if (fnCreateSocket(&socketfd)) {
		do {
			fnReceiveData(socketfd);
		} while (1);
	}*/

	//int ch = getchar();
	//putchar(ch);

	puts("Terminated..");
	return EXIT_SUCCESS;
}

/*int fnGetIfaceIndex(char *ifname, int *iface_index) {
	return if_nametoindex(ifname);
}*/

int fnGetMac(char mac[6]) {
	struct ifreq s;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

	strcpy(s.ifr_ifrn.ifrn_name,iface_name);
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
		int	i;
		for (i = 0; i < 6; ++i) mac[i] = s.ifr_ifru.ifru_addr.sa_data[i];
		close(fd);
		return 1;
	}
	close(fd);
	return 0;
}

void fnShowIfaces(void) {
	int ifIndex;
	char ifName[30];

	for (ifIndex = 0; ifIndex < 100; ifIndex++) {
		if_indextoname(ifIndex, ifName);
		if (ifName != NULL) {
			puts(ifName);
		}
	}
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
	struct sockaddr_ll socket_addr = {
		PF_PACKET,			/*sll_family*/					//Protocol family PF_PACKET - Device level Packet Socket(AF_INET(IPv4), AF_INET6(IPv6))
		htons(ETH_P_ALL),	/*sll_protocol*/				//low level protocol ID (like CAN IrDA and so on)
		0,					/*sll_ifindex*/					//communication interface index(eth0)
		ARPHRD_ETHER,		/*sll_hatype*/					//ARP Protocol Hardware ID(Ethernet 10Mbps)
		PACKET_OTHERHOST,	/*sll_pkttype*/					//PACKET Type(to all broadcast/to group multicast/to user space /to kernel space/ and so on)
		ETH_ALEN,			/*sll_halen*/					//Ethernet address length(MAC - length)
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}	//Physical layer MAC
	};

	socket_addr.sll_ifindex = iface_index;					//network interface index

	void 		  *pvSndBuf 	= (void *)malloc(ETH_FRAME_LEN);//Send Packet Buffer
	unsigned char *etherhead	= pvSndBuf;					//Ethernet header pointer
	unsigned char *data 		= pvSndBuf + 14;			//Ethernet packet data pointer
	struct ethhdr *eh 			= (struct ethhdr *)etherhead;//Structure pointer to Ethernet header



	//our mac address
	unsigned char src_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	//other host mac addr
	unsigned char dst_mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


	int	Send_result;

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
