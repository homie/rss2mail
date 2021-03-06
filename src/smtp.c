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

#include "r2m_config.h"

#define MAX_BUF 1024

/* XXX: split into other module */
char *base64(const unsigned char *input, int length)
{
	BIO *bmem, *b64;
	BUF_MEM *bptr;

	b64 = BIO_new(BIO_f_base64());
	bmem = BIO_new(BIO_s_mem());
	b64 = BIO_push(b64, bmem);
	BIO_write(b64, input, length);
	BIO_flush(b64);
	BIO_get_mem_ptr(b64, &bptr);

	char *buff = (char *)malloc(bptr->length);
	memcpy(buff, bptr->data, bptr->length-1);
	buff[bptr->length-1] = 0;

	BIO_free_all(b64);

	return buff;
}

int tcp_connect(const char *server)
{
        const char *PORT = "465";
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

void send_mail(const char *host, const char *login, const char *password)
{
	int sockfd, ret, err;
	char buffer[MAX_BUF + 1];
	char *cmd, *tmp;
	SSL_CTX *ctx;
	SSL *ssl;
	BIO *sbio;

	SSL_library_init();
	SSL_load_error_strings();

	ctx = SSL_CTX_new(SSLv23_method());

	sockfd = tcp_connect(host);

	ssl = SSL_new(ctx);
	sbio = BIO_new_socket(sockfd, BIO_NOCLOSE);
	SSL_set_bio(ssl, sbio, sbio);

	if ((err = SSL_connect(ssl)) <= 0) {
		printf("fuck: %s %d\n", ERR_error_string(err), SSL_get_error(ssl, err));
		exit(0);
	}

	/* XXX: split ssl_read into function as well */
	SSL_read(ssl, buffer, MAX_BUF);
	printf("buffer = \n----\n%s\n----\n", buffer);

	smtp_ssl_command(ssl, "EHLO\r\n", buffer);
	printf("buffer = \n----\n%s\n----\n", buffer);

	smtp_ssl_command(ssl, "AUTH LOGIN\r\n", buffer);
	printf("auth = \n----\n%s\n----\n", buffer);

	tmp = base64(login, strlen(login));
	cmd = (char *)malloc(strlen(tmp) + 3);

	strcpy(cmd, tmp);
	strcat(cmd, "\r\n");
	smtp_ssl_command(ssl, cmd, buffer);
	printf("auth = \n!!!!\n%s\n!!!!\n", buffer);
	
	free(tmp);
	free(cmd);

	tmp = base64(password, strlen(password));
	cmd = (char *)malloc(strlen(tmp) + 3);
	strcpy(cmd, tmp);
	strcat(cmd, "\r\n");

	smtp_ssl_command(ssl, cmd, buffer);
	printf("auth = \n???\n%s\n??????????\n", buffer);

	SSL_free(ssl);
	SSL_CTX_free(ctx);
	close(sockfd);
}

int main()
{
	char *auth_string;
	char *login;
	char *password;	

	parse_config();

	login = r2m_config->smtp->login;
	password = r2m_config->smtp->password;
	auth_string = (char *)malloc(strlen(login) + strlen(password) + 2);
	auth_string[0] = '\0';
	strcpy(auth_string + 1, login);
	strcpy(auth_string + strlen(login) + 2, password);

	printf("login: %s\n", r2m_config->smtp->login);
	send_mail(r2m_config->smtp->host, r2m_config->smtp->login,
			r2m_config->smtp->password);

	free(auth_string);
}
