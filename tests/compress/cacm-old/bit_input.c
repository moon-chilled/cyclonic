/* BIT INPUT ROUTINES. */

#include <stdio.h>


/* THE BIT BUFFER. */
static int bits_to_go;
static int buffer;

/* INITIALIZE BIT INPUT. */

void start_inputing_bits()
{   bits_to_go = 0;				/* Buffer starts out with   */
}						/* no bits in it.           */


/* INPUT A BIT. */

int input_bit()
{   
  int t;
  bits_to_go -= 1;
  if (bits_to_go<0) {				/* Read the next byte if no */
    buffer = getc(stdin);			/* bits are left in the     */
    bits_to_go = 7;				/* buffer. Return anything  */
  }			   			/* after end-of-file.       */
  t = buffer&1;
  buffer >>= 1;				/* Return the next bit from */
  return t;					/* the bottom of the byte.  */
}





