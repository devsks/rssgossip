/*
 * @author Santosh Kumar Shaw (devsks)
 * @quote "Code like there's no tommorow!"
 */
 #include <stdio.h>
 #include <string.h>
 #include <unistd.h>
 #include <errno.h>
 #include <stdlib.h>
 #define LINK_S 512
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
     size_t size = 10, index;
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
     if(argc==1)
         fire("");
     int number = loadMyFeeds(&feeds), ch_id,ch_status, fds[2];
     char *args[] ={"/usr/bin/python","./rssgossip.py", argv[1],NULL}, 
         *env[] = {NULL,NULL};
     if(pipe(fds)==-1)
         fire("Unable to create pipe");
     for(int i = 0; i < number; ++i)
     {
         printf("Using the FEED: %s\n",feeds[i]);
         asprintf(&env, "RSS_FEED=%s", feeds[i]);
         
         
         if( (ch_id = fork()) == 0)
         {
             close(fds[0]);
             dup2(fds[1],1);
             if(execve(args[0], args, env) == -1)
                 fire("Unable to continue");
         }
         waitpid(ch_id,&ch_status,0);
         dup2(fds[0],0);
         close(fds[1]);
         char str[123];
         while(fgets(str,123,stdin))
            puts(str);
     
         printf("Process returned %i\n",WEXITSTATUS(ch_status));
 
     }
     
     return 0;
 }