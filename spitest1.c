//
//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013
 
 
// Access from ARM Running Linux
 
#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#define AUX_BASE   0x20215000

#define AUXENB_SPI1 (1<<1)
#define AUXENB_UART (1<<0)


#define AUX_IRQ     0
#define AUX_ENABLES 1
#define AUX_SPI0_CNTL0_REG 32
#define AUX_SPI0_CNTL1_REG 33
#define AUX_SPI0_STAT_REG  34
#define AUX_SPI0_PEEK_REG  35

#define AUX_SPI0_IO_REG    40
#define AUX_SPI0_TX_HOLD   44

#define AUXSPI_CNTL0_SPEED(x)      ((x)<<20)
#define AUXSPI_CNTL0_CS(x)         ((x)<<17)
#define AUXSPI_CNTL0_POSTINP         (1<<16)
#define AUXSPI_CNTL0_VAR_CS          (1<<15)
#define AUXSPI_CNTL0_VAR_WIDTH       (1<<14)
#define AUXSPI_CNTL0_DOUT_HOLD(x)  ((x)<<12)
#define AUXSPI_CNTL0_ENABLE          (1<<11)
#define AUXSPI_CNTL0_IN_RISING(x)  ((x)<<10)
#define AUXSPI_CNTL0_CLR_FIFOS       (1<<9)
#define AUXSPI_CNTL0_OUT_RISING(x) ((x)<<8)
#define AUXSPI_CNTL0_INVERT_CLK(x) ((x)<<7)
#define AUXSPI_CNTL0_MSB_FIRST       (1<<6)
#define AUXSPI_CNTL0_SHIFT_LEN(x)  ((x)<<0)

#define AUXSPI_CNTL1_CS_HIGH(x) ((x)<<8)
#define AUXSPI_CNTL1_TX_IRQ       (1<<7)
#define AUXSPI_CNTL1_DONE_IRQ     (1<<6)
#define AUXSPI_CNTL1_MSB_FIRST    (1<<1)
#define AUXSPI_CNTL1_KEEP_INPUP   (1<<0)

#define AUXSPI_STAT_TX_FIFO(x) ((x)<<28)
#define AUXSPI_STAT_RX_FIFO(x) ((x)<<20)
#define AUXSPI_STAT_TX_FULL      (1<<10)
#define AUXSPI_STAT_TX_EMPTY     (1<<9)
#define AUXSPI_STAT_RX_EMPTY     (1<<7)
#define AUXSPI_STAT_BUSY         (1<<6)
#define AUXSPI_STAT_BITS(x)    ((x)<<0)

 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
 
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
 
int  mem_fd;
void *gpio_map;
void *spi_map;
 
// I/O access
volatile unsigned *gpio;
volatile unsigned *aux;

static void gpioSetMode(unsigned g, unsigned mode)
{
   int reg, shift;

   reg   =  g/10;
   shift = (g%10) * 3;

   gpio[reg] = (gpio[reg] & ~(7<<shift)) | (mode<<shift);
}

 
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define SET_GPIO_ALT(g,a) gpioSetMode(g,a)
#define INP_GPIO(g) gpioSetMode(g,0)
#define OUT_GPIO(g) gpioSetMode(g,1)
 
#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
 
#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH
 
#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock


/*!   \brief bcm2835PortFunction
  Port function select modes for bcm2835_gpio_fsel()
*/
typedef enum
{
    BCM2835_GPIO_FSEL_INPT  = 0x00,   /*!< Input 0b000 */
    BCM2835_GPIO_FSEL_OUTP  = 0x01,   /*!< Output 0b001 */
    BCM2835_GPIO_FSEL_ALT0  = 0x04,   /*!< Alternate function 0 0b100 */
    BCM2835_GPIO_FSEL_ALT1  = 0x05,   /*!< Alternate function 1 0b101 */
    BCM2835_GPIO_FSEL_ALT2  = 0x06,   /*!< Alternate function 2 0b110, */
    BCM2835_GPIO_FSEL_ALT3  = 0x07,   /*!< Alternate function 3 0b111 */
    BCM2835_GPIO_FSEL_ALT4  = 0x03,   /*!< Alternate function 4 0b011 */
    BCM2835_GPIO_FSEL_ALT5  = 0x02,   /*!< Alternate function 5 0b010 */
    BCM2835_GPIO_FSEL_MASK  = 0x07    /*!< Function select bits mask 0b111 */
} bcm2835FunctionSelect;
 
