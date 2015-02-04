#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>

#include <stdint.h>
#include <sys/mman.h>
#include <fcntl.h>

// Broadcom 2835 timer address and offset
#define ST_BASE (0x20003000)
#define TIMER_OFFSET (4)

// Use GPIO Pin 24, which is Pin 5 for wiringPi library

#define BUTTON_PIN_FIRST 5

// Use GPIO Pin 17, which is Pin 0 for wiringPi library

#define BUTTON_PIN_SECOND 0

#define FASTER_RNG 1

// Timer frequency
#define HZ 1000000

// the event counters
volatile int64_t t0 = 0;
volatile int64_t t1 = 0;
volatile int64_t t2 = 0;
volatile int64_t t3 = 0;
volatile int64_t t4 = 0;

volatile int count = 0;

FILE *f;

int64_t *timer; // 64 bit timer

void WriteBit (int bit, FILE *f)
{
    static volatile int current_bit = 0;
    static volatile unsigned char bit_buffer = 0;
    unsigned char byte;

    int64_t t, dt;

    if (bit)
    bit_buffer |= (1<<(7-current_bit));

    if (++current_bit > 7) {
    t = *timer;
    dt = t-t0;
    t0 = t;
    byte = bit_buffer;
    fwrite (&byte, 1, 1, f);
    fflush(f);
    current_bit = 0;
    bit_buffer = 0;
#ifdef FASTER_RNG
    printf(" 0x%02X %1.3f bps %1.3f uSv/h\r\n", byte, 8.0/((float)dt/(float)HZ), 24*144/((float)dt/(float)HZ*420));
#else
    printf(" 0x%02X %1.3f bps %1.3f uSv/h\r\n", byte, 8.0/((float)dt/(float)HZ), 32*144/((float)dt/(float)HZ*420));
#endif
    }
}

// -------------------------------------------------------------------------
// myInterrupt:  called every time an event occurs
void myInterrupt(void) {
    int64_t dt;

#ifdef FASTER_RNG
    switch(count++) {
    case 0:
        t1 = *timer;
        break;
    case 1:
        t2 = *timer;
        break;
    case 2:
        t3 = *timer;
        count = 0;
        if((t3-t2) > (t2-t1)) {
            printf("1");
            WriteBit(1,f);
        } else if((t3-t2) < (t2-t1)) {
            printf("0");
            WriteBit(0,f);
        }

        fflush(stdout);
        break;
    default:
        count = 0;
    }
#else
    switch(count++) {
    case 0:
        t1 = *timer;
        break;
    case 1:
        t2 = *timer;
        break;
    case 2:
        t3 = *timer;
        break;
    case 3:
        t4 = *timer;
        count = 0;

        if((t4-t3) > (t2-t1)) {
            printf("1");
            WriteBit(1,f);
        } else if((t4-t3) < (t2-t1)) {
            printf("0");
            WriteBit(0,f);
        }

        fflush(stdout);
        break;
    default:
        count = 0;
    }
#endif
}


// -------------------------------------------------------------------------
// main
int main(void) {
    void *st_base; // ptr to simplify offset math
    int fd; // fopen descriptor

    // Trying to open core memory
    if (-1 == (fd = open("/dev/mem", O_RDONLY))) {
        fprintf(stderr, "open() failed.\n");
        return 255;
    }
 
    // map a specific page into our address space
    if (MAP_FAILED == (st_base = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, ST_BASE))) {
        fprintf(stderr, "MAP_FAILED.\n");
        return 254;
    }
 
    // open output file
    f = fopen("out.bin", "ab+");

    // sets up the wiringPi library
    if (wiringPiSetup () < 0) {
        fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
        return 1;
    }

    pwmSetClock(1);

    // set Pins 17/0 and 24/5 to generate an interrupt on low-to-high transitions
    // and attach myInterrupt() to the interrupt
    if ( wiringPiISR (BUTTON_PIN_FIRST, INT_EDGE_RISING, &myInterrupt) < 0  || wiringPiISR (BUTTON_PIN_SECOND, INT_EDGE_RISING, &myInterrupt) < 0 ) {
        fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
        return 1;
    }


    // set up pointer, based on mapped page
    timer = (int64_t *)((char *)st_base + TIMER_OFFSET);
 
    // initialize timer counters
    t0 = t1 = t2 = t3 = t4 = *timer;

    // main loop
    while ( 1 ) {
        delay( 1000 ); // wait 1 second
    }

    return 0;
}
