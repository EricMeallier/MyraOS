#include "shell.h"

#include "keyboard/keyboard.h"
#include "print/kprint.h"

#include "kernel/string.h"

#define BACKSPACE 0x8

void shell_run(void) {
    char buffer[SHELL_INPUT_BUFFER_SIZE];
    int index = 0;

    kprint(">> ");

    while (true) {
        key_event_t e;

        if (!keyboard_read_event(&e)) {
            continue;
        }

        if (!e.pressed || !e.ascii_value) {
            continue;
        }

        char ch = e.ascii_value;

        if (ch == '\n') {
            buffer[index] = '\0';

            kprint("\n");

            if (index > 0) {
                char command[SHELL_INPUT_BUFFER_SIZE];
                char args[SHELL_INPUT_BUFFER_SIZE];

                size_t i = 0;
                while (buffer[i] != ' ' && buffer[i] != '\0' && i < SHELL_INPUT_BUFFER_SIZE) {
                    command[i] = buffer[i];
                    i++;
                }
                command[i] = '\0';

                size_t j = 0;
                while (buffer[i] == ' ') i++;

                while (buffer[i] != '\0' && j < SHELL_INPUT_BUFFER_SIZE - 1) {
                    args[j++] = buffer[i++];
                }
                args[j] = '\0';


                while (i < SHELL_INPUT_BUFFER_SIZE) {
                    args[i] = buffer[i];

                    i++;
                }

                if (kstrcmp(command, "echo") == 0) {
                    kprintln(args);
                } else if (kstrcmp(command, "clear") == 0) {
                    kclear_screen();
                } else if (kstrcmp(command, "help") == 0) {
                    kprintln("Commands: echo, clear, help");
                } else {
                    kprintf("Unknown command: %s\n", buffer);
                }
            }

            index = 0;
            kprint(">> ");
        } else if (ch == BACKSPACE) {
            if (index > 0) {
                index--; 
                kclear(1);
            }
        } else if (index < SHELL_INPUT_BUFFER_SIZE - 1) {
            buffer[index++] = ch;
            kprintf("%c", ch);
        }
    }
}