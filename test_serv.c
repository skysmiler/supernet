#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>


int main( int argc, char** argv ){

	struct sockaddr_in servaddr;
	int    sockfd;


	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	memset( &servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(atoi(argv[1]));
	servaddr.sin_addr.s_addr = INADDR_ANY;

	bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
	listen(sockfd, 5);
	
	int connfd;
	struct sockaddr_in cliaddr;
	int    cli_len = sizeof(cliaddr);
	while(1){
		connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &cli_len );
		
		//printf("local %d:%d, foreign %d:%d\n", );
		if( fork() ==0 ){
			close(sockfd);
			write(connfd, "HEH", 3);
			exit(0);	
		}
		close(connfd);



	}

}
