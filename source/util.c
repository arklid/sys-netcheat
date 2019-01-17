#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <switch.h>
#include "util.h"
#include "cheat.h"

int setupServerSocket()
{
    int lissock;
    struct sockaddr_in server;
    lissock = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5555);

    while (bind(lissock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        svcSleepThread(1e+9L);
    }
    listen(lissock, 3);
    return lissock;
}

void fatalLater(Result err)
{
    Handle srv;

    while (R_FAILED(smGetServiceOriginal(&srv, smEncodeName("fatal:u"))))
    {
        // wait one sec and retry
        svcSleepThread(1000000000L);
    }

    // fatal is here time, fatal like a boss
    IpcCommand c;
    ipcInitialize(&c);
    ipcSendPid(&c);
    struct
    {
        u64 magic;
        u64 cmd_id;
        u64 result;
        u64 unknown;
    } * raw;

    raw = ipcPrepareHeader(&c, sizeof(*raw));

    raw->magic = SFCI_MAGIC;
    raw->cmd_id = 1;
    raw->result = err;
    raw->unknown = 0;

    ipcDispatch(srv);
    svcCloseHandle(srv);
}

// Prints the specified number of bytes as prettified hex and ascii
void print_hex(u64 addr, u32 size) {

    u8 this_line[16], last_line[16];
    bool matched_before = false;

    u8* ptr = malloc(size);
    svcReadDebugProcessMemory(&ptr, debughandle, addr, size);

    // print header
    printf("\n     OFFSET       0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F");

    const u32 rows = size / 16;
    const u32 row_remainder = size % 16;

    for(u32 i = 0; i < rows; i++) {

        // store this line so we can compare it to the last line (and print a "*" instead of a bunch of duplicate lines)
        memcpy(this_line, ptr + i * 16, 16);

        // if this isn't the first loop, check to see if this line matches the last line
        if (i > 0 && memcmp(this_line, last_line, 16) == 0) {

            // it does, but don't print another "*" if we just did
            if (!matched_before) {
                printf("\n     *");
                matched_before = true;
            }
            continue;
        }

        // not a duplicate line
        matched_before = false;

        // start a new row and print the offset (in hex)
        printf("\n     0x%08lx  ", (u64)addr + (i * 16));

        // print the next 16 characters in hex
        for(u32 i1 = 0; i1 < 16; i1++) {
            u32 c = i * 16 + i1;
            printf("%02X ", ptr[c]);
        }

        // print them again in ascii
        printf(" ");

        for(u32 i1 = 0; i1 < 16; i1++) {
            const u32 c = i * 16 + i1;

            // print zeros as dots
            if(ptr[c] == 0)
                printf (".");

            // don't print control codes! (print a space instead)
            else if (ptr[c] < 0x20 || (ptr[c] > 0x7E && ptr[c] < 0xA1))
                printf(" ");
            else
                printf("%c", ptr[c]);
        }

        memcpy(last_line, this_line, 16);
    }

    if (row_remainder) {
        // start a new row and print the offset (in hex)
        printf("\n     0x%08lx  ", (u64)addr + (rows * 16));

        // print the last remaining characters in hex
        for(u32 i = 0; i < row_remainder; i++) {
            const int c = rows * 16 + i;
            printf("%02X ", ptr[c]);
        }

        // pad with spaces up to ascii area
        for (u32 i = 0; i < 16 - row_remainder; i++) {
            printf("   ");
        }

        // print remaining characters again in ascii
        printf(" ");

        for(u32 i = 0; i < row_remainder; i++) {
            const u32 c = rows * 16 + i;

            // print zeros as dots
            if(ptr[c] == 0)
                printf (".");

            // don't print control codes! (print a space instead)
            else if (ptr[c] < 0x20 || (ptr[c] > 0x7E && ptr[c] < 0xA1))
                printf(" ");
            else
                printf("%c", ptr[c]);
        }
    }
    
    printf("\n");
}


