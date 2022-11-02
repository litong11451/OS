
#include <stdint.h>
#include <kernel/wrap_asm.h>
#include <kernel/tty.h>
#include <driver/keyboard.h>
#include <stdio.h>
#include <stdbool.h>

char scan_code_chars[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
    '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

uint16_t kbd_ack() {
    uint8_t ack ,s;
    uint16_t i = 1000;
    do {
        s = inb(STATUS_PORT);
        if(s & 0x01){
            ack = inb(DATA_PORT);
            if(ack == ACK){
                return ack;
            }
        }
    }while(--i != 0);
    return i;
}

uint16_t kbd_ready() {
    uint8_t s;
    uint16_t i = 1000;
    do {
        s = inb(STATUS_PORT);
        printf("s=%d:",s);
        if(s & 0x01){
            break;
        }
    }while(--i != 0);
    return i;
}

uint8_t read_data() {
    uint8_t s;
    uint16_t i = 1000;
    do {
        s = inb(STATUS_PORT);
        //printf("s=%d:",s);
        if(s & 1){
            uint8_t d = inb(DATA_PORT);
            return d;
        }
    }while(--i != 0);
    return i;
}


void cmd(uint8_t cmd) {
    uint8_t s;
    uint16_t i = 1000;
    do {
        s = inb(STATUS_PORT);
        //printf("s=%d:",s);
        if((s & 2) == 0){
            outb(CMD_PORT,cmd);
            break;
        }
    }while(--i != 0);
}

void write_data(uint8_t d) {
    uint8_t s;
    uint16_t i = 1000;
    do {
        s = inb(STATUS_PORT);
        //printf("s=%d:",s);
        if((s & 2) == 0){
            outb(DATA_PORT,d);
            break;
        }
    }while(--i != 0);
}

void kbd_init() {
    //Disable port
    cmd(DIS_PORT1);
    cmd(DIS_PORT2);
    //flush out buffer
    read_data();
    //read configuration byre
    cmd(CONFIG_READ);
    uint8_t c = read_data();
    //printf("PS/2 configuration:%d\n",c);
    uint8_t supported2 = c & 0x20;
    //printf("PS/2 supported2:%d\n",supported2);
    //test
    cmd(PS2_TEST);
    uint8_t test = read_data();
    //printf("PS/2 self test:%d\n",test);

    cmd(PS2_PORT1_TEST);
    uint8_t test1 = read_data();
    //printf("PS/2 PORT1 self test:%d\n",test1);

    cmd(PS2_PORT2_TEST);
    uint8_t test2 = read_data();
    //printf("PS/2 PORT2 self test:%d\n",test2);

    c |= 1;
    c &= 127;
    if(supported2) {
        c |= 3;
    }

    cmd(CONFIG_WRITE);
    write_data(c);

    //enable port
    cmd(EN_PORT1);
    if(supported2) {
        cmd(EN_PORT2);
    }
    
    //printf("PS/2 init end");
    cmd(0xff);
    uint8_t reset = read_data();
    printf("PS/2 reset:%d\n",reset);
}

uint8_t scan_keyboard() {
    uint8_t s = inb(STATUS_PORT);
    if(!(s & 1)) {
        return -1;
    }
    uint8_t d = inb(DATA_PORT);
    if(d == ACK) {
        return -1;
    }
    return d;

}

void keyboard_handler() {
    uint8_t code = scan_keyboard();
    printf("PS/2 code:%d\n",code);
    char c;
    if(code & 0x80) {
        //__asm__ volatile ("cli;hlt"); 
    }else {
        c  = scan_code_chars[code];
        printf("PS/2 char:%c\n",c);
    }
    
}
