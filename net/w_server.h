/*
 * w_server.h
 */



#ifndef W_SERVER_H_
#define W_SERVER_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <map>
#include "../utils/w_log.h"
#include "w_net.h"
#include "w_selector.h"
#include "w_conn.h"

namespace wlib {
	using namespace std;

	class w_server : public w_sock_handler {
		int port_;
		w_mutex m_;
		w_serve_mux* sh_;
		map<peer, w_conn<w_server>*> conns_;
		typedef map<peer, w_conn<w_server>*>::iterator IT;

		virtual int accept_handler(w_socket s, struct sockaddr_storage peer_addr) {
			w_trace(("w_server::accept_handler()"));
			// create a conn for serve the client 
			w_conn<w_server>* c = new w_conn<w_server>;
			c->s_ = this;
			w_reader_writer rw(s);
			c->rw_ = rw;

			socklen_t addr_len = sizeof(struct sockaddr_storage);
			char host[NI_MAXHOST], serv[NI_MAXSERV];

			int r = getnameinfo((struct sockaddr*)&peer_addr, addr_len, 
					host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICSERV);

			if (r != 0) {
				w_error((WARN, "getnameinfo error."));
			} else {
				c->peer_.host_ = host;
				c->peer_.serv_ = serv;
			}	

			w_debug((INFO, "new conn from %s:%s", host, serv));

			pair<peer, w_conn<w_server>*> p(c->peer_, c);
         {
			   w_lock l(m_);
			   conns_.insert(p);
         }

			return c->serve();
		}

	public:
		w_server(int port) : port_(port) {
			w_trace(("w_server::w_server()"));
			sh_ = new w_serve_mux; // default serve handler
		}

		virtual ~w_server() {
			w_trace(("w_server::~w_server()"));
			delete sh_;
			w_lock l(m_);
			IT it = conns_.begin();
			for (; it != conns_.end(); ++it) {
				delete it->second;
			}
		}

		// Serve Accept incomming connections on the listening socket and
		// creating a thread service for each client
		int serve(bool joinable = false) {
			w_trace(("w_server::serve()"));
			w_socket r = listen(port_, true);
			if (r == -1) {
				w_error_r((WARN, "listen error in w_server::serve()"), -1);
			}

			// now we use select modle for asynchronous commucation.
			// maybe use the other modle like epoll or complete port
			w_selector::instance()->handle(this, r);

			// a thread loop select and asyn call handler
			return w_selector::instance()->run(joinable);
		}

		// get handler
		w_serve_handler* handler() {
			return sh_; 
		}

		// send a request to remote client
		int write_request(string& host, int port, w_request* r) {
			w_trace(("w_server::write_request()"));
			char serv[NI_MAXSERV];
			sprintf(serv, "%s", port);
			peer p;
			p.host_ = host;
			p.serv_ = serv;
			
			w_conn<w_server>* c = 0;
			{ // lock the conns_
				w_lock l(m_);
				IT it = conns_.find(p);
				if (it == conns_.end()) {
					w_error_r((CRIT, "the peer %s:%s no connection", host.c_str(), serv), -1);
				}
				c = it->second;
			}
			return c->write_request(r);
		}

		int handle(int type, w_serve_handler* sh) {
			w_trace(("w_server::handle()"));

			return sh_->handle(type, sh);
		}

      int error_handler(peer& p) { 
         w_trace(("w_server::error_handler()"));
         
         IT it = conns_.find(p);
         conns_.erase(it);
         return 0; 
      }
	};

} //namespace wlib

#endif // W_SERVER_H_
