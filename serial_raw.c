#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#ifdef __unix__
# include <termios.h>
#endif

#include "serial_raw.h"

/* 
   Set serial to be "raw" mode, i.e., totally binary. It returns error
   code in case of failure, or 0 for success. Failure codes are:

   1    could not get serial atributes
   2    could not set serial atributes
*/
int serial_set_raw(int fd) {
#ifdef __unix__
  struct termios term;

  /* get current settings */
  if (tcgetattr(fd, & term) != 0)
    return 1;

  cfmakeraw(&term);               

  term.c_iflag &= ~ ( BRKINT | ICRNL | IGNCR | INLCR | IGNPAR | INPCK |
                      PARMRK | ISTRIP | IXOFF | IXON | IMAXBEL);
  term.c_iflag |= IGNBRK;
  
  /* output mode flags */
  term.c_oflag &= ~ ( OPOST | ONLCR | OCRNL | ONOCR | ONLRET | OFILL |
                      OFDEL | NLDLY | CRDLY | TABDLY | BSDLY | VTDLY |
                      FFDLY);
  
  /* local mode flags */
  term.c_lflag &= ~ ( ECHO | ECHOE | ECHOK | ECHONL | ICANON | IEXTEN |
                      ISIG | TOSTOP );
  term.c_lflag |= NOFLSH;

  /* control mode flags */
  term.c_cflag &= ~ ( HUPCL | CSTOPB | PARENB | CSIZE);
  term.c_cflag |= CLOCAL | CREAD | CS8;

  /* set terminal attributes */
  if (tcsetattr(fd, TCSANOW, &term) != 0)
    return 2;

#endif  
  return 0;
}

int serial_set_no_echo(int fd) {
#ifdef __unix__
  struct termios term;

  /* get current settings */
  if (tcgetattr(fd, & term) != 0)
    return 1;
  
  /* local mode flags */
  term.c_lflag &= ~ECHO;
  
  /* set terminal attributes */
  if (tcsetattr(fd, TCSANOW, &term) != 0)
    return 2;
  
#endif  
  return 0;
}

int serial_set_not_line_buffered(int fd) {
#ifdef __unix__
  struct termios term;

  /* get current settings */
  if (tcgetattr(fd, & term) != 0)
    return 1;

  /* local mode flags */
  term.c_lflag &= ~ICANON;

  /* set terminal attributes */
  if (tcsetattr(fd, TCSANOW, &term) != 0)
    return 2;

#endif  
  return 0;
}

/* 
   Set serial speed (baudrate). You will need to use B9600, B19200,
   etc. constants for baudrate argument.

   Error codes:
   0    success
   1    could not get serial atributes
   2    could not set serial atributes
   3    could not set input speed
   4    could not set output speed
*/
int serial_set_baudrate(int fd, int baudrate) {
#ifdef __unix__
  struct termios term;

  /* get current settings */
  if (tcgetattr(fd, & term) != 0)
    return 1;
  
  /* set speed */
  if (cfsetispeed (&term, baudrate) != 0)
    return 3;
  if (cfsetospeed (&term, baudrate) != 0)
    return 4;

  /* set terminal attributes */
  if (tcsetattr(fd, TCSANOW, &term) != 0)
    return 2;
#endif
  return 0;
}

/*
  Set VMIN and VTIME attributes of serial.

   Error codes:
   0    success
   1    could not get serial atributes
   2    could not set serial atributes
*/
int serial_set_min_time(int fd, int vmin, int vtime) {
#ifdef __unix__
  struct termios term;

  /* get current settings */
  if (tcgetattr(fd, & term) != 0)
    return 1;

  /* control characters */
  //term.c_cc[VINTR] = 0;
  term.c_cc[VMIN] = vmin;
  //term.c_cc[VQUIT] = 0;
  //term.c_cc[VSUSP] = 0;
  term.c_cc[VTIME] = vtime;
  //term.c_cc[VSTART] = 0;
  //term.c_cc[VSTOP] = 0;
  
  /* set terminal attributes */
  if (tcsetattr(fd, TCSANOW, &term) != 0)
    return 2;
#endif  
  return 0;
}

#ifdef SERIAL_TEST
int main() {
  int st;
  int a = 0, i = 0, hd;
  
  st = open("/dev/ttyS1", O_RDONLY);
  assert(st >= 0);

  assert(serial_set_raw(st) == 0);
  assert(serial_set_speed(st, B9600) == 0);

  close(st);
  return 0;
}
#endif
