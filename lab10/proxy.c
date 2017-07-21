/*
 * proxy.c - CS:APP Web proxy
 *
 * TEAM MEMBERS:
 *     Name: Xinyue Wu
 *     Student Number: 515030910231
 *     Email: 971137346@qq.com
 *     
 * 
 * My naive but correct tiny proxy. Use multi-threads to accomplish concurrency.
 */ 

#include "csapp.h"

/*
 * Function prototypes
 */
int parse_uri(char *uri, char *target_addr, char *path, int  *port);
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);
void doit(int fd,struct sockaddr_in sockaddr);
void read_requesthdrs(rio_t *rp);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
ssize_t Rio_readlineb_w(rio_t *rp,void *usrbuf,size_t maxlen);
ssize_t Rio_readnb_w(rio_t *rp,void *usrbuf,size_t maxlen);
void Rio_writen_w(int fd,void *usrbuf,size_t n);
int open_clientfd_ts(char *hostname,int port);
void *thread(void *arg);

/*
 * Global variable declerations
 */
sem_t gethost_mutex;
sem_t log_mutex;
int logfile;

/*
 * self-define stuctures
 */
typedef struct{
	int connfd;
	struct sockaddr_in clientaddr;
}conn_info;

/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
	int listenfd;		/* file descriptor for listening */
	int connfd;			/* file descriptor for connection */
	int port;
	struct sockaddr_in clientaddr;
	int clientlen=sizeof(clientaddr);
	pthread_t tid;

    /* Check arguments */
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	exit(0);
    }

	/* initialize log file mutex */
	Sem_init(&log_mutex,0,1);
	/* open log file */
	logfile=Open("proxy.log",O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR);

	/* initialize gethostbyname mutex */
	Sem_init(&gethost_mutex,0,1);

	/* ignore SIGPIPE signals */
	Signal(SIGPIPE,SIG_IGN);

	/* open listen file descriptor */
	port = atoi(argv[1]);
	listenfd = Open_listenfd(port);


	while(1){
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *)&clientlen);
		conn_info *arg = (conn_info *)malloc(sizeof(conn_info));
		arg->connfd = connfd;
		arg->clientaddr = clientaddr;
		Pthread_create(&tid,0,thread,(void *)arg);
	}

    exit(0);
}

/*
 * thread - Thread routine
 */
void *thread(void *arg){
	conn_info *conn = (conn_info *)arg;
	Pthread_detach(pthread_self());
	int connfd = conn->connfd;
	struct sockaddr_in clientaddr = conn->clientaddr;
	free(arg);
	doit(connfd,clientaddr);
	Close(connfd);
	return NULL;
}

/*
 * doit - parse and handle request line
 *
 * Read request line and headers from client side. Only GET method
 * is allowed, others will send error message. Then parse the URI,
 * get data from the server side and send it to client side. Write
 * log file to record this connection.
 */
void doit(int fd,struct sockaddr_in sockaddr)
{
	char buf[MAXLINE];
	char method[MAXLINE];
	char uri[MAXLINE];
	char version[MAXLINE];
	char host[MAXLINE];
	char path[MAXLINE];
	int port;
	int svr_clientfd;
	int datasize = 0;
	rio_t conn_rio;
	rio_t svr_rio;

	/* Read request line and headers */
	Rio_readinitb(&conn_rio,fd);
	Rio_readlineb_w(&conn_rio,buf,MAXLINE);
	sscanf(buf,"%s %s %s",method,uri,version);
	if(strcasecmp(method,"GET")){	/* Cases that method isn't GET*/
		clienterror(fd,method,"501","Not Implemented",
				"Tiny does not implement this method");
		return;
	}

	/* Ignore request headers */
	read_requesthdrs(&conn_rio);

	/* Parse URI from GET request */
	if(parse_uri(uri,host,path,&port)<0){
		clienterror(fd,method,"400","Bad request",
				"Invalid request");
	}
	
	/* Connect server */
	svr_clientfd = open_clientfd_ts(host,port);
	Rio_readinitb(&svr_rio,svr_clientfd);

	/* build request line for server */
	sprintf(buf,"%s /%s HTTP/1.0\r\n",method,path);
	sprintf(buf,"%sHost: %s\r\n\r\n",buf,host);

	/* send request line to server */
	Rio_writen_w(svr_clientfd,buf,strlen(buf));

	/* send request headers */
	/*while(Rio_readlineb_w(&conn_rio,buf,MAXLINE)!=0){
		Rio_writen_w(svr_clientfd,buf,strlen(buf));
		if(strcmp(buf,"\r\n")==0){
			break;
		}
	}*/
	//Rio_writen_w(svr_clientfd,"\r\n\r\n",4);
	/* send file to client */
	//Rio_readinitb(&svr_rio,svr_clientfd);
	int n;
	while((n=Rio_readnb_w(&svr_rio,buf,MAXLINE))!=0){
		datasize+=n;
		printf("%d\n",n);
		Rio_writen_w(fd,buf,n);
	}

	/* close server */
	Close(svr_clientfd);

	printf("datasize:%d\n",datasize);
	
	/* write to log file */
	format_log_entry(buf,&sockaddr,uri,datasize);
	P(&log_mutex);
	Rio_writen(logfile,buf,strlen(buf));
	V(&log_mutex);
}

