int atoport(const char *service, const char *proto);
struct in_addr *atoaddr(const char *address);
int get_connection(int socket_type, short port, int *listener);
int make_connection(const char *service, int type, const char *netaddress);
int sock_read(int sockfd, char *buf, size_t count);
int sock_write(int sockfd, const char *buf, size_t count);
int sock_gets(int sockfd, char *str, size_t count);
int sock_puts(int sockfd, const char *str);
void ignore_pipe(void);

