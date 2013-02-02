/*
 * w_conn.h
 */


#ifndef W_CONN_H_
#define W_CONN_H_

#include "../utils/w_thread.h"
#include "w_net.h"
#include "w_selector.h"


namespace wlib {

	struct peer {
		string host_;
		string serv_;

		bool operator==(const peer& p) {
			return host_ == p.host_ ? serv_ == p.serv_ : false;
		}

		bool operator<(const peer& p) {
			return host_ < p.host_ ? true : serv_ < p.serv_;
		}
	};

	struct w_respond_writer {
		w_reader_writer rw_;

		w_respond_writer() {
		}

		explicit w_respond_writer(w_reader_writer rw) : rw_(rw) {
			w_trace(("w_respond_writer::w_respond_writer()"));
		}

		int write(w_respond* res) {
			w_trace(("w_respond_writer::write()"));
			int n = rw_.write((char*)&(res->h_), HEADER_LENGTH);
			if (n != HEADER_LENGTH) {
				w_error_r((CRIT, "write header error in w_respond_writer::write()"), -1);
			}
			n = rw_.write(res->body_, res->h_.body_len_);
			if (n != res->h_.body_len_) {
				w_error_r((CRIT, "write data error in w_respond_writer::write()"), -1);
			}
			return 0;
		}
	};

	class w_serve_handler {
		public:
			virtual ~w_serve_handler() {
			}

			virtual int serve_request(w_respond_writer w, w_request* r) = 0;
			virtual int serve_respond(w_respond* r) = 0;
	};

	bool operator<(const peer& p1, const peer& p2) {
		return p1.host_ < p2.host_ ? true : p1.serv_ < p2.serv_;
	}

	class w_serve_mux: public w_serve_handler {
		typedef int handler_key;
		map<handler_key, w_serve_handler*> handler_map_; 
		typedef map<handler_key, w_serve_handler*>::iterator IT; 
	
   public:
		virtual ~w_serve_mux() {
			w_trace(("w_serve_mux::~w_serve_mux()"));
		}

		// Serve according to the request type search the handler 
		virtual int serve_request(w_respond_writer w, w_request* req) {
			w_trace(("w_serve_mux::serve_request()"));

         handler_key key = req->h_.req_type_;

			IT it = handler_map_.find(key);
			if (it == handler_map_.end()) {
				w_error_r((CRIT, "no match the request handler function"), -1);
			}
			return it->second->serve_request(w, req);
		}

		virtual int serve_respond(w_respond* r) {
			w_trace(("w_serve_mux::serve_respond()"));
			handler_key key = r->h_.req_type_;

			IT it = handler_map_.find(key);
			if (it == handler_map_.end()) {
				w_error_r((CRIT, "no match the respond handler function"), -1);
			}
			return it->second->serve_respond(r);
		}

		// register the handler function 
		virtual int handle(int req_type, w_serve_handler* sh) {
			w_trace(("w_serve_mux::handle()"));
			handler_map_[req_type] = sh;
			return 0;
		}
	};

	template<class T>
		struct w_conn : w_sock_handler {
		w_reader_writer rw_;
      peer peer_;
		T* s_;

		virtual int error_handler(w_socket s) {
			w_trace(("w_conn::error_handler()"));
         unserve();
         close(s);
         return s_->error_handler(peer_);
		}

		virtual int read_handler(w_socket s) {
			w_trace(("w_conn::read_handler()"));
			// first read header from rw_;
			w_rr_header header;
			int n = rw_.read((char*)&header, HEADER_LENGTH);
			if (n == 0) { // the peer orderly shutdown
				return -1; // TODO should wait the writing completed
			}
			if (n != HEADER_LENGTH) {
				w_error_r((CRIT, "read header error in w_conn::read_handler()"), -1);
			}

			// read the body
			int bodylen = header.body_len_;
			char* body = new char[bodylen];
			w_buffer_deleter dl(body);

			n = rw_.read(body, bodylen);
			if (n != bodylen) {
				w_error_r((CRIT, "read body error in w_conn::read_handler()"), -1);
			}

			if (header.isreq_ == 0) { // is a respond
				w_respond res;
				res.h_ = header;
				res.body_ = body;
				return s_->handler()->serve_respond(&res);
			}

			w_request req;
			req.h_ = header;
			req.body_ = body;

			w_respond_writer w(rw_);
			{
				w.p_ = peer_;
				return s_->handler()->serve_request(w, &req);
			}
		}

		w_conn() {
			w_trace(("w_conn::w_conn()"));
		}

		virtual ~w_conn() {
			w_trace(("w_conn::~w_conn()"));
		}

		int serve() {
			w_trace(("w_conn::serve()"));
			return w_selector::instance()->handle(this, rw_.fd());
		}

		int unserve() {
			w_trace(("w_conn::unserve()"));
			return w_selector::instance()->unhandle(rw_.fd());
		}

		int write_request(w_request* r) {
			w_trace(("w_conn::write_request()"));

			int bodylen = r->h_.body_len_;
			int n = rw_.write((char*)&(r->h_), HEADER_LENGTH);
			if (n != HEADER_LENGTH) {
				w_error_r((CRIT, "write header error in w_conn::write_request()"), -1);
			}
			n = rw_.write(r->body_, bodylen);
			if (n != bodylen) {
				w_error_r((CRIT, "write body error in w_conn::write_request()"), -1);
			}
			return 0;
		}
	};

} //namespace wlib

#endif // W_LOCK_H_
