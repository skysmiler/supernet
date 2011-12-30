#include<arpa/inet.h>
#include<netdb.h>
#include<sys/un.h>
#include<sys/types.h>
#include<sys/socket.h>

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>


int sendfd( int sockfd, int fd ){
    
    int res;
    struct msghdr msg;
    struct iovec  iov[1];
    char  c;
    

    union{
        struct cmsghdr cm;
        char   control[CMSG_SPACE( sizeof( int ) )];
        
    }control_un;

    struct cmsghdr  *cmptr;
    
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
    
    cmptr = CMSG_FIRSTHDR( &msg );
    cmptr->cmsg_len = CMSG_LEN( sizeof( int ) );
    cmptr->cmsg_level = SOL_SOCKET;
    cmptr->cmsg_type  = SCM_RIGHTS;
    *( int* )CMSG_DATA( cmptr ) = fd;
    

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    iov[0].iov_base = &c;
    iov[0].iov_len = 1;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    
    res = sendmsg( sockfd, &msg, 0 );
    
    return res;
    
}

void sys_quit( const char *msg ){
    perror( msg );
    exit( -1 );
}
    /* openfd : unix_path filename */
int main(int argc, char** argv){
	
    int fd, sockfd, connfd, len;
    
    struct sockaddr_un unaddr, conaddr;
    
    if( argc !=3 )
        return -1;
    
    if( (fd = open( argv[2], O_CREAT|O_RDWR, 0644 )) <0 )
        sys_quit( "open file" );
    
    if( (sockfd = socket( AF_UNIX, SOCK_STREAM, 0 ) ) <0)
        sys_quit( "socket failed" );
    
    unlink( argv[1] );
    memset( &unaddr, 0, sizeof( unaddr ) );
    unaddr.sun_family =  AF_LOCAL;
    strncpy( unaddr.sun_path, argv[1], sizeof( unaddr.sun_path )-1 );;
    if (bind( sockfd, ( struct sockaddr* )&unaddr, sizeof(unaddr)) <0){
        perror( "bind" );
        return -1;
    }
    
    listen( sockfd, 5 );
    len = sizeof( conaddr );
    
    if( (connfd = accept( sockfd, ( struct sockaddr*)&conaddr, &len )) <0 ){
        perror( "accept" );
        return -1;
    }
    
    sendfd( connfd, fd );
    printf( "send fd:%d ok\n",fd );
        
    close( sockfd );
    
}
