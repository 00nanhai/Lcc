#ifndef _EVHTTP_OPT_H_
#define _EVHTTP_OPT_H_

#include <event.h>
#include <evhttp.h>

#define SW_HTTP_ERRNO				580
#define SW_HTTP_UNREADABLE_ERRNO	581
#define SW_HTTP_CREATE_BFILE_ERRNO	582
#define SW_HTTP_UNDEFILE_ERRNO		583
#define SW_HTTP_CLIENT_ERRNO		480

void evhttp_send_failinfo(struct evhttp_request *req,
						int error_num,
						char const *reason);


void _evhttp_send_file(struct evhttp_request *req,
					  struct evbuffer *buf,
					  int fd,
					  int size,
					  const char *type);


void evhttp_send_file(struct evhttp_request *req,
					  struct evbuffer *buf,
					  const char *path);


void evhttp_replyinfo(struct evhttp_request *req,
					  struct evbuffer *buf,
					  const char *replyinfo);

void evhttp_replay_bad_request(struct evhttp_request *req, 
							   struct evbuffer *buf, 
							   const char *badinfo, 
							   const char *decode_uri);

void evhttp_get_data(struct evhttp_request *req, 
					char **ip, 
					uint16_t *port, 
					char **decode_uri, 
					struct evbuffer **buf);

void evhttp_free_buf(struct evbuffer *buf);
void evhttp_base_dispath(struct event_base *evbase);
bool evhttp_init(struct event_base **evbase, struct evhttp **httpd);
#endif
