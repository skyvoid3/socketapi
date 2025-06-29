#ifndef NETUTILS_H
#define NETUTILS_H

#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_MSG 256
#define MYIP "127.0.0.1"
#define AWAITING_NAME 1
#define READY 0

/*
 * Sets AI_PASSIVE flag to bind to all available local IP addresses.
 *
 * Parameters:
 *   port - The service port as a string (e.g., "8080").
 *
 * Returns:
 *   On success: Pointer to a struct addrinfo.
 *   On failure: NULL is returned, and an error message is printed to stderr.
 */
struct addrinfo *resolve_server_host(const char *port);

/*
 * Used for connect() to a TCP server
 *
 * On failure: returns NULL and prints error to stderr
 *
 * On success: returns pointer to a struct addrinfo
 */
struct addrinfo *resolve_client_host(const char *host, uint16_t port);

/*
 * Tries to create a listening sockfd using socket(), listen(), set SO_REUSEADDR
 * and bind()
 *
 * over every addrinfo returned by gai
 *
 * Returns -1 on failure
 */
int create_server_socket(struct addrinfo *res);

//


// Client information
typedef struct {
	char           name[32];
	int state;
} client_t;

// accept_connections() fills out this struct
typedef struct {
	struct sockaddr_storage addr;
	client_t                client;
	socklen_t               addr_len;
} client_info_t;

// Information about connection and client
typedef struct {
	client_info_t client_info;
	struct pollfd pfds; // Socket fd and events for poll()
} connection_info_t;

/* Tries to accept() connections
 *
 * Success: returns struct with all clients info (see accept_return_t)
 *
 * Failure: accept_fd == -1
 */
bool accept_connections(int sockfd, int *conn_count, int *conn_size,
                        connection_info_t **conn_info);

/* Calls socket() and connect()
 *
 * If no connection was made program exits
 *
 * Returns sockfd on success
 *
 * Parameters:
 *
 * struct addrinfo res
 *
 * uns shrt amount of tries
 *
 * uns shrt delay between tries in seconds
 */
int retry_connect(struct addrinfo *res, unsigned short tries,
                  unsigned short delay);

/* Gets peer name of ipv4 and ipv6 addresses
 *
 * Prints error message (perror) on failure
 */
void print_peer_name(int sockfd);

/* Pass in the sockfd and port variable to get the port number
 *
 * Returns -1 on err
 *
 */
int get_port(int sockfd, uint16_t *port);

// Send messages
ssize_t send_message(int sockfd);

// Receive messages
ssize_t recv_message(int sockfd, char *buf);

// Returns sin_addr or sin6_addr based for Ipv4 or Ipv6 respectively
void *get_in_addr(struct sockaddr *sa);

/* Adds a new client fd to poll()-ing process
 *
 * If the fd_size == fd_count resizes it by multiplying its capacity
 */
void add_to_pfds(connection_info_t **conn_info, int newfd, int *conn_count,
                 int *conn_size);

// Delete client fd from poll()-ing when client disconnects
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count);

/* Receives messages from clients and sends it to everybody else
 *
 * The pfd_i parameter is the index number of socket inside pfds array
 *
 * as determined in the poll() loop inside process_connections function
 */
bool handle_client_data(int sockfd, int *fd_count, int *pfd_i,
                        connection_info_t *conn_info);

/* Processes every client socket in pfds array.
 *
 * If the socket is the servers listening socket - accepts new connections
 */
void process_connections(int sockfd, int *conn_count, int *conn_size,
                         connection_info_t **conn_info);

// send() but cooler
int sendall(int s, char *buf, int *len);

/* Request client for name when client connects to server and state == AWAITING_NAME
 *
* On success sets state == READY and cleint.name now holds the clients name
*
* on failure returns false, deletes connection
*
* THIS FUNCTION HAS A FLAW. THE RECV CALL INSIDE BLOCKING. PLANNING TO FIX IT IN NEXT PATCH
*/
bool request_name(connection_info_t *conn_info, int pfd_i, int *conn_count);


/* Sets up a new connection and client info to conn list.  
 *
 * All poll, client and connection info is stored inside connection_info_t array
 *
 * THIS FUNCTION HAS A FLAW. IF REALLOCATION FAILS THE SERVER WILL SHUT DOWN.
*/
void add_connection(connection_info_t **conn_info, int newfd, int *conn_count,
                    int *conn_size, struct sockaddr_storage addr,
                    socklen_t addrlen);

/* Deletes all connection info when user disconnects
*/
void del_connection(connection_info_t conn_info[], int i, int *conn_count);


/*inet_ntop() but better ;)
* */
const char *inet_ntop2(void *addr, char *buf, size_t size);

// Cleanup in case of a signal
void cleanup(connection_info_t *conn_info, int conn_count);


#endif
