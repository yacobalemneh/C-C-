#include "csapp.h"


int parseline(char *buf, char **argv)
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
        buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0)  /* Ignore blank line */
        return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}



int main(int argc, char **argv)
{
    int clientfd;
    int test = 0;
    char *host, *port, buf[MAXLINE], choice[MAXLINE], serverBuf[MAXLINE], *my_argv[128];
    rio_t rio;
    unsigned int key;   
    
    if (argc != 4) { // Error message if arguements aren't met 
        fprintf(stderr, "usage: %s <host> <port> <key>\n", argv[0]);
        exit(0);
    }
    
    host = argv[1]; // store host
    port = argv[2]; // store port
    key = atoi(argv[3]);  // get key and convert to int
    buf[0] = '\0';
    buf[1] = '\0';  // initialize buffers
    buf[2] = '\0';
    buf[3] = '\0';
    memcpy(&buf[4], &key, 4); // send the first four bytes to server
    
    clientfd = Open_clientfd(host, port);  // open client
    Rio_readinitb(&rio, clientfd); // read
    Rio_writen(clientfd, buf, sizeof(buf)); 
    Rio_readnb(&rio, serverBuf, sizeof(serverBuf)); 
    printf("Possible list of commands:\n");   
    printf("To Copy File Locally Enter: cp [FILE] [NEWFILE] \n");
    printf("To Copy File to Cloud Enter: cp [FILE] c: [NEWFILE] \n");
    printf("To Copy File from the Cloud Enter: cp c: [FILE] [NEWFILE] \n");
    printf("To Remove File Stored on the Cloud Enter: rm c: [FILE] \n");
    printf("To Remove File Stored Locally Enter: rm [FILE] \n");
    printf("To list files stored in Cloud Enter: list \n \n");
  	
	
    while (1){
        test = 0;
        if (serverBuf[0] == '9' && serverBuf[1] == '9'){
            printf("Invalid key!\n");  // check to see if keys match
            exit(2);
        }
        printf("> ");
        Fgets(choice, MAXLINE, stdin); //get user input 
	parseline(choice, my_argv); // user parseline to break up commands

        if (strcmp(my_argv[0], "quit") == 0) { // exit choice
	    printf("GOODBYE! \n");
	    exit(0);
       	    }
        else if (strcmp(my_argv[0],"cp") == 0)  {  // user chooses to copy
                 
            if (((my_argv[1]) == NULL) || (my_argv[2] == NULL) ){ // check to make sure user provides file name
                printf("Invalid command: Needs a filename!");
		printf("Enter as: cp [FILE] [NEWFILE]\n");
            }   
	    else if ( (my_argv[3] == NULL)) { // user chooses to copy local file
		FILE *fp1, *fp2;
		
		fp1 = fopen(my_argv[1], "r");
		if (fp1 != NULL) {
	 		fp2 = fopen(my_argv[2], "w"); // new file created with name provided in my_argv[1]
   			char ch;
   			int pos;
 			fseek(fp1, 0L, SEEK_END); // file pointer at end of file
   			pos = ftell(fp1);
   			fseek(fp1, 0L, SEEK_SET); // file pointer set at start
    			while (pos--)  {
       			   ch = fgetc(fp1);  // copying file character by character
       		  	   fputc(ch, fp2);
   		 	}   
			printf("SUCCESS File Copied Locally\n"); 
  	  		fclose(fp2);
			fclose(fp1); 
	 		}
		else {
		 	printf("FILENAME doesn't exist \n Enter as: cp [FILE] [NEWFILE}\n");

			}	

        	}
	      	   
            else if (strcmp(my_argv[2],"c:") == 0)  { //send file
                buf[0] = '1';  // if copying to cloud pass store 1 in first byte
                buf[1] = '\0';
                buf[2] = '\0';
                buf[3] = '\0';
               
                FILE *get;
                get = fopen(my_argv[1], "r"); // open file to copy
                if (get != NULL){
                    fseek(get, 0L, SEEK_END);
                    unsigned int size = ftell(get);
                    rewind(get);
                    char filecontents[MAXLINE];
                    Fread(&filecontents, sizeof(filecontents), 1, get);
                    memcpy(&buf[8],my_argv[3], 80);   // send file name to server
                    memcpy(&buf[87], &size, 4); // send size to server
                    memcpy(&buf[92], filecontents, size); // send file contents
		    printf("File COPY SUCCESS \n");
                }
                else{
                    printf("Error: No file found!\n"); // error if no file found
                    buf[0] = '\0';
                    buf[1] = '\0';
                    buf[2] = '\0';
                    buf[3] = '\0';
               	    }
                fclose(get);
            	}
       	     else if (strcmp(my_argv[1], "c:") == 0){   // copy file from cloud
                if (my_argv[2] == NULL){
                   printf("Invalid command: Needs a filename!"); // make user user gives valid filename
		   printf("Enter as: cp c: [FILE] [NEWFILE] \n");
                   } 
		else if (my_argv[3] == NULL) {
		   printf("Invalid command: Needs new file name! ");
		   printf("Enter as: cp c: [FILE] [NEWFILE] \n");
		} 
                else{
                   buf[0] = '2';
                   buf[1] = '\0'; // store 2 in buffer
                   buf[2] = '\0';
                   buf[3] = '\0'; 
                   memcpy(&buf[8], my_argv[2], 80); // pass buffer and filename to server
            }
	  }
	}
      else if (strcmp(my_argv[0], "rm") == 0){ //remove choice
            if (my_argv[1] ==  NULL) {
                printf("Invalid command: Needs a filename!");
		printf("Enter as: rm [FILE] to Delete Local File or \n");
		printf("Enter as: rm c: [FILE] to Delete file on Cloud. \n"); //make sure its a valid choice
            }

	    else if (my_argv[2] == NULL)  {  // user chooses to delete local file
		int status;
		status = remove(my_argv[1]);
		if (status == 0) {
			printf("[%s] DELETED\n", my_argv[1]);
			}
		else  {
			printf("Unable to Find File!\n"); // if file isnt found
        	}
		
	    }
         
	      else if (strcmp(my_argv[1], "c:") == 0) { // user chooses to delete file in cloud
		 if (my_argv[2] == NULL) {
		    printf("Invalid \n Enter as: rm c: [FILE] \n");
		    }
		 else {
		    buf[0] = '3';  // then store 3 in buffer to pass to server
                    buf[1] = '\0';
                    buf[2] = '\0';
                    buf[3] = '\0';
                    memcpy(&buf[8], my_argv[2], 80); // pass buffer and filename to server

		} 
       
            }
	}
	
        else if (strcmp(my_argv[0], "list") == 0){
            buf[0] = '4';
            buf[1] = '\0';  // list option
            buf[2] = '\0'; // pass 4 in buffer
            buf[3] = '\0';
        }
	
        else{
            printf("Invalid command!\n"); /// message if user hasnt made a valid choice
	    printf("Possible list of commands:\n");
	    printf("To Copy File Locally Enter: cp [FILE] [NEWFILE] \n");
            printf("To Copy File to Cloud Enter: cp [FILE] c: [NEWFILE] \n");
	    printf("To Copy File from the Cloud Enter: cp c: [FILE] [NEWFILE] \n");
	    printf("To Remove File Stored in Cloud Enter: rm c: [FILE] \n");
	    printf("To Remove File Stored Locally Enter: rm [FILE] \n");
	    printf("To list files stored in Cloud Enter: list \n \n");
	  
            buf[0] = '\0';
            buf[1] = '\0';
            buf[2] = '\0';
            buf[3] = '\0';
            test = 1;
        }     
        clientfd = Open_clientfd(host, port); 
        Rio_readinitb(&rio, clientfd);
        Rio_writen(clientfd, buf, sizeof(buf));
        Rio_readnb(&rio, serverBuf, sizeof(serverBuf));
        Close(clientfd); //line:netp:echoclient:close // 
        int returned;
        memcpy(&returned, &serverBuf[0], 1);
        if (returned == 0xFF && test != 1 && buf[0] == '1'){
            printf("Error with file UPLOAD\n"); // Error message
        }
        else if (returned == 0xFF && test != 1 && buf[0] == '2'){
            printf("Error with file RETIREVAL\n"); // error message
        }
        if (serverBuf[0] == '0' && buf[0] == '2'){
            int size;
            memcpy(&size, &serverBuf[4], 4);
            char data[MAXLINE];
            memcpy(&data, &serverBuf[8], size);
            FILE *write;
            write = fopen(my_argv[3], "w+");  // file creation if user chooses to 
            fputs(data, write);			// copy file from cloud
            fclose(write);
            printf("[%s] COPIED to this directory as [%s]\n", my_argv[2], my_argv[3]);
        }
        if (serverBuf[0] == '0' && buf[0] == '3'){
            printf("[%s] DELETED\n", my_argv[2]); // message if user deletes a file
        }
        if (serverBuf[0] == '0' && buf[0] == '4'){
            int entries = 0;
            memcpy(&entries, &serverBuf[4], 4);
            int i = 0;
            while (entries != i){ // while loop to display files if user chooses list
                char file[80];
                memcpy(file, &serverBuf[i*80+8], 80);
                printf("%s\n", file); 
		i++;
             
            }
        }
      }
   	         
   } 
