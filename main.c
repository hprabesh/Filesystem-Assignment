// The MIT License (MIT)
//
// Copyright (c) 2016, 2017 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include "filestructure.h"

#define WHITESPACE " \t\n" // We want to split our command line up into tokens
                           // so we need to define what delimits our tokens.
                           // In this case  white space
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5 // Mav shell only supports five arguments

int main()
{

    char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);

    // initializaiton of the file system
    init();
    while (1)
    {
        // Print out the mfs prompt
        printf("mfs> ");

        // Read the command from the commandline.  The
        // maximum command that will be read is MAX_COMMAND_SIZE
        // This while command will wait here until the user
        // inputs something since fgets returns NULL when there
        // is no input
        while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
            ;

        /* Parse input */
        char *token[MAX_NUM_ARGUMENTS];

        int token_count = 0;

        // Pointer to point to the token
        // parsed by strsep
        char *arg_ptr;

        char *working_str = strdup(cmd_str);

        // we are going to move the working_str pointer so
        // keep track of its original value so we can deallocate
        // the correct amount at the end
        char *working_root = working_str;

        // Tokenize the input stringswith whitespace used as the delimiter
        while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
               (token_count < MAX_NUM_ARGUMENTS))
        {
            token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
            if (strlen(token[token_count]) == 0)
            {
                token[token_count] = NULL;
            }
            token_count++;
        }

        // Now print the tokenized input as a debug check
        // \TODO Remove this code and replace with your shell functionality

        int token_index = 0;
        if (token[0] != NULL)
        {
            if (strcmp(token[0], "df") == 0) // implementing the df functionality
            {
                printf("Disk free: %d bytes\n", df());
            }
            else if (strcmp(token[0], "put") == 0) // implementing the put functionality
            {
                put(token[1]);
            }
            else if (strcmp(token[0], "list") == 0) // implementing the list functionality
            {
                // printf("__%d__", token_count);
                if (token_count == 3 && strstr(token[1], "-h") != NULL) // find if there is list hidden flag
                    list(1);                                            // show the hidden files
                else
                    list(0); // otherwise show only visible flag
            }
            else if (strcmp(token[0], "quit") == 0) // implementing the quit functionality
            {
                printf("Filesystem exited\n");
                break;
            }
            else if (strcmp(token[0], "attrib") == 0)
            {
                if (token_count >= 4)

                {
                    char *fileName = token[token_count - 2];
                    for (int i = 1; i < token_count - 2; i++)
                    {
                        if (strcmp(token[i], "+h") == 0)
                            attrib(1, 1, fileName);
                        else if (strcmp(token[i], "-h") == 0)
                            attrib(1, 0, fileName);
                        else if (strcmp(token[i], "+r") == 0)
                            attrib(0, 1, fileName);
                        else if (strcmp(token[i], "-r") == 0)
                            attrib(0, 0, fileName);
                        else if (strcmp(token[i], "+hr") == 0)
                        {
                            attrib(1, 1, fileName);
                            attrib(0, 1, fileName);
                        }
                        else if (strcmp(token[i], "-hr") == 0)
                        {
                            attrib(1, 0, fileName);
                            attrib(0, 0, fileName);
                        }
                        else
                            continue;
                    }
                }
                else
                {
                    printf("Invalid syntax for attrib\n");
                }
            }
            else if (strcmp(token[0], "get") == 0)
            {

                if (token_count == 4)
                {
                    get(token[1], token[2]);
                }
                else if (token_count == 3)
                {
                    get(token[1], token[1]);
                }
                else
                {
                    printf("Invalid syntax for get\n");
                }
            }
            else if (strcmp(token[0], "del") == 0)
            {
                if (token_count == 3)
                {
                    del(token[1]);
                }
                else
                {
                    printf("Invalid syntax for del\n");
                }
            }
            else if (strcmp(token[0], "createfs") == 0)
            {
                if (token_count == 3)
                {
                    createFs(token[1]);
                }
                else
                {
                    printf("createfs: File not found.\n");
                }
            }
        }

        free(working_root);
    }
    return 0;
}
