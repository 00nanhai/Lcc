#ifndef _RESPONSE_H_
#define _RESPONSE_H_

void response(struct evhttp_request *req,
			  struct evbuffer *buf,
			  map<string,string> &params,
			  int opt_code);
bool init_response();
#endif
