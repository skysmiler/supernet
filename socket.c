#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#include<fcntl.h>
#include<signal.h>
#include<sys/wait.h>

struct sockaddr_in serv_addr;

int listenfd;
int connfd;

void sys_error( const char* );
void sys_quit( const char* );


int _do_child( int sockfd ){
    fd_set allset, ioset;
    FD_ZERO( &allset );
    FD_SET( sockfd, &allset );

    ioset = allset;
    
//    select( sockfd+1, &ioset, NULL, NULL, NULL );
    
    write( sockfd, "world",5 );
    
}
int do_child( int sockfd ){
        
    _do_child( sockfd );
    
}
void do_signal( int sig ){
    pid_t      pid;
    int        status;
    
    if( sig == SIGCHLD ){
        printf( "get sigchld\n" );
        while( (pid = waitpid( -1, &status, WNOHANG ))>0)
            printf( "pid:%d exit\n",pid );
        return ;
    }
}
int main( int argc, char** argv ){
    fd_set allset;
    fd_set rset;
    int    res;
    
    listenfd = socket( AF_INET, SOCK_STREAM, 0 );

    memset( &serv_addr, 0, sizeof( serv_addr ) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons( atoi( argv[1] ) );
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    if(bind( listenfd, ( struct sockaddr* )&serv_addr, sizeof( serv_addr ) ) <0 )
        sys_error( "bind failed" );
    
    listen( listenfd, 5 );
    signal( SIGCHLD, do_signal );
    
    
    FD_ZERO( &allset );
    FD_SET( listenfd, &allset );
    while( 1 ){
        rset = allset;
        res = select( listenfd+1, &rset, NULL, NULL, NULL );
        
        if( res <=0 ){
            if( res == -1 && errno == EINTR )
                continue;
            
            perror( "select abort" );
            continue;
        }
        
        
        if( FD_ISSET( listenfd, &rset ) ){
            if( ( connfd = accept( listenfd,NULL,NULL ) )<0 )
                perror( "accept failed\n" );
            if( fork() ==0 ){ /* child process */
                close( listenfd );
                do_child( connfd );
		close(connfd);
                exit( 0 );
            }
            
            close( connfd );
            
        }
    }
}

void sys_quit( const char* msg ){
    fputs( msg, stderr );
    exit( -1 );
}
void sys_error( const char* msg ){
    perror( msg );
    exit( -1 );
}
