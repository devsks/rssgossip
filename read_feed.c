/*
 * @author Santosh Kumar Shaw (devsks)
 * @quote "Code like there's no tommorow!"
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#define LINK_S 512
//#define debug
#define linkSource "./links"
void fire(const char* msg)
{
    perror(msg);
    exit(1);
}
int loadMyFeeds(char*** array)
{
    FILE* inp = fopen(linkSource,"r");
    char link[LINK_S];
    size_t size = 10, index = 0;
    *array = (char**)calloc(size, LINK_S);
    if(inp == NULL)
        fire("Unable to reads link source file");
    while(fgets(link,LINK_S,inp))
    {

        if(index==size)
        {
            size<<=1;
            *array = (char**)realloc(*array,size*LINK_S);
        }
        (*array)[index++] = strdup(link);
    }
    return index;
}
int main(int argc, char**argv)
{
    char**feeds = NULL;
    char str[LINK_S];
    bool found = false;
    if(argc==1)
        fire("Usage: ./read_feed [searchkey]");
    int number = loadMyFeeds(&feeds), ch_id,ch_status, fds[2];
    char *args[] ={"/usr/bin/python","./rssgossip.py","-u",argv[1],NULL}, 
        *env[] = {NULL,NULL};
    if(pipe(fds)==-1)
        fire("Unable to create pipe");
    for(int i = 0; i < number; ++i)
    {
        #ifdef debug
            printf("Using the FEED: %s\n",feeds[i]);
        #endif
        asprintf(&env, "RSS_FEED=%s", feeds[i]);
        if( (ch_id = fork()) == 0)
        {
            close(fds[0]);
            dup2(fds[1],1);
            if(execve(args[0], args, env) == -1)
                fire("Unable to continue");
        }
        
		waitpid(ch_id,&ch_status,0);			//Wait for child process to read the feed
        close(fds[1]);							
        dup2(fds[0],0);						   	//Direct's stdin to child's output
        
        while(fgets(str, LINK_S, stdin))
        {
            if(str[0]=='\t')
            {
                #ifdef debug
					fprintf(stdout,"Found Data at %s.Opening it in the brower\n",str);
                #endif
				char* browse = NULL;
                asprintf(&browse,"open %s",str);
                system(browse);
				free(browse);
                found = true;
                break;
            }
        }
        if(found)
            return 0;
        #ifdef debug  
            printf("Process returned %i\n",WEXITSTATUS(ch_status));
        #endif
    }
    if(!found)
        puts("Unable to find news Today ;(. Try to add more RSS_FEED links");
    return 0;
}