/*
 * clienterror - send error message to client
 *
 * Send HTTP response to client side and interpret the error to user
 */
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
	char buf[MAXLINE], body[MAXBUF];

	/* Build the HTTP response body */
	sprintf(body,"<html><title>Tiny Error</title>");
	sprintf(body,"%s<body bgcolor=""ffffff"">\r\n",body);
	sprintf(body,"%s%s: %s\r\n",body,errnum,shortmsg);
	sprintf(body,"%s<p>%s: %s\r\n",body,longmsg,cause);
	sprintf(body,"%s<hr><em>The Tiny Web server</em>\r\n",body);

	/* Print the HTTP response */
	sprintf(buf,"HTTP/1.0 %s %s\r\n",errnum,shortmsg);
	Rio_writen_w(fd,buf,strlen(buf));
	sprintf(buf,"Content-type: text/html\r\n");
	Rio_writen_w(fd,buf,strlen(buf));
	sprintf(buf,"Content-length: %d\r\n\r\n",(int)strlen(body));
	Rio_writen_w(fd,buf,strlen(buf));
	Rio_writen_w(fd,body,strlen(body));
}

/*
 * read_requesthdrs - read request headers and ignore it
 *
 */
void read_requesthdrs(rio_t *rp)
{
	char buf[MAXLINE];

	Rio_readlineb_w(rp,buf,MAXLINE);
	while(strcmp(buf,"\r\n")){
		Rio_readlineb_w(rp,buf,MAXLINE);
		printf("%s",buf);
	}
	return;
}


/*
 * Rio_readlineb_w - wrapper of rio_readlineb
 *
 * the function simply return after printing a warning message when I/O fails.
 */
ssize_t Rio_readlineb_w(rio_t *rp,void *usrbuf,size_t maxlen)
{
	ssize_t rc;
	if((rc=rio_readlineb(rp,usrbuf,maxlen))<0){
		fprintf(stderr,"Rio_readlineb error\n");
		return 0;
	}
	return rc;
}

/*
 * Rio_readnb_w - wrapper of rio_readnb
 *
 * the function simply return after printing a warning message when I/O fails.
 */
ssize_t Rio_readnb_w(rio_t *rp,void *usrbuf,size_t n)
{
	ssize_t rc;
	if((rc=rio_readnb(rp,usrbuf,n))<0){
		fprintf(stderr,"Rio_readnb error\n");
		return 0;
	}
	return rc;
}

/*
 * Rio_writen_w - wrapper of rio_writen
 *
 * the function simply return after printing a warining message when I/O fails.
 */
void Rio_writen_w(int fd,void *usrbuf,size_t n)
{
	if(rio_writen(fd,usrbuf,n)!=n){
		fprintf(stderr,"Rio_writen error\n");
	}
}

/*
 * open_clientfd_ts - a thread-safe version of open_clientfd
 *
 * The function uses the lock-and-copy technique when it calls gethostbyaddr.
 */
int open_clientfd_ts(char *hostname,int port)
{
	int clientfd;
	struct hostent *hp;
	struct sockaddr_in serveraddr;

	if((clientfd=socket(AF_INET,SOCK_STREAM,0))<0)
		return -1;	/* check errno for cause of error */

	/* Fill in the sever's IP address and port */
	P(&gethost_mutex);
	if((hp=gethostbyname(hostname))==NULL){
		V(&gethost_mutex);
		return -2;	/* check h_errno for cause of error*/
	}
	bzero((char *)&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family=AF_INET;
	bcopy((char *)hp->h_addr_list[0],
			(char *)&serveraddr.sin_addr,hp->h_length);
	V(&gethost_mutex);
	serveraddr.sin_port=htons(port);

	/* Establish a connection with the server */
	if(connect(clientfd,(SA *)&serveraddr,sizeof(serveraddr))<0){
		return -1;
	}
	return clientfd;
}

/*
 * parse_uri - URI parser
 * 
 * Given a URI from an HTTP proxy GET request (i.e., a URL), extract
 * the host name, path name, and port.  The memory for hostname and
 * pathname must already be allocated and should be at least MAXLINE
 * bytes. Return -1 if there are any problems.
 */
int parse_uri(char *uri, char *hostname, char *pathname, int *port)
{
    char *hostbegin;
    char *hostend;
    char *pathbegin;
    int len;

    if (strncasecmp(uri, "http://", 7) != 0) {
	hostname[0] = '\0';
	return -1;
    }
       
    /* Extract the host name */
    hostbegin = uri + 7;
    hostend = strpbrk(hostbegin, " :/\r\n\0");
    len = hostend - hostbegin;
    strncpy(hostname, hostbegin, len);
    hostname[len] = '\0';
    
    /* Extract the port number */
    *port = 80; /* default */
    if (*hostend == ':')   
	*port = atoi(hostend + 1);
    
    /* Extract the path */
    pathbegin = strchr(hostbegin, '/');
    if (pathbegin == NULL) {
	pathname[0] = '\0';
    }
    else {
	pathbegin++;	
	strcpy(pathname, pathbegin);
    }

    return 0;
}

/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
		      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d\n", time_str, a, b, c, d, uri,size);
}


