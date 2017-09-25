/*
 * @author Santosh Kumar Shaw (devsks)
 * @quote "Code like there's no tommorow!"
 *
 *	This command-line tools helps users to search information
 *	about a pirticular topic with the help of rss feed links
 *	by using the python module API in the backend.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
//#define debug
#define BLOCK 512
#define TABS 3
#define linkSource "./links"
/*	Methods Declaration */
void fire(const char* msg)
{
    perror(msg), exit(1);
}
void* loadMyFeeds(void*);	//Reads the links from the sourceFile using threads
void clean(char**, int);	//Memorymanagement is very important

int main(int argc, char**argv)
{
    char **feeds = NULL, str[BLOCK];
	int number, ch_id, ch_status, filedes[2], tabs = 0, input, output;
    
	// Search Pharase is important
	if (argc == 1)
        fire("Usage: ./read_feed [searchkey]");
	
	//	Threads to read links from file
	pthread_t fileThread;
	pthread_create(&fileThread, NULL, loadMyFeeds, (void*)&feeds);
    pthread_join(fileThread, (void*)&number);

	char *args[] = {"/usr/bin/python", "./rssgossip.py", "-u", argv[1], NULL}, 
         *env[] = {NULL, NULL};
    
	if (pipe(filedes) == -1)	// InterProcess Communication
        fire("Unable to create pipe");
    // For better understanding
	input = filedes[0], output = filedes[1];
	
	for(int i = 0; i < number; ++i)
    {
        #ifdef debug
            printf("Using the FEED: %s\n",feeds<:i:>);
        #endif
		asprintf((char**)&env, "RSS_FEED=%s", feeds<:i:>);	//	Creating environment variable
        
        if ((ch_id = fork()) == 0)  // one Child Process for every RSS Feeds
        {
            close(input);
            dup2(output, fileno(stdout));
			// Calling pyhton API
            if(execve(args<:0:>, args, env) == -1)
                fire("Unable to continue...");
        }
        
		waitpid(ch_id, &ch_status, 0);			//Wait for child process to read the feed
        int exit_status = WEXITSTATUS(ch_status);
		#ifdef debug  
       		printf("Process returned with %i\n", exit_status);
        #endif
       	if(exit_status == 1)
			continue;

		close(output);							
        FILE * news_buf = fdopen(input,"r");
		//dup2(input, fileno(stdin));				//Direct's stdin to child's output
        while (fgets(str, BLOCK, news_buf) != NULL)
        {
			puts(str);
            if (str[0] == '\t')
            {
                #ifdef debug
					fprintf(stdout, "Found Data at %s.Opening it in the brower\n", str);
                #endif
				char* browse = NULL;
                asprintf(&browse, "open %s", str);
                //system(browse);
				free(browse);
                ++tabs;
            }
            if(tabs==TABS)
            {
                return 0;
            }
        }
       
    }
    puts("Unable to find news Today :(  Try to add more RSS_FEED to the file links");
    return 0;
}
void* loadMyFeeds(void* args)
{
	char ***array = (char***) args;
    FILE* inp = fopen(linkSource,"r");
    if (inp == NULL)
        fire("Unable to reads link source file");
    
    char link[BLOCK];
    size_t size = 8, index = 0;
    *array = (char**) calloc(size, BLOCK);
    
    while(fgets(link, BLOCK, inp) !=NULL)
    {
        if(index == size)
        {
            size <<= 1;
            *array = (char**) realloc(*array, size * BLOCK);
        }
        /*
			making a copy of string in heap 
			and saving it to array
		*/
        array[0][index++] = strdup(link);
    }
	return (void*)index;
}
