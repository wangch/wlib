/*
 * w_reqres.h
 */

#ifndef W_REQRES_H_
#define W_REQRES_H_

namespace wlib {

	struct w_rr_header {
		int isreq_; // if 1, the body is req; if 0, the body is respond;
		int req_type_; // 
		int req_id_; // if respond, req_id_ is reply the request id;
		int body_len_; // the body length, without the header 
		int compress_type_;
		int crypto_type_;
	};


	enum {
		HEADER_LENGTH = sizeof(w_rr_header),
	};

	struct w_request {
		w_rr_header h_;
		char* body_;
	};

	struct w_respond {
		w_rr_header h_;
		char* body_;
	};

}

#endif // W_REQRES_H_
