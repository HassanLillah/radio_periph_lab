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

#define RADIO_TUNER_FAKE_ADC_PINC_OFFSET 0
#define RADIO_TUNER_TUNER_PINC_OFFSET 1
#define RADIO_TUNER_CONTROL_REG_OFFSET 2
#define RADIO_TUNER_TIMER_REG_OFFSET 3
#define RADIO_PERIPH_ADDRESS 0x43c00000
#define RADIO_FIFO_ADDRESS 0x43c10000

// the below code uses a device called /dev/mem to get a pointer to a physical
// address.  We will use this pointer to read/write the custom peripheral
volatile unsigned int * get_a_pointer(unsigned int phys_addr)
{

	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); 
	void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr); 
	volatile unsigned int *radio_base = (volatile unsigned int *)map_base; 
	return (radio_base);
}

void print_message()
{
    printf("Executible not called correctly. Please provide 4 arguments along with the executable.\n\r");
    printf("The order should be: IP Address, Fake ADC Frequency, Tune Frequency, and Enable/Disable (1/0) UDP Datagrams.\n\r");
    printf("Example: ./final_lab 192.157.1.131 30001000 30000000 1\n\r");
}

int main(int argc, char **argv)
{

    printf("\r\n\r\n\r\nFinal Lab: Linux SDR with Ethernet by Hassan Nisar \n\r");
    
    if(argc!=5){
        print_message();
        return EXIT_FAILURE;
    }

    int sockfd;
    int16_t data_frame[513];
    int16_t frame_num = 0;
    int index = 0;
    int data_word;
    int reads_available;
    char *hostname = argv[1];
    struct sockaddr_in	 servaddr;
    int input_freq = (int) strtol(argv[2], (char **)NULL, 10);
    int tune_freq = (int) strtol(argv[3], (char **)NULL, 10);
    int udp_enable = (int) strtol(argv[4], (char **)NULL, 10);
    volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);
    volatile unsigned int *my_fifo = get_a_pointer(RADIO_FIFO_ADDRESS);

    printf("The host address is %s\n", hostname);
    printf("Input Frequency is %d\n", input_freq);
    printf("Tune Frequency is %d\n", tune_freq);
    printf("UDP Enable is %d\n", udp_enable);

    printf("Setting up Radio\n");

    *(my_periph) = input_freq;
    *(my_periph+1) = tune_freq;
    *(my_periph+2) = 1;

    while(udp_enable) {
        // Creating Datagram to be sent using UDP
        data_frame[0] = frame_num;
        frame_num++;
        index = 1;
        while(index<513)
        {
            reads_available = *(my_fifo+1);
            int remaining = (513 - index)/2;
            int imag_index;
            int real_index;
            if (remaining >= reads_available){
                for(int i=0; i<reads_available; i++) {
                    data_word = *(my_fifo);
                    imag_index = index+2*i;
                    real_index = index+2*i+1;
                    data_word = *(my_fifo);
                    data_frame[imag_index] = (int16_t)(data_word>>16);
                    data_frame[real_index] = (int16_t)(data_word & 0xFFFF);
                }
                index = index + reads_available*2;
            } else {
                for(int i=0; i<remaining; i++) {
                    data_word = *(my_fifo);
                    imag_index = index+2*i;
                    real_index = index+2*i+1;
                    data_word = *(my_fifo);
                    data_frame[imag_index] = (int16_t)(data_word>>16);
                    data_frame[real_index] = (int16_t)(data_word & 0xFFFF);
                }
                index = index + remaining*2;
            }
        }
        

        // Setting up connection and sending UDP Datagram
        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
            perror("socket creation failed");
            exit(EXIT_FAILURE);
        }

        memset(&servaddr, 0, sizeof(servaddr));
            
        // Filling server information
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT);
        servaddr.sin_addr.s_addr = inet_addr(hostname);

	    sendto(sockfd, (const char *)data_frame, sizeof(data_frame),
		MSG_CONFIRM, (const struct sockaddr *) &servaddr,
		sizeof(servaddr));

        close(sockfd);

    }

    return 0;
}
