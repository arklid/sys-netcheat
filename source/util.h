#include <switch.h>
#define MAX_LINE_LENGTH 300

extern Mutex actionLock;
extern int sock;

void fatalLater(Result err);
int setupServerSocket();
void print_hex(u64 addr, u32 size);
