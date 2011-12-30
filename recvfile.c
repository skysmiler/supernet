#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>

#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>


void sys_quit( const char* msg ){
    perror( msg );
    exit( -1 );
}

int recvfd( int sockfd ){
    
    int res;
    struct msghdr msg;
    struct iovec  iov[1];
    char c;
    

    union{
        struct cmsghdr cm;
        char   control[CMSG_SPACE( sizeof( int ) )];
    }control_un;
    struct cmsghdr  *cmptr;
    
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof( control_un.control );
    
    iov[0].iov_base = &c;
    iov[0].iov_len  = 1;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    if( (res = recvmsg(sockfd, &msg, 0) ) <=0 )
        sys_quit( "recvmsg -1" );
    
    if( (cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
        cmptr->cmsg_len == CMSG_LEN( sizeof(int) )
        ){
        if( cmptr->cmsg_level != SOL_SOCKET ){
            perror( "cmsg_level error" );
            return -1;
        }
        if( cmptr->cmsg_type  != SCM_RIGHTS ){
            perror( "cmsg_type error" );
            return -1;
        }
        return *( int* )CMSG_DATA( cmptr);
        
    }
    
    return -1;
    
}

int unix_connect( const char* filename ){
    int sockfd;
    struct sockaddr_un unaddr;

    if( ( sockfd = socket( AF_LOCAL, SOCK_STREAM, 0) ) <0 )
        return -1;
    
    memset( &unaddr, 0, sizeof( unaddr ) );
    unaddr.sun_family = AF_LOCAL;
    strncpy( unaddr.sun_path, filename, sizeof( unaddr.sun_path )-1 );
    
    if( connect( sockfd, ( struct sockaddr* )&unaddr, SUN_LEN( &unaddr ) )  <0 )
        sys_quit( "connect failed" );
        
    return sockfd;
    
}

int main( int argc, char** argv ){
    int fd, sockfd;
    char buf[ 128 ]={0};
    
    
    
    if( argc != 2 )
        return -1;
    
    sockfd = unix_connect( argv[1] );
    
    fd = recvfd( sockfd );
    printf( "recv fd:%d ok\n", fd );
    
    
    read(fd, buf, sizeof( buf ) );
    write( STDOUT_FILENO, buf, strlen( buf ) );
    

    close( sockfd );
    close( fd );
    

    return 0;
    
}
