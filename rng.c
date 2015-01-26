#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <sys/times.h>

// Use GPIO Pin 17, which is Pin 0 for wiringPi library

#define BUTTON_PIN 0

//#define TRUE_RNG 1
//#define FASTER_RNG 1
#define FAST_RNG 1
//#define FASTEST_RNG 1

#define HZ 100

// the event counters
volatile clock_t t0 = 0;
volatile clock_t t1 = 0;
volatile clock_t t2 = 0;
volatile clock_t t3 = 0;
volatile clock_t t4 = 0;

volatile int count = 0;

FILE *f;

struct tms tms;

void WriteBit (int bit, FILE *f)
{
    static volatile int current_bit = 0;
    static volatile unsigned char bit_buffer = 0;
    unsigned char byte;
    clock_t t, dt;

    if (bit)
	bit_buffer |= (1<<(7-current_bit));

    if (++current_bit > 7) {
	t = times(&tms);
	dt = t-t0;
	t0 = t;
	byte = bit_buffer;
	fwrite (&byte, 1, 1, f);
	fflush(f);
	current_bit = 0;
	bit_buffer = 0;
#ifdef FAST_RNG
	printf(" 0x%02X %1.3f bps %1.3f uSv/h\r\n", byte, 8.0/((float)dt/(float)HZ), 16*144/((float)dt/(float)HZ*420));
#endif
#ifdef FASTEST_RNG
	printf(" 0x%02X %1.3f bps %1.3f uSv/h\r\n", byte, 8.0/((float)dt/(float)HZ), 8*144/((float)dt/(float)HZ*420));
#endif
#ifdef FASTER_RNG
	printf(" 0x%02X %1.3f bps %1.3f uSv/h\r\n", byte, 8.0/((float)dt/(float)HZ), 24*144/((float)dt/(float)HZ*420));
#endif
#ifdef TRUE_RNG
	printf(" 0x%02X %1.3f bps %1.3f uSv/h\r\n", byte, 8.0/((float)dt/(float)HZ), 32*144/((float)dt/(float)HZ*420));
#endif
    }
}

// -------------------------------------------------------------------------
// myInterrupt:  called every time an event occurs
void myInterrupt(void) {
	clock_t dt;

#ifdef FASTEST_RNG
	dt = t2-t1;
	t1 = t2;
	t2 = times(&tms);
	if(dt > (t2-t1)) {
	    printf("1");
	    WriteBit(1,f);
	} else if(dt < (t2-t1)) {
	    printf("0");
	    WriteBit(0,f);
	}
	fflush(stdout);
#endif

#ifdef FAST_RNG
	count = ++count & 0x01;

	if(!count) {
	    t2 = times(&tms);
	} else {
	    t1 = t3;
	    t3 = times(&tms);
	    if((t2-t1) > (t3-t2)) {
		printf("1");
		WriteBit(1,f);
	    } else if((t2-t1) < (t3-t2)) {
		printf("0");
		WriteBit(0,f);
	    }
	    fflush(stdout);
	}
#endif

#ifdef FASTER_RNG
    switch(count++) {
	case 0:
	    t1 = times(&tms);
	    break;
	case 1:
	    t2 = times(&tms);
	    break;
	case 2:
	    t3 = times(&tms);
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
#endif

#ifdef TRUE_RNG
    switch(count++) {
	case 0:
	    t1 = times(&tms);
	    break;
	case 1:
	    t2 = times(&tms);
	    break;
	case 2:
	    t3 = times(&tms);
	    break;
	case 3:
	    t4 = times(&tms);
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

    // open output file
    f = fopen("out.bin", "ab+");

  // sets up the wiringPi library
  if (wiringPiSetup () < 0) {
      fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
      return 1;
  }

  // set Pin 17/0 generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (BUTTON_PIN, INT_EDGE_FALLING, &myInterrupt) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }

  // initialize timer counters
  t0 = t1 = t2 = t3 = t4 = times(&tms);

    // main loop
    while ( 1 ) {
	delay( 1000 ); // wait 1 second
    }

  return 0;
}