void setup_io();
 
void printButton(int g)
{
  if (GET_GPIO(g)) // !=0 <-> bit is 1 <- port is HIGH=3.3V
    printf("Button pressed!\n");
  else // port is LOW=0V
    printf("Button released!\n");
}
enum {
  PIN_SWDI=19,
  PIN_SWDO=20,
  PIN_RESET=26,
  PIN_SWDCLK=21,
};

static void short_wait(void);
static void short_wait(void)	
{
//	int i;
//	for (i=0; i<150; i++) {
//		asm volatile("nop");
//	}
	fflush(stdout); //
	usleep(1); // suggested as alternative for asm which c99 does not accept
}

void swd_reset(int n)
{
  int rep;
  OUT_GPIO(PIN_SWDO);
  GPIO_SET =  (1 << PIN_SWDO);
  for (rep=0; rep<n; rep++)
  {
       GPIO_SET =  (1 << PIN_SWDCLK);
       GPIO_CLR =  (1 << PIN_SWDCLK);
  }
}
void swd_out(int out)
{
   int rep;
  for (rep=0; rep<8; rep++)
  {
       GPIO_SET =  (1 << PIN_SWDCLK);
       GPIO_SET =  (1 << PIN_SWDCLK);
       GPIO_CLR =  (1 << PIN_SWDCLK);
	if(out & 1 ==1)
        {
          GPIO_SET =  (1 << PIN_SWDO);
        }else{
          GPIO_CLR =  (1 << PIN_SWDO);
        }
        out=out >>1;
  }
   // see the reply
}

void cycle(int v)
{
  if(v==0) {
          GPIO_CLR =  (1 << PIN_SWDO);
  }
else {
          GPIO_SET =  (1 << PIN_SWDO);
 }
       GPIO_CLR =  (1 << PIN_SWDCLK);
       GPIO_CLR =  (1 << PIN_SWDCLK);
       GPIO_SET =  (1 << PIN_SWDCLK);
       GPIO_SET =  (1 << PIN_SWDCLK);
}
  int speed=5;
void fast_swd_out(int val)
{


  int spiDefaults = AUXSPI_CNTL0_SPEED(speed)   |
                 AUXSPI_CNTL0_CS(0)      |
                 AUXSPI_CNTL0_IN_RISING(1)  |
                 AUXSPI_CNTL0_DOUT_HOLD(0)  |
                 AUXSPI_CNTL0_OUT_RISING(1) |
                 AUXSPI_CNTL0_INVERT_CLK(1) |
                 AUXSPI_CNTL0_SHIFT_LEN(8);

   aux[AUX_SPI0_CNTL0_REG] = AUXSPI_CNTL0_CLR_FIFOS | AUXSPI_CNTL0_ENABLE | spiDefaults;

   aux[AUX_SPI0_CNTL0_REG] = AUXSPI_CNTL0_ENABLE | spiDefaults;
  SET_GPIO_ALT(PIN_SWDO, BCM2835_GPIO_FSEL_ALT4);
  SET_GPIO_ALT(PIN_SWDCLK, BCM2835_GPIO_FSEL_ALT4);
   aux[AUX_SPI0_CNTL1_REG] = 0; // AUXSPI_CNTL1_MSB_FIRST;
  aux[AUX_SPI0_TX_HOLD] = val;
   while ((aux[AUX_SPI0_STAT_REG] & AUXSPI_STAT_BUSY)) ;
    INP_GPIO(PIN_SWDO); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(PIN_SWDO);
    INP_GPIO(PIN_SWDCLK); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(PIN_SWDCLK);

}
int fast_swd_in()
{
  int i;
  int spiDefaults = AUXSPI_CNTL0_SPEED(speed)   |
                 AUXSPI_CNTL0_CS(0)      |
                 AUXSPI_CNTL0_IN_RISING(1)  |
                 AUXSPI_CNTL0_DOUT_HOLD(0)  |
                 AUXSPI_CNTL0_OUT_RISING(0) |
                 AUXSPI_CNTL0_INVERT_CLK(1) |
                 AUXSPI_CNTL0_SHIFT_LEN(16);

    aux[AUX_SPI0_CNTL0_REG] = AUXSPI_CNTL0_CLR_FIFOS | AUXSPI_CNTL0_ENABLE | spiDefaults;
    aux[AUX_SPI0_CNTL0_REG] = AUXSPI_CNTL0_ENABLE | spiDefaults;
  INP_GPIO(PIN_SWDO); // must use INP_GPIO before we can use OUT_GPIO
    INP_GPIO(PIN_SWDI); // must use INP_GPIO before we can use OUT_GPIO
  SET_GPIO_ALT(PIN_SWDI, BCM2835_GPIO_FSEL_ALT4);
  SET_GPIO_ALT(PIN_SWDCLK, BCM2835_GPIO_FSEL_ALT4);
   aux[AUX_SPI0_CNTL1_REG] = 0; // AUXSPI_CNTL1_MSB_FIRST;
   int v[4];
   for(i=0;i<2;i++)
   {
        aux[AUX_SPI0_TX_HOLD] = 0;
        while ((aux[AUX_SPI0_STAT_REG] & AUXSPI_STAT_BUSY)) ;
	v[i]= aux[AUX_SPI0_TX_HOLD];
   }
   printf("Got values %x %x %x %x\n", v[0],v[1],v[2],v[3]);
    //INP_GPIO(PIN_SWDO); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(PIN_SWDO);
    //INP_GPIO(PIN_SWDCLK); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(PIN_SWDCLK);

}
void trigger()
{
    OUT_GPIO(16);
    GPIO_SET = (1<<16) ;
    GPIO_CLR = (1<<16) ;
}
 
