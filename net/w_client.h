/*
 * w_client.h
 */


#ifndef W_CLIENT_H_
#define W_CLIENT_H_

#include "../util/w_log.h"
#include "w_net.h"
#include "w_selector.h"
#include "w_conn.h"

namespace wlib {

	class w_client {
		w_serve_mux* h_;
		w_conn<w_client>* c_;

	public:
		w_client() : c_(0) {
			w_trace(("w_client()"));
			h_ = new w_serve_mux;
		}

		~w_client() {
			w_trace(("~w_client()"));
			if (c_) {
				c_->unserve();
			}
			delete c_;
			delete h_;
		}

		w_serve_handler* handler() {
			return h_;
		}

		int connect(string& host, int port) {
			w_trace(("w_connect::connect()"));

			w_conn<w_client>* c = new w_conn<w_client>;
			c->peer_.host_ = host;
			char serv[NI_MAXSERV];
			sprintf(serv, "%d", port);
			c->peer_.serv_ = serv;
			c->s_ = this;

			w_socket s = c->connect(host, port);
			if (s == -1) {
				c_ = 0;
				delete c;
				w_error_r((CRIT, "connect error in w_conncet()"), -1);
			}
			w_reader_writer rw(s);
			c->rw_ = rw;
			c_ = c;

			w_debug((INFO, "%s:%s", host.c_str(), serv));

			c->serve();

			return w_selector::instance()->run();
		}

		int write_request(w_request* r) {
			w_trace(("w_client::write_request()"));
			return c_->write_request(r);
		}

		int handle(int type, w_serve_handler* sh) {
			w_trace(("w_client::handle()"));
			return h_->handle(type, sh);
		}

      int error_handler(peer& p) { 
         return 0; 
      }
   };

} //namespace wlib

#endif // W_CLIENT_H_
