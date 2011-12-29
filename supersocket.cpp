#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<unistd.h>
#include<errno.h>
#include<string.h>

#include<iostream>
#include<exception>

#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netdb.h>

class supersock{


public:
    supersock(int family = AF_INET, int type = SOCK_STREAM, int protol = IPPROTO_TCP ){

        if ( (_sockfd = socket( family, type, protol )) >0 )
            return ;
        else 
            _family = family;
        
    }
    virtual ~supersock(  ){
        
    }


public:
    int max_send_buffer();
    int max_recv_buffer();
    int getopt  (int type, int option );
    int getopts (int type, int option, void* val);
    int setopt  (int type, int option, int val);
    int setopts (int type, int option, void* val );
    
    int fd(  ){
        return _sockfd;
       
    }
    int family(  ){
        return _family;
        
    }
    
public:
    virtual int _connect(  ){
        
    }
    virtual int _assign_addr( struct sockaddr* inaddr, int len){
        memset(inaddr, 0, len );
        
        
    }

private:
    int _family;
    int _sockfd;
    struct sockaddr* _sockaddr;
    
};
    // implementation
int supersock::max_send_buffer(){
   
    return getopt( SOL_SOCKET, SO_SNDBUF );
    
}

int supersock::max_recv_buffer(){
        return getopt( SOL_SOCKET, SO_RCVBUF );
    
}
int supersock::getopt( int type , int option ){
    int val;
    socklen_t len = sizeof( val );
    if( getsockopt( _sockfd, type, option, &val, &len ) )
        return -1;
    return val;
    
}
int supersock::setopt( int type, int option, int val ){
   
    socklen_t len = sizeof( val );
    
    return setsockopt( _sockfd, type, option, &val, len );
   
}

int main(){
    supersock sock;
        printf("%d,%d\n", sock.max_send_buffer(), sock.max_recv_buffer());
    sock.setopt( SOL_SOCKET, SO_RCVBUF, 1025 );
    
    printf("%d,%d\n", sock.max_send_buffer(), sock.max_recv_buffer());

}