int main(int argc, char **argv)
{
  int g,rep;
 
  // Set up gpi pointer for direct register access
  setup_io();


// enable pull-up on GPIO24&25
GPIO_PULL = 2;
short_wait();
// clock on GPIO 24 & 25 (bit 24 & 25 set)
GPIO_PULLCLK0 = 1 << PIN_SWDI;
short_wait();
GPIO_PULL = 0;
GPIO_PULLCLK0 = 0;
 
 
aux[AUX_ENABLES] |= AUXENB_SPI1;
    INP_GPIO(PIN_SWDO); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(PIN_SWDO);
    INP_GPIO(PIN_SWDCLK); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(PIN_SWDCLK);
    INP_GPIO(PIN_RESET); // must use INP_GPIO before we can use OUT_GPIO
    OUT_GPIO(PIN_RESET);
    INP_GPIO(PIN_SWDI); // must use INP_GPIO before we can use OUT_GPIO


    GPIO_SET = (1<<PIN_SWDO) ;
    GPIO_CLR =  (1 << PIN_RESET);
short_wait();
    GPIO_SET =  (1 << PIN_RESET);
short_wait();
short_wait();
short_wait();
short_wait();
short_wait();
short_wait();

    swd_reset(50);
    swd_out(0x9E);
    swd_out(0xE7);
    swd_reset(51);
    cycle(0);
    cycle(0);
    fast_swd_out(0xA5);

    INP_GPIO(PIN_SWDO); 
  int ack=0;
trigger();
  for (rep=0; rep<4; rep++)
  {
       GPIO_SET =  (1 << PIN_SWDCLK);
       GPIO_SET =  (1 << PIN_SWDCLK);
       GPIO_CLR =  (1 << PIN_SWDCLK);
	   ack=ack<<1;
	if(GET_GPIO( PIN_SWDO)!=0)
	   ack=ack|1 ;

  }
  fast_swd_in();
  printf("Ack %x\n",ack);





  return 0;
 
} // main
 
 
//
// Set up a memory regions to access GPIO
//
void setup_io()
{
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }
 
   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );
 
 
   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }
 
   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;


   /* mmap SPI */
   spi_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      AUX_BASE         //Offset to GPIO peripheral
   );
 
   close(mem_fd); //No need to keep mem_fd open after mmap
 
   if (spi_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)spi_map);//errno also set!
      exit(-1);
   }
 
   aux = (volatile unsigned *)spi_map;
 
 
} // setup_io
