/*
    Name: Prabesh Humagain
    ID: 1001734700
    Email: prabesh.humagain@mavs.uta.edu
*/
#include "filestructure.h"

_DirectoryEntry *directory_ptr;

_Inode *inode_array_ptr[NUM_INODES];

unsigned char data_blocks[NUM_BLOCKS][BLOCK_SIZE];
int used_blocks[NUM_BLOCKS];

int df()
{
    int count = 0;
    int i = 0;
    for (i = 130; i < NUM_BLOCKS; i++)
    {
        if (used_blocks[i] == 0)
        {
            count++;
        }
    }
    return count * BLOCK_SIZE;
}

int findFreeInodeBlockEntry(int inode_index)
{
    int i = 0;
    int retVal = -1;
    for (i = 0; i < MAX_BLOCKS_PER_FILE; i++)
    {
        if (inode_array_ptr[inode_index]->blocks[i] == -1)
        {
            retVal = i;
            break;
        }
    }
    return retVal;
}

void init()
{
    int i;
    directory_ptr = (_DirectoryEntry *)data_blocks[0];
    for (i = 0; i < NUM_FILES; i++)
    {
        directory_ptr[i].valid = 0;
    }

    int inode_index = 0;
    for (i = 1; i < 130; i++)
    {
        inode_array_ptr[inode_index] = (_Inode *)&data_blocks[i];
        inode_array_ptr[inode_index++]->valid = 0;
    }
}

int findFreeDirectoryEntry()
{
    int i = 0;
    int retval = -1;
    for (i = 0; i < NUM_FILES; i++)
    {
        if (directory_ptr[i].valid == 0)
        {
            retval = i;
            break;
        }
    }
    return retval;
}

int findFreeInode()
{
    int i = 0;
    int retVal = -1;
    for (i = 0; i < NUM_INODES; i++)
    {
        if (inode_array_ptr[i]->valid == 0)
        {
            retVal = i;
            break;
        }
    }
    return retVal;
}

// get the free block from our data block
int findFreeBlock()
{
    int i = 0;
    int retVal = -1;
    for (i = 130; i < NUM_BLOCKS; i++)
    {
        if (used_blocks[i] == 0)
        {
            retVal = i;
            break;
        }
    }
    return retVal;
}

void list(bool flag)
{
    int i = 0;
    int found = 0;
    for (i = 0; i < NUM_FILES; i++)
    {
        if (directory_ptr[i].valid == 1)
        {
            _Inode *directoryInode = inode_array_ptr[directory_ptr[i].inode];
            // if show hidden flag, just print the file metadata, we don't need to verify
            // anything
            if (flag)
                printf("%d\t%s\t%s\n",
                       directoryInode->size,
                       ctime(&directoryInode->date),
                       directory_ptr[i].name);
            else // otherwise, we have to check whether the file is marked as hidden
            {
                if (((directoryInode->attrib & 0x2) >> 1) % 2 == 0)
                    printf("%d\t%s\t%s\n",
                           directoryInode->size,
                           ctime(&directoryInode->date),
                           directory_ptr[i].name);
            }
            found = 1;
        }
    }
    if (found == 0)
    {
        printf("list: No files found\n");
    }
    return;
}

