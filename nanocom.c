/* $Id: nanocom.c,v 1.10 2004/10/18 11:57:21 pzn Exp $ */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>

#include "serial_raw.h"
#include "screen.h"

struct termios stdin_attr;
volatile int main_loop = 1;
volatile int sig_number = -1;

/* TODO: add parser for this options */
char opt_eol_output[2] = { 0x0d, 0x0a };
int opt_eol_output_length = 2;
int opt_eol_input_show = 1; /* 1: will show CR and LF codes */
int opt_eol_input_linefeed = 0; /* -1 will disable linefeeds */

char * cvs_version (void) {
  static char * s = NULL;
  if (s == NULL) {
    s = strdup("$Revision: 1.10 $");
    s[strlen(s)-2] = 0;
  }
  return s+11;
}

void stdin_saveattr (void) {
  assert (tcgetattr(STDIN_FILENO, & stdin_attr) == 0);
}

void stdin_restoreattr (void) {
  assert (tcsetattr(STDIN_FILENO, TCSANOW, & stdin_attr) == 0);
}

void got_signal (int signum) {
  main_loop = 0;
  sig_number = signum;
}

int main (int argc, char * argv[]) {
  int fd = 0, i, opt_noecho = 0, opt_rxonly = 0, usage_help = 0;
  int opt_cfgonly = 0, opt_rxhex = 0, opt_rxhext = 0;
  char s[8], se[8];

  if (argc == 4) {
    usage_help = 1;
    if (strcmp("noecho",argv[3]) == 0) {
      opt_noecho = 1;
      usage_help = 0;
    }
    if (strcmp("rxonly",argv[3]) == 0) {
      opt_rxonly = 1;
      usage_help = 0;
    }
    if (strcmp("rxhex",argv[3]) == 0) {
      opt_rxonly = 1;
      opt_rxhex = 1;
      usage_help = 0;
    }
    if (strcmp("rxhext",argv[3]) == 0) {
      opt_rxonly = 1;
      opt_rxhex = 0;
      opt_rxhext = 1;
      usage_help = 0;
    }
    if (strcmp("cfgonly",argv[3]) == 0) {
      opt_cfgonly = 1;
      opt_rxonly = 1; /* does not change STDIN parameters */
      usage_help = 0;
    }
  }

  if ((argc < 3) || (argc > 4) || (usage_help == 1)) {
    fprintf(stderr,
	    "This is nanocom version %s\n"
	    "(c) 2004 - Pedro Zorzenon Neto\n\n"
	    "  Usage: %s serial-device baudrate [noecho | rxonly | cfgonly | rxhex | rxhext ]\n"
	    "  Usage example: %s ttyS1 57600\n"
	    "  Usage example: %s ttyS1 57600 rxhex\n"
	    "  Try baudrate 0 if you do not want to change baudrate\n"
	    "  Options: noecho:  what you type on keyboard will not be seen\n"
	    "                    on screen (but will be send to serial-device).\n"
	    "           rxonly:  only receives from serial-device. does not\n"
	    "                    send anything.\n"
	    "           cfgonly: configure the serial port and exit.\n"
	    "           rxhex:   only receives from serial-device. does not\n"
	    "                    send anything. uses 'hexdump -C' format\n"
	    "           rxhext:  only receives from serial-device. show the\n"
	    "                    timestamp and the received byte in hex\n",
	    cvs_version(), argv[0], argv[0], argv[0]);
    return 1;
  }

  if (opt_rxonly) {
    if (access(argv[1],R_OK) == 0) {
      fd = open(argv[1],O_RDONLY);
    } else {
      char * s;
      if (asprintf(&s, "/dev/%s", argv[1]) == -1) { }
      if (access(s,R_OK) == 0) {
	fd = open(s,O_RDONLY);
	free(s);
      } else {
	fprintf(stderr,"NANOCOM: devices '%s' and '%s' not found, or no "
		"permission to open them in read_only mode\n",argv[1],s);
	return 2;
      }
    }
  } else {
    if (access(argv[1],R_OK|W_OK) == 0) {
      fd = open(argv[1],O_RDWR);
    } else {
      char * s;
      if (asprintf(&s, "/dev/%s", argv[1]) == -1) { }
      if (access(s,R_OK|W_OK) == 0) {
	fd = open(s,O_RDWR);
	free(s);
      } else {
	fprintf(stderr,"NANOCOM: devices '%s' and '%s' not found, or no "
		"permission to open them in read+write mode\n",argv[1],s);
	return 2;
      }
    }
  }
  assert(fd >= 0);
  assert(serial_set_raw(fd) == 0);

  if (! opt_rxonly) {
    stdin_saveattr();
  }
  signal(SIGINT, got_signal);
  signal(SIGQUIT, got_signal);
  signal(SIGTERM, got_signal);
  if (! opt_rxonly) {
    assert(serial_set_not_line_buffered(STDIN_FILENO) == 0);
    if (opt_noecho == 1) {
      assert(serial_set_no_echo(STDIN_FILENO) == 0);
    }
  }

  {
    int bps = -1;
    int opt;
    opt = strtol(argv[2], (char **)NULL, 10);
    switch(opt) {
    case 0:
      break;
    case 300:
      bps = B300;
      break;
    case 1200:
      bps = B1200;
      break;
    case 2400:
      bps = B2400;
      break;
    case 4800:
      bps = B4800;
      break;
    case 9600:
      bps = B9600;
      break;
    case 19200:
      bps = B19200;
      break;
    case 38400:
      bps = B38400;
      break;
    case 57600:
      bps = B57600;
      break;
    case 115200:
      bps = B115200;
      break;
    default:
      fprintf(stderr,"NANOCOM: could not understand baudrate value\n");
      return 1;
    }
    if (bps != -1) {
      fprintf(stderr,"NANOCOM: changing baudrate to %d\n",opt);
      assert(serial_set_baudrate(fd,bps) == 0);
      tcflush(fd,TCIOFLUSH);
    } else {
      fprintf(stderr,"NANOCOM: baudrate not changed\n");
    }
  }

  fprintf(stderr,"NANOCOM: ready\n");
  if (opt_cfgonly == 1) {
    fprintf(stderr,"NANOCOM: cfgonly mode, will exit now\n");
    main_loop = 0;
  }
  if (opt_rxhex) {
    screen_init();
  }
  while (main_loop == 1) {
    fd_set rfds;
    struct timeval tv;
    int retval;
    
    /* Watch stdin + fd to see when it has input. */
    FD_ZERO(&rfds);
    if (! opt_rxonly) {
      FD_SET(STDIN_FILENO, &rfds);
    }
    FD_SET(fd, &rfds);
    
    /* Wait up to five seconds. */
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    retval = select(fd+1, &rfds, NULL, NULL, &tv);
    
    if (retval == -1) {
      if (errno != EINTR) {
	perror("NANOCOM: select()");
	return 2;
      }
    }
    if (retval > 0) {
      if (! opt_rxonly) {
	if (FD_ISSET(STDIN_FILENO, &rfds)) {
	  /* data available in STDIN */
	  i = read(STDIN_FILENO, s, 1);
	  if (i == 1) {
	    if (s[0] == 0x0a) {
	      /* user pressed ENTER key */
	      int i;
	      for (i = 0; i < opt_eol_output_length; i++) {
		if (write(fd,s+i,1) != 1) { }
	      }
	    } else {
	      if (write(fd,s,1) != 1) { }
	    }
	    fdatasync(fd);
	  }
	}
      }
      if (FD_ISSET(fd, &rfds)) {
	/* data available in fd */
	i = read(fd, s, 1);
	if (i == 1) {
	  static unsigned int last_char = 256;
	  unsigned int this_char;
	  this_char = s[0];
	  if (opt_rxhex) {
	    screen_post(this_char);
	  } else if (opt_rxhext) {
	    static struct timeval tv1;
	    static int i = 0;
	    struct timeval tv2;
	    struct timezone tz;
	    double t;
	    gettimeofday(&tv2,&tz);
	    if (!i) { /* first byte, time from previous is 0 */
	      i = 1;
	      tv1.tv_sec = tv2.tv_sec;
	      tv1.tv_usec = tv2.tv_usec;
	    }
	    t = tv2.tv_sec + (tv2.tv_usec * 0.000001);
	    t -= tv1.tv_sec + (tv1.tv_usec * 0.000001);
	    printf("%.6f %02x\n", t, this_char & 0xff);
	    tv1.tv_sec = tv2.tv_sec;
	    tv1.tv_usec = tv2.tv_usec;
	  } else {
	    if ((s[0] == 0x0d) || (s[0] == 0x0a)) {
	      if (opt_eol_input_show) {
		sprintf(se,"%c%02x%c",183,this_char & 0xff,183);
		if (write(STDOUT_FILENO,se,4) != 4) { }
		fdatasync(STDOUT_FILENO);
	      }
	      if (((s[0] == 0x0d) && (last_char != 0x0a)) ||
		  ((s[0] == 0x0a) && (last_char != 0x0d))) {
		if (opt_eol_input_linefeed == 1) {
		  se[0] = 0x0a;
		  if (write(STDOUT_FILENO,se,1) != 1) { }
		  fdatasync(STDOUT_FILENO);
		  opt_eol_input_linefeed = 0;
		}
		if (opt_eol_input_linefeed == 0) {
		  opt_eol_input_linefeed = 1;
		}
	      }
	    } else {
	      if (opt_eol_input_linefeed == 1) {
		se[0] = 0x0a;
		if (write(STDOUT_FILENO,se,1) != 1) { }
		fdatasync(STDOUT_FILENO);
		opt_eol_input_linefeed = 0;
	      }
	      if ((s[0] < 0x20) || (s[0] > 0x7e)) {
		sprintf(se,"%c%02x%c",183,this_char & 0xff,183);
		if (write(STDOUT_FILENO,se,4) != 4) { }
		fdatasync(STDOUT_FILENO);
	      } else {
		if (write(STDOUT_FILENO,s,1) != 1) { }
		fdatasync(STDOUT_FILENO);
	      }
	    }
	    last_char = this_char;
	  }
	}
      }
    } else {
      if (opt_rxhex) {
	screen_refresh();
      }
    }
  }
  if (! opt_rxonly) {  
    stdin_restoreattr();
  }
  if (sig_number >= 0) {
    fprintf(stderr,"NANOCOM: received signal %d\n",sig_number);
  }
  fprintf(stderr,"NANOCOM: exit\n");
  return 0;
}
