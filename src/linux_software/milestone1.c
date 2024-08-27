#include <stdio.h>
#include <sys/mman.h> 
#include <fcntl.h> 
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <errno.h>
#include <stdlib.h>
#define _BSD_SOURCE
#define PORT	 25344

// the below code uses a device called /dev/mem to get a pointer to a physical
// address.  We will use this pointer to read/write the custom peripheral
volatile unsigned int * get_a_pointer(unsigned int phys_addr)
{

	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); 
	void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr); 
	volatile unsigned int *radio_base = (volatile unsigned int *)map_base; 
	return (radio_base);
}

int main(int argc, char **argv)
{

// first, get a pointer to the peripheral base address using /dev/mem and the function mmap
    int sockfd;
    char *hostname = argv[1];
    char *msg="Hello World";
    struct sockaddr_in	 servaddr;
    int16_t data[513];

    for (int i=1; i<513; i++)
    {
        data[i] = 5;
    }

    printf("\r\n\r\n\r\nFinal Lab Milestone 1\n\r");

    printf("The host address is %s\n", hostname);
    printf("Datagram is = %s\n", msg);

	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

    memset(&servaddr, 0, sizeof(servaddr));
		
	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr(hostname);

	int n, len;

    data[0] = 1;	
	sendto(sockfd, (const char *)data, sizeof(data),
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
			sizeof(servaddr));

    close(sockfd);
    return 0;
}