void put(char *filename)
{
    // stat system call to get the file info and metadata from the pwd
    struct stat st;

    printf("%s file has been uploaded \n", filename);
    int status = stat(filename, &st);

    // first we need to know if the file is present in the working directory
    if (status == -1)
    {
        fprintf(stderr, "Error: %d\n", errno);
        perror("File does not exist\n");
        fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        return;
    }

    // before we load the file on our FS, we need to also make sure that we have enough space
    if (st.st_size > df())
    {
        fprintf(stderr, "Error: %d\n", errno);
        perror("Not enough space\n");
        fprintf(stderr, "Error in file size: %s\n", strerror(errno));
        return;
    }

    // next our filesystem can only support 128 files (our requirement)
    // so, do we have valid directory entry?
    // we need to very that before we start loading
    int directoryIndex = findFreeDirectoryEntry();
    if (directoryIndex == -1)
    {
        fprintf(stderr, "Error: %d\n", errno);
        perror("No more directory entry on our filesystem \n");
        fprintf(stderr, "Error in directory entry: %s\n", strerror(errno));
        return;
    }

    // now we are ready to populate our directory entry in our FS
    // so we need to set the valid, name
    // then the Inode Number
    directory_ptr[directoryIndex].valid = 1;
    // without mallocing we will segfault, so be careful!!!
    directory_ptr[directoryIndex].name = (char *)malloc(strlen(filename) + 1);
    strncpy(directory_ptr[directoryIndex].name, filename, strlen(filename));

    // get the free inode block
    int inodeIndex = findFreeInode();

    if (inodeIndex == -1)
    {
        fprintf(stderr, "Error: %d\n", errno);
        perror("No more inodes\n");
        fprintf(stderr, "Error in num_inodes: %s\n", strerror(errno));

        // clean up the used directory to avoid memory corruption
        directory_ptr[directoryIndex].valid = 0;
        free(directory_ptr[directoryIndex].name);

        return;
    }

    // update the inode value now
    directory_ptr[directoryIndex].inode = inodeIndex;
    // now that we populate the directory entry, we have to update INODE value
    //  update the valid, size, date,attribute ... metadata
    inode_array_ptr[inodeIndex]->valid = 1;
    inode_array_ptr[inodeIndex]->size = st.st_size;
    inode_array_ptr[inodeIndex]->date = time(NULL);
    memset(inode_array_ptr[inodeIndex]->blocks,
           -1,
           MAX_BLOCKS_PER_FILE * sizeof(inode_array_ptr[inodeIndex]->blocks[0]));
    inode_array_ptr[inodeIndex]->attrib = 0x0000;
    // we need to set the attribute of the file
    // 0x01 = read only
    // 0x02 = hidden
    // and we can add upto another 6 bit map values to uniquely represent the value

    // Finally we are now ready to copy the file from the actual file to Data Block
    // First, open the file in read only mode
    FILE *fp = fopen(filename, "r");

    // get the file size of the file name from the stat sys call
    int copy_size = st.st_size;

    int offset = 0;

    // let's copy the file, and we are gonna do that in the block size

    // first, find the free blocks on our data blocks
    int block_index = findFreeBlock();

    if (block_index == -1)
    {
        // since we don't have block, we need to revert back everything
        fprintf(stderr, "Error: %d\n", errno);
        perror("No more blocks\n");
        // clean up the inode and directory entry

        // for inode, we can just say that the valid is 0, and we will be done
        inode_array_ptr[inodeIndex]->valid = 0;

        // next we need to clear the directory_ptr and make it available for next use
        free(directory_ptr[directoryIndex].name);
        directory_ptr[directoryIndex].valid = 0;

        return;
    }

    // we are going to copy the file in a chunk size of BLOCK_SIZE
    while (copy_size >= BLOCK_SIZE)
    {
        int block_index = findFreeBlock();
        used_blocks[block_index] = 1;

        int inode_block_index = findFreeInodeBlockEntry(inodeIndex);
        if (inode_block_index == -1)
        {
            fprintf(stderr, "Error: %d\n", errno);
            perror("No more inode blocks\n");
            // clean up the inode and directory entry
            inode_array_ptr[inodeIndex]->valid = 0;
            directory_ptr[directoryIndex].valid = 0;

            return;
        }
        inode_array_ptr[inodeIndex]->blocks[inode_block_index] = block_index;
        printf("Index: %d\n", inode_block_index);
        if (block_index == -1)
        {
            fprintf(stderr, "Error: %d\n", errno);
            perror("No more blocks\n");
            // clean up the inode and directory entry
            inode_array_ptr[inodeIndex]->valid = 0;
            directory_ptr[directoryIndex].valid = 0;

            return;
        }

        fseek(fp, offset, SEEK_SET);

        int bytes = fread(data_blocks[block_index], BLOCK_SIZE, 1, fp);

        // we need to check if we reached the end of the file
        // we just store bytes value to double verify
        if (bytes == 0 && !feof(fp))
        {
            fprintf(stderr, "Error: %d\n", errno);
            perror("Error reading file\n");
            return;
        }

        clearerr(fp);

        // we need to decrement by the BLOCK_SIZE because we have read the BLOCK_SIZE data
        copy_size -= BLOCK_SIZE;
        // similarly increase the BLOCK_SIZE
        offset += BLOCK_SIZE;
    }

    // what if the file size is not the multiple of BLOCK_SIZE, so we need to
    // add this line so that we don't miss anything
    // this is just for the last block
    if (copy_size > 0)
    {
        // get the free block
        int block_index = findFreeBlock();

        // if block_index still -1, throw error and clean up the used inode and directory entry :(
        if (block_index == -1)
        {
            fprintf(stderr, "Error: %d\n", errno);
            perror("No more blocks\n");
            // clean up the inode and directory entry
            inode_array_ptr[inodeIndex]->valid = 0;
            directory_ptr[directoryIndex].valid = 0;
            return;
        }

        used_blocks[block_index] = 1;

        // now that we got the inode, we also need to store the block into the inode
        int inode_block_index = findFreeInodeBlockEntry(inodeIndex);

        if (inode_block_index == -1)
        {
            fprintf(stderr, "Error: %d\n", errno);
            perror("No more inode blocks\n");
            // clean up the inode and directory entry
            inode_array_ptr[inodeIndex]->valid = 0;
            directory_ptr[directoryIndex].valid = 0;

            return;
        }
        inode_array_ptr[inodeIndex]->blocks[inode_block_index] = block_index;

        fseek(fp, offset, SEEK_SET);
        int bytes = fread(data_blocks[block_index], copy_size, 1, fp);
    }
    fclose(fp);
    return;
}

