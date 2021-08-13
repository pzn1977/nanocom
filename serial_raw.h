#ifndef _serial_raw_h
#define _serial_raw_h

int serial_set_raw(int fd);
int serial_set_not_line_buffered(int fd);
int serial_set_no_echo(int fd);
int serial_set_baudrate(int fd, int baudrate);
int serial_set_min_time(int fd, int vmin, int vtime);

#endif
