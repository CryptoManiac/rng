#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <sys/times.h>

// Use GPIO Pin 17, which is Pin 0 for wiringPi library

#define BUTTON_PIN 0

extern unsigned long volatile jiffies;

// the event counters
volatile clock_t t1 = 0;
volatile clock_t t2 = 0;
volatile clock_t t3 = 0;
volatile clock_t t4 = 0;

volatile int count = 0;

struct tms tms;

// -------------------------------------------------------------------------
// myInterrupt:  called every time an event occurs
void myInterrupt(void) {
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
	    } else {
		printf("0");
	    }
	    fflush(stdout);
	    break;
	default:
	    count = 0;
    }
}


// -------------------------------------------------------------------------
// main
int main(void) {
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

  // initialize timer counter
  t1 = times(&tms);

  // display counter value every second.
  while ( 1 ) {
    delay( 1000 ); // wait 1 second
  }

  return 0;
}
