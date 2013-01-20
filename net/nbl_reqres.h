/*
 * reqres.h
 *
 * Copyright (c) 2007-2011 by NetBox, Inc.
 *
 * 2011-09-11 created by wangch
 *
 */

#ifndef NBX_REQRES_H_
#define NBX_REQRES_H_


namespace nbcl
{

	struct nbl_rr_header
	{
		int isreq_; // if 1, the body is req; if 0, the body is respond;
		int req_type_; // 
		int req_id_; // if respond, req_id_ is reply the request id;
		int body_len_; // the body length, without the header 
		int compress_type_;
		int crypto_type_;
	};


	enum
	{
		HEADER_LENGTH = sizeof(nbl_rr_header),
	};

	struct nbl_request
	{
		nbl_rr_header h_;
		char* body_;
	};


	struct nbl_respond
	{
		nbl_rr_header h_;
		char* body_;
	};

}

#endif //NBX_REQRES_H_
