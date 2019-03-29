#include "csapp.h"  


int main(int argc, char **argv)
{
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE], retVal[MAXLINE], *my_argv[128] ;
    unsigned int key, clientkey;
       
    if (argc != 3) {  // make sure all arguements are entered
        fprintf(stderr, "usage: %s <port> <key>\n", argv[0]);
        exit(0);
    }
    
    port = atoi(argv[1]);
    key = atoi(argv[2]); // change key to int
    
    
    listenfd = Open_listenfd(argv[1]); //open listen fc
    while (1) {
        clientlen = sizeof(struct sockaddr_storage); 
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
                    client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        char buf[MAXLINE];
        
  
        Rio_readn(connfd, &buf, MAXLINE);
        memcpy(&clientkey, &buf[4], 4);
        if (key != clientkey){ // validate key
            printf("Failure: invalid key!\n");
            retVal[0] = '9';
            retVal[1] = '9';
            retVal[2] = '9';
            retVal[3] = '9';
        }
	
        else{
            if(buf[0] == '1'){ // 1 is passed from client indicating server to "copy" file
                printf("Secret key: %d\tSTORE\t", key);
                int size;
                memcpy(&size, &buf[87], 4);
                
                char data[MAXLINE];
                memcpy(&data, &buf[92], size);
                
                char filename[80];
                memcpy(&filename, &buf[8], 80); // file name recieved from client
                
                FILE *write;
                write = fopen(filename, "w+");    //new file created
                if (fputs(data, write) != EOF){
                    retVal[0] = '0';
                    printf("%s\t[SUCCESS]\n",filename);   // success message
                }
                else{
                    int mem = -1;
                    memcpy(&retVal[0], &mem, 1);
                    printf("%s\t[FAILURE]\n", filename); 
                }
                fclose(write); // close file when done
                
            }
            else if(buf[0] == '2'){ // recieve file from server
                printf("Secret key: %d\t[RETRIEVE]\t", key);
                char filename[80];

                memcpy(filename, &buf[8], 80);
                FILE *get;
                get = fopen(filename, "r"); // open file on server and read file
                if (get != NULL){
                    fseek(get, 0L, SEEK_END);
                    unsigned int size = ftell(get);
                    rewind(get);
                    char filecontents[MAXLINE];
                    Fread(&filecontents, sizeof(filecontents), 1, get); //read file contents
                    memcpy(&retVal[4], &size, 4);   			// and send to client
                    memcpy(&retVal[8], &filecontents, size);
                    printf("%s\t[SUCCESS]\n", filename); 
                    retVal[0] = '0';
                }
                else{
                    printf("%s\t[FAILURE]\n", filename);    
                    int mem = -1;
                    memcpy(&retVal[0], &mem, 1);
                }
                fclose(get);
                
            }
            else if (buf[0] == '3'){ // remove file from server 
                printf("Secret key: %d\t[REMOVE]\t", key);
                char filename[80];
                memcpy(filename, &buf[8], 80); // get filename to remove from user
                if (strcmp(filename, "Server") == 0){
                    printf("%s\t[FAILURE]\n", filename);
                    int mem = -1;
                    memcpy(&retVal[0], &mem, 1);
                }
                else if (remove(filename) == 0){
                    printf("%s\t[SUCCESS]\n", filename);
                    retVal[0] = '0';
                }
                else{
                    printf("%s\t[FAILURE]\n", filename);
                    int mem = -1;
                    memcpy(&retVal[0], &mem, 1);
                }
            }
            else if (buf[0] == '4'){
                printf("Secret key: %d\t[LIST]\t", key);
                DIR *d;
                struct dirent *dir;
                d = opendir(".");
                int num = 0;
                if (d){
                    while ((dir = readdir(d)) != NULL){
                        if (strcmp(dir->d_name, "Server") != 0){
                            char file[80];
                            memcpy(file, dir->d_name, 80);
                            memcpy(&retVal[num*80+8], file, 80);//send list of filenames 
                            num++;				// to client for list request
                        }
                        
                    }
                    memcpy(&retVal[4], &num, 4);
                    printf("\t[SUCCESS]\n");
                    retVal[0] = '0';
                }
	     
                else{
                    printf("\t[FAILURE]\n");
                    int mem = -1;
                    memcpy(&retVal[0], &mem, 1);
                }
                
            }
        }
        Rio_writen(connfd, retVal, sizeof(retVal));
    }
}
