
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#define NUM_BLOCKS 4226
#define BLOCK_SIZE 8192
#define NUM_FILES 128
#define NUM_INODES 128
#define MAX_BLOCKS_PER_FILE 32

typedef struct _DirectoryEntry
{
    char *name;
    uint32_t inode;
    uint8_t valid;
} _DirectoryEntry;

typedef struct _Inode
{
    uint32_t blocks[MAX_BLOCKS_PER_FILE];
    uint32_t size; // max size is 10 megabytes
    time_t date;
    uint32_t valid;
    uint8_t attrib; // we need to use bitmask for this one:
                    // 0x01 = read only
                    // 0x02 = hidden
} _Inode;

int df();

int findFreeInodeBlockEntry(int inode_index);

void init();

int findFreeDirectory();

int findFreeDirectoryEntry();

int findFreeInode();

int findFreeBlock();

void list(bool flag); // flag  = 1, show hidden files, flag = 0 show visible files

void put(char *filename);

_Inode *checkIfFileExist(char *fileName);

void attrib(int position, int booleanVal, char *fileName);

void get(char *fileName, char *newFileName);

void del(char *fileName);

void createFs(char *fileName);

void openFs(char *fileName);

void saveFs();
