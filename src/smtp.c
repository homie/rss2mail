#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/ssl.h>

#define MAX_BUF 1024

int tcp_connect(const char *server)
{
        const char *PORT = "587";
        int err, sd;
        struct sockaddr_in sa;
        struct hostent *hp;

	char buffer[256];

        hp = gethostbyname(server);
        sd = socket (AF_INET, SOCK_STREAM, 0);

        memset (&sa, '\0', sizeof (sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons (atoi (PORT));
        //  inet_pton (AF_INET, SERVER, &sa.sin_addr);
        bcopy(hp->h_addr, &(sa.sin_addr.s_addr), hp->h_length);

        err = connect (sd, (struct sockaddr *) & sa, sizeof (sa));
        if (err < 0) {
                fprintf (stderr, "Connect error\n");
                exit (1);
        }
       
	err = read(sd, buffer, 255);
	printf("---\n%s\n---\n", buffer);

	smtp_plain_command(sd, "EHLO\r\n", buffer);
	printf("A buffer = \n====\n%s\n====\n", buffer);

	smtp_plain_command(sd, "STARTTLS\r\n", buffer);
	printf("A buffer = \n====\n%s\n====\n", buffer);

        return sd;
}

int smtp_plain_command(int sockfd, const char *command, char *buffer)
{
	int err;

	err = write(sockfd, command, strlen(command));
	/* XXX: check err */
	
	err = read(sockfd, buffer, MAX_BUF);

	return err;
}

int smtp_ssl_command(SSL *ssl, const char *command, char *buffer)
{
	int err;

	err = SSL_write(ssl, command, strlen(command));
	printf("err = %d\n", err);

	err = SSL_read(ssl, buffer, MAX_BUF);
	printf("read err = %d\n", err);

	return err;
}

void send_mail()
{
	int sockfd, ret, err;
	char buffer[MAX_BUF + 1];
	SSL_CTX *ctx;
	SSL *ssl;
	BIO *sbio;

	SSL_library_init();
	SSL_load_error_strings();

	ctx = SSL_CTX_new(SSLv23_method());

	sockfd = tcp_connect("smtp.gmail.com");

	printf("\n\nSSSSSSSSSSSSSSSSSSSSSSSSSSSS\n\n");

	ssl = SSL_new(ctx);
	sbio = BIO_new_socket(sockfd, BIO_NOCLOSE);
	SSL_set_bio(ssl, sbio, sbio);

	if ((err = SSL_connect(ssl)) <= 0) {
		printf("fuck: %s %d\n", ERR_error_string(err), SSL_get_error(ssl, err));
		exit(0);
	}

	smtp_ssl_command(ssl, "EHLO\r\n", buffer);
	printf("buffer = \n----\n%s\n----\n", buffer);

	/*
	err = SSL_write(ssl, "EHLO\r\n", strlen("EHLO\r\n"));
	printf ("SSL: err = %d\n", err);

	ret = SSL_read(ssl, buffer, MAX_BUF);
	printf("%s\n", buffer);
	
	err = SSL_write(ssl, "AUTH PLAIN\r\n", strlen("AUTH PLAIN\r\n"));
	printf ("SSL: err = %d\n", err);

	ret = SSL_read(ssl, buffer, MAX_BUF);
	printf("%s\n", buffer);*/

	SSL_free(ssl);
	SSL_CTX_free(ctx);
	close(sockfd);
}

int main()
{
	send_mail();
}
