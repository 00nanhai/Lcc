#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <boost/lexical_cast.hpp>

#include <event.h>
#include <evhttp.h>

#include "common.h"
#include "global.h"
#include "file_opt.h"
#include "evhttp_opt.h"
#include "request.h"

void evhttp_send_failinfo(struct evhttp_request *req,
						int error_num,
						char const *reason)
{
	evhttp_send_error(req, errno, reason);
}

void _evhttp_send_file(struct evhttp_request *req,
					  struct evbuffer *buf,
					  int fd,
					  int size,
					  const char *type)
{
	char slen[32] = {0};
	snprintf(slen, 32, "%d", size);
	evhttp_add_header(evhttp_request_get_output_headers(req),
					  "Content-Type", 
						type);	
	evhttp_add_header(evhttp_request_get_output_headers(req),
						"Content-Length", 
						slen);	
	evbuffer_add_file(buf, fd, 0, size);		
	evhttp_send_reply(req, 200, "OK", buf);
}

void evhttp_send_file(struct evhttp_request *req,
					  struct evbuffer *buf,
					  const char *path)
{
	
	if (!is_file_exist(path)) {
		evhttp_send_failinfo(req, 404, "Not Found.");
		return;
	}

	if (!is_file_readable(path)) {
		evhttp_send_failinfo(req, SW_HTTP_UNREADABLE_ERRNO, "Can't read.");
		return;
	}

	int fd = open(path, O_RDONLY);	
	if (fd < 0) {			
		Perror("open");
		Assert(fd > 0);
		//close(fd)
		return;
	}
		
	struct stat st;
	if (fstat(fd, &st) < 0) {			
		Perror("fstat");
		//close(fd);
		Assert(0);
		return;
	}
	
	const char *type = "application/octet-stream";
	_evhttp_send_file(req, buf, fd, st.st_size, type);
	//FIXME Don't close the fd. close fd at evbuffer_free.
	//close(fd);
}

void evhttp_replyinfo(struct evhttp_request *req,
					  struct evbuffer *buf,
					  const char *replyinfo)

{
	evhttp_add_header(req->output_headers, 
						"Content-Type", 
						"text/plain; charset=UTF-8");
	evhttp_add_header(req->output_headers, 
						"Author", 
						"Data-Frog.Inc 2012");
	evhttp_add_header(req->output_headers, 
						"Cache-Control",
						"no-cache");
	evbuffer_add_printf(buf, replyinfo);
	evhttp_send_reply(req, HTTP_OK, "OK", buf);
}

void evhttp_replay_bad_request(struct evhttp_request *req, 
							   struct evbuffer *buf, 
							   const char *badinfo, 
							   const char *decode_uri)
{
	evbuffer_add_printf(buf, badinfo);
	evbuffer_add_printf(buf, decode_uri);
	evhttp_send_reply(req, HTTP_BADREQUEST, "Bad Request", buf);
	//evbuffer_free(buf);
}

void evhttp_get_data(struct evhttp_request *req, 
					char **ip, 
					uint16_t *port, 
					char **decode_uri, 
					struct evbuffer **buf)
{
	*buf = evbuffer_new();
	evhttp_connection_get_peer(req->evcon, ip, port);
	*decode_uri = evhttp_decode_uri(evhttp_request_uri(req));
}

void evhttp_free_buf(struct evbuffer *buf)
{
	evbuffer_free(buf);
}

bool evhttp_init(struct event_base **evbase, struct evhttp **httpd)
{
	/* http选项 */
	int httpd_option_port = 
			boost::lexical_cast<int>(g_base_conf->get("listen").c_str());
	int httpd_option_timeout = 180;
	//FIXME old Api event_base_new()
	*evbase = event_init();
	if (*evbase == NULL) {
		goto END;
	}
	*httpd = evhttp_new(*evbase);
	if (*httpd == NULL) {
		goto END;
	}
	
	if (0 != evhttp_bind_socket(*httpd, "0.0.0.0", httpd_option_port)) {
        LOG_ERRO("evhttp_bind_socket on  %d failed.", httpd_option_port);
		goto END;
    }
	LOG_INFO("bind on %d done.", httpd_option_port);

	evhttp_set_cb(*httpd, "/lcc.php", lcc_request::location_cc_handler, NULL);
	evhttp_set_timeout(*httpd, httpd_option_timeout);
	return true;

END:
	if(*httpd) evhttp_free(*httpd);
	if(*evbase) event_base_free(*evbase);
	return false;
}

void evhttp_base_dispath(struct event_base *evbase)
{
	LOG_INFO("Libevent start dispatch.");
	event_base_dispatch(evbase);
}
