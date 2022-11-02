#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <kernel/itr.h>
#include <kernel/tty.h>

//Keyboard controller
#define DATA_PORT       0x60
#define CMD_PORT        0x64
#define STATUS_PORT     0x64
#define DIS_PORT1       0xAD
#define DIS_PORT2       0xA7
#define EN_PORT1        0xAE
#define EN_PORT2        0xA8
#define CONFIG_READ     0x20
#define CONFIG_WRITE    0x60
#define PS2_TEST        0xAA
#define PS2_PORT1_TEST  0xAB
#define PS2_PORT2_TEST  0xA9
#define RESET           0xFF

#define ACK             0xFA

void kbd_init();

void keyboard_handler();

#endif