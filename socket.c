#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/socket.h>
#include<arpa/inet.h>

#include<fcntl.h>
#include<signal.h>
#include<sys/wait.h>

struct sockaddr_in serv_addr;
#define MAX_CHILD_FORK 1024

int listenfd;
int connfd;
struct childpid_t {
    pid_t    pid;
    int      pread;
    int      pwrite;
    int      lock; // semid for lock
} childpid[MAX_CHILD_FORK];


void sys_error( const char* );
void sys_quit( const char* );

    //for semaphore
void lock_init( int* );
void lock_get( int* );
void lock_put( int* );



int _do_child( int sockfd ){
    int res;
    fd_set allset, ioset;
    char buf[128];
    

    FD_ZERO( &allset );
    FD_SET( sockfd, &allset );
    
    while( 1 ){
        ioset = allset;
        res = select( sockfd+1, &ioset, NULL, NULL, NULL );
        if( res <=0 ){
            perror( "child select:" );
            continue;
        }
        if( FD_ISSET( sockfd, &ioset ) ){
            memset( buf, 0, sizeof( buf ) );
            res  = read( sockfd, buf, sizeof( buf ) );
            if( res == 0 )
                break;
            else if( res >0 ){
                if( buf[0] == '.' ){
                    
                    if( !strcmp( ".exit\n", buf ) )
                        break;
                }
                
                printf( "Read data:%d bytes\n", res );
                write( sockfd, buf, res );
            }
            
        }
    }
    return 0;
}
int do_child( int sockfd ){
        
    return _do_child( sockfd );

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
int pick_child( void ){
    int child;
    for( child=0; child < MAX_CHILD_FORK; child++ ){
        if( childpid[child].pid == 0 )
            break;
    }
    if( child == MAX_CHILD_FORK )
        return -1;
    return child;
}
int child_make( int connfd ){
    int child;
    int pd[2];
    
    if( connfd <0 )
        return -1;
    if( (child = pick_child()) <0 ){
        fputs( "max child exceed been forked!", stderr );
        write( connfd, "max fork\n",sizeof( "max fork\n" ) );
        return -1;
    }
    if( pipe( pd ) <0 ){
        perror( "child_make->child_make" );
        return -1;
    }
    lock_init( &childpid[child].lock );
    lock_get( &childpid[child].lock );
    childpid[child].pread = pd[0];
    childpid[child].pwrite = -1;
    lock_put( &childpid[child].lock );
    
    if( fork() ==0 ){ /* child process */
        close( listenfd );
        do_child( connfd );
        close(connfd);
        exit( 0 );
    }
    
    close( connfd );
    return 0;
}
int main( int argc, char** argv ){
    fd_set allset;
    fd_set rset;
    int    res;
    
    if( argc !=2 )
        return -1;
    
    
    listenfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    memset( &serv_addr, 0, sizeof( serv_addr ) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons( atoi( argv[1] ) );
    serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
    
    res = 1;
    setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &res, sizeof( int ) );
    

    if(bind( listenfd, ( struct sockaddr* )&serv_addr, sizeof( serv_addr ) ) <0 )
        sys_error( "bind failed" );
    
    listen( listenfd, 5 );
    signal( SIGCHLD, do_signal );
    
    
    FD_ZERO( &allset );
    FD_SET( listenfd, &allset );
    
    fcntl( listenfd, F_SETFL, fcntl(listenfd, F_GETFL, 0) | O_NONBLOCK );
    
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
recon:
            if( ( connfd = accept( listenfd,NULL,NULL ) )<0 )
		if( errno = EINTR )
                    goto recon; // already set nonblock I/O ,so this is needless
		else 
                    perror( "accept failed\n" );
            fcntl( connfd, F_SETFL, fcntl( connfd, F_GETFL, 0) & ~O_NONBLOCK );
            
            child_make(connfd);
            
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
void lock_init(int* semid ){
    
}
void lock_get( int* semid ){
    
}
void lock_put( int* semid ){
    
}
