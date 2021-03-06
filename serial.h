#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

int serial_init(int index);
int serial_is_send_enable(int index);
int serial_send_byte(int index, unsigned char b);
int serial_is_recv_enable(int index);
unsigned char serial_recv_byte(int index);

#endif /* SERIAL_H_INCLUDED */
