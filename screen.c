#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "screen.h"

unsigned char buf[16];
uint32_t offset;
int pos;
time_t refresh_time = 0;

void screen_init(void) {
  pos = 0;
  offset = 0;
  refresh_time = time(NULL);
}

void screen_post(char c) {
  buf[pos++] = c;
  if (pos == 16) {
    screen_refresh();
    printf("\n");
    offset += 16;
    pos = 0;
  }
  if ((time(NULL) - refresh_time) > 1) {
    screen_refresh();
  }
}

void screen_refresh(void) {
  int i;
  refresh_time = time(NULL);
  if (pos == 0) {
    return;
  }
  printf("\r%08x  ",offset);
  for (i=0; i<8; i++) {
    if (pos > i) {
      printf("%02x ",buf[i]);
    } else {
      printf("   ");
    }
  }
  printf(" ");
  for (; i<16; i++) {
    if (pos > i) {
      printf("%02x ",buf[i]);
    } else {
      printf("   ");
    }
  }
  printf(" |");
  for (i=0; i<8; i++) {
    if (pos > i) {
      if (buf[i] > 0x1f) {
	printf("%c",buf[i]);
      } else {
	printf(".");
      }
    } else {
      printf(" ");
    }
  }
  for (; i<16; i++) {
    if (pos > i) {
      if (buf[i] > 0x1f) {
	printf("%c",buf[i]);
      } else {
	printf(".");
      }
    } else {
      printf(" ");
    }
  }
  printf("|");
  fflush(stdout);
}

void screen_newline(void) {
  screen_refresh();
  printf("\n");
  offset += pos+1;
  pos = 0;
}