_Inode *checkIfFileExist(char *fileName)
{
    int i;
    for (i = 0; i < NUM_FILES; i++)
    {
        if (directory_ptr[i].valid == 1 &&
            strcmp(directory_ptr[i].name, fileName) == 0)
        {
            _Inode *directoryInode = inode_array_ptr[directory_ptr[i].inode];
            return directoryInode;
        }
    }
    return NULL;
}

void attrib(int position, int booleanVal, char *fileName)
{
    _Inode *ptr = checkIfFileExist(fileName);

    if (ptr != NULL)
    {
        uint8_t mask = 1 << position;
        ptr->attrib = (ptr->attrib & ~mask) | ((booleanVal << position) & mask);
    }
    else
    {
        printf("attrib: File not found");
    }
    return;
}

void get(char *fileName, char *newFileName)
{
    _Inode *ptr = checkIfFileExist(fileName);
    if (ptr != NULL)
    {
        int i;
        FILE *fp = fopen(newFileName, "w+");

        for (i = 0; i < MAX_BLOCKS_PER_FILE; i++)
        {
            uint32_t block = ptr->blocks[i];
            if (block != -1)
            {
                if (fputs(data_blocks[block], fp) == EOF)
                {
                    fprintf(stderr, "Error: %d\n", errno);
                    perror("Error writing to file\n");
                    return;
                }
            }
        }
        fclose(fp);
    }
    else
    {
        printf("get error: File not found.");
    }
    return;
}

void del(char *fileName)
{
    // deleting the file is a bit tricky
    // we need to first free the content inside the diectory entry
    // then we need to free the inode it uses
    _Inode *ptr = checkIfFileExist(fileName);
    if (ptr->attrib & 0x1)
    {
        printf("del: That file is marked read-only.");
        return;
    }
    if (ptr != NULL)
    {
        // also we need to free the block
        int i;
        // first updating the directory entry
        for (i = 0; i < NUM_FILES; i++)
        {
            if (directory_ptr[i].valid == 1 &&
                strcmp(directory_ptr[i].name, fileName) == 0)
            {
                directory_ptr[i].valid = 0;
                free(directory_ptr[i].name);
            }
        }
        // next clearing the inode valid entry and blocks
        ptr->valid = 0;
        memset(ptr->blocks,
               -1,
               MAX_BLOCKS_PER_FILE * sizeof(ptr->blocks[0]));
    }
    else
    {
        printf("del error: File not found.");
    }
    return;
}

void createFs(char *fileName)
{
    FILE *fp = fopen(fileName, "w+");
    if (fp == NULL)
    {
        printf("createFs: Error creating file system.");
        return;
    }
    fclose(fp);
}

FILE *openedFileSystem; // this stores the opened file system

void openFs(char *fileName)
{
    openedFileSystem = fopen(fileName, "r+");
    if (openedFileSystem == NULL)
    {
        printf("openFs: Error opening file system / File not found.");
        return;
    }
    return;
}

void saveFs()
{

    fclose(openedFileSystem);
}