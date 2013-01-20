/*
 * nbl_net.h
 *
 * Copyright (c) 2007-2011 by NetBox, Inc.
 *
 * 2011-09-09 created by wangch
 *
 */

#include "nbl_thread.h"
#include "nbl_net.h"
#include "nbl_selector.h"


namespace nbcl
{
	struct peer
	{
		string host_;
		string serv_;
		bool operator==(const peer& p)
		{
			return host_ == p.host_ ? serv_ == p.serv_ : false;
		}

		bool operator<(const peer& p)
		{
			return host_ < p.host_ ? true : serv_ < p.serv_;
		}
	};

	struct nbl_respond_writer
	{
		nbl_reader_writer rw_;
		peer p_;
		nbl_respond_writer()
		{
		}
		explicit nbl_respond_writer(nbl_reader_writer rw) :
			rw_(rw)
		{
			nbl_trace(("nbl_respond_writer::nbl_respond_writer()"));
		}
		int write(nbl_respond* res)
		{
			nbl_trace(("nbl_respond_writer::write()"));
			int n = rw_.write((char*)&(res->h_), HEADER_LENGTH);
			if (n != HEADER_LENGTH) {
				nbl_error_r((CRIT, "write header error in nbl_respond_writer::write()"), -1);
			}
			n = rw_.write(res->body_, res->h_.body_len_);
			if (n != res->h_.body_len_) {
				nbl_error_r((CRIT, "write data error in nbl_respond_writer::write()"), -1);
			}
			return 0;
		}
	};

	class nbl_serve_handler
	{
		public:
			virtual ~nbl_serve_handler()
			{
			}
			virtual int serve_request(nbl_respond_writer w, nbl_request* r) = 0;
			virtual int serve_respond(nbl_respond* r) = 0;
	};

	bool operator<(const peer& p1, const peer& p2)
	{
		return p1.host_ < p2.host_ ? true : p1.serv_ < p2.serv_;
	}


	class nbl_serve_mux: public nbl_serve_handler
	{
		typedef int handler_key;
		mutex m_;
		map<handler_key, nbl_serve_handler*> handler_map_; 
		typedef map<handler_key, nbl_serve_handler*>::iterator IT; 
	public:
		virtual ~nbl_serve_mux()
		{
			nbl_trace(("nbl_serve_mux::~nbl_serve_mux()"));
		}
		// Serve according to the request type search the handler 
		virtual int serve_request(nbl_respond_writer w, nbl_request* req)	
		{
			nbl_trace(("nbl_serve_mux::serve_request()"));
			lock l(m_);
			handler_key key = req->h_.req_type_;

			IT it = handler_map_.find(key);
			if (it == handler_map_.end()) {
				nbl_error_r((CRIT, "no match the request handler function"), -1);
			}
			return it->second->serve_request(w, req);
		}
		virtual int serve_respond(nbl_respond* r)
		{
			nbl_trace(("nbl_serve_mux::serve_respond()"));
			lock l(m_);
			handler_key key = r->h_.req_type_;

			IT it = handler_map_.find(key);
			if (it == handler_map_.end()) {
				nbl_error_r((CRIT, "no match the respond handler function"), -1);
			}
			return it->second->serve_respond(r);
		}
		// register the handler function 
		virtual int handle(int req_type, nbl_serve_handler* sh)
		{
			nbl_trace(("nbl_serve_mux::handle()"));
			lock l(m_);
			handler_map_[req_type] = sh;
			return 0;
		}
	};

	template<class T>
		struct nbl_conn : nbl_sock_handler
	{
		peer peer_; // net work addess of remote side
		nbl_reader_writer rw_;
		mutex m_;
		T* s_;
		bool closed_;

		virtual int error_handler(nbl_socket s)
		{
			closed_ = true;
			return 0;
		}
		virtual int read_handler(nbl_socket s)
		{
			nbl_trace(("nbl_conn::read_handler()"));
			// first read header from rw_;
			nbl_rr_header header;
			int n = rw_.read((char*)&header, HEADER_LENGTH);
			if (n == 0) { // the peer orderly shutdown
				return -1; // TODO should wait the writing completed
			}
			if (n != HEADER_LENGTH) {
				nbl_error_r((CRIT, "read header error in nbl_conn::read_handler()"), -1);
			}

			// read the body
			int bodylen = header.body_len_;
			char* body = new char[bodylen];
			nbl_buffer_deleter dl(body);

			n = rw_.read(body, bodylen);
			if (n != bodylen) {
				nbl_error_r((CRIT, "read body error in nbl_conn::read_handler()"), -1);
			}

			if (header.isreq_ == 0) { // is a respond
				nbl_respond res;
				res.h_ = header;
				res.body_ = body;
				return s_->handler()->serve_respond(&res);
			}

			nbl_request req;
			req.h_ = header;
			req.body_ = body;

			nbl_respond_writer w(rw_);
			{
				w.p_ = peer_;
				lock l(m_); // lock the map
				return s_->handler()->serve_request(w, &req);
			}
		}

		nbl_conn() :
			closed_(true)
		{
			nbl_trace(("nbl_conn::nbl_conn()"));
		}
		virtual ~nbl_conn()
		{
			nbl_trace(("nbl_conn::~nbl_conn()"));
		}
		int serve()
		{
			nbl_trace(("nbl_conn::serve()"));
			closed_ = false;
			return nbl_selector::instance()->handle(this, rw_.fd());
		}
		int unserve()
		{
			nbl_trace(("nbl_conn::unserve()"));
			return nbl_selector::instance()->unhandle(rw_.fd());
		}
		int write_request(nbl_request* r)
		{
			nbl_trace(("nbl_conn::write_request()"));
			lock l(m_); // lock the write
			int bodylen = r->h_.body_len_;
			int n = rw_.write((char*)&(r->h_), HEADER_LENGTH);
			if (n != HEADER_LENGTH) {
				nbl_error_r((CRIT, "write header error in nbl_conn::write_request()"), -1);
			}
			n = rw_.write(r->body_, bodylen);
			if (n != bodylen) {
				nbl_error_r((CRIT, "write body error in nbl_conn::write_request()"), -1);
			}
			return 0;
		}
	};

} //namespace nbcl
