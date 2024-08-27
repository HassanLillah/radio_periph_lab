#include <stdio.h>
#include <sys/mman.h> 
#include <fcntl.h> 
#include <unistd.h>
#define _BSD_SOURCE

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

int main()
{

// first, get a pointer to the peripheral base address using /dev/mem and the function mmap
    volatile unsigned int *my_periph = get_a_pointer(RADIO_PERIPH_ADDRESS);
    volatile unsigned int *my_fifo = get_a_pointer(RADIO_FIFO_ADDRESS);
    printf("\r\n\r\n\r\nFinal Lab Mile Stone 2\n\r");
    printf("Setting up the Radio\n");
    *(my_periph) = 30001000;
    *(my_periph+1) = 30000000;
    *(my_periph+2) = 1;
    int count = 0;
    int reads_available;
    int dummy;
    while(count <480000) 
    {
        reads_available = *(my_fifo+1);
        for(int i=0; i<reads_available; i++) 
        {
            dummy = *(my_fifo);
        }
        count = count + reads_available;
    }

    printf("Finished reading 480000 samples.\n\r");
    return 0;
}
