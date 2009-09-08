#ifndef __R2M_CONFIG_H
#define __R2M_CONFIG_H

void parse_config();

struct r2m_smtp_t {
	char *host;
	char *login;
	char *password;
};

struct r2m_config_t {
	struct r2m_smtp_t *smtp;
	char *email;
};

struct r2m_config_t *r2m_config;

#endif /* __R2M_CONFIG_H */
