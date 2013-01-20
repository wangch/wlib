/*
 * server.h
 *
 * Copyright (c) 2007-2011 by NetBox, Inc.
 *
 * 2011-09-11 created by wangch
 *
 */



#ifndef NBL_SERVER_H
#define NBL_SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <map>
#include "nbl_log.h"
#include "nbl_net.h"
#include "nbl_selector.h"
#include "nbl_conn.h"

namespace nbcl
{
	using namespace std;

	class nbl_server : public nbl_sock_handler
	{
		int port_;
		mutex m_;
		nbl_serve_mux* sh_;
		map<peer, nbl_conn<nbl_server>*> conns_;
		typedef map<peer, nbl_conn<nbl_server>*>::iterator IT;

		virtual int accept_handler(nbl_socket s, struct sockaddr_storage peer_addr)
		{
			nbl_trace(("nbl_server::accept_handler()"));
			// create a conn for serve the client 
			nbl_conn<nbl_server>* c = new nbl_conn<nbl_server>;
			c->s_ = this;
			nbl_reader_writer rw(s);
			c->rw_ = rw;

			socklen_t addr_len = sizeof(struct sockaddr_storage);
			char host[NI_MAXHOST], serv[NI_MAXSERV];

			int r = getnameinfo((struct sockaddr*)&peer_addr, addr_len, 
					host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICSERV);

			if (r != 0) {
				nbl_error((WARN, "getnameinfo error."));
			} else {
				c->peer_.host_ = host;
				c->peer_.serv_ = serv;
			}	

			nbl_debug((INFO, "new conn from %s:%s", host, serv));

			pair<peer, nbl_conn<nbl_server>*> p(c->peer_, c);
			{ // lock the conns_
				lock l(m_);
				IT it = conns_.begin();
				for (; it != conns_.end();) {
					if (it->second->closed_) {
						nbl_conn<nbl_server>* c = it->second;
						delete c;
						conns_.erase(it++);
					} else {
						it++;
					}
				}
				// add to clients list
				conns_.insert(p);
			}

			return c->serve();
		}

	public:
		nbl_server(int port) :
			port_(port)
		{
			nbl_trace(("nbl_server::nbl_server()"));
			sh_ = new nbl_serve_mux; // default serve handler
		}
		virtual ~nbl_server()
		{
			nbl_trace(("nbl_server::~nbl_server()"));
			delete sh_;
			lock l(m_);
			IT it = conns_.begin();
			for (; it != conns_.end(); ++it) {
				delete it->second;
			}
		}

		// Serve Accept incomming connections on the listening socket and
		// creating a thread service for each
		int serve(bool joinable = false) 
		{
			nbl_trace(("nbl_server::serve()"));
			nbl_socket r = listen(port_, true);
			if (r == -1) {
				nbl_error_r((WARN, "listen error in nbl_server::serve()"), -1);
			}

			// now we use select modle for asynchronous commucation.
			// maybe use the other modle like epoll or complete port
			nbl_selector::instance()->handle(this, r);

			//a thread loop select and asyn call handler
			return nbl_selector::instance()->run(joinable);
		}

		// get handler
		nbl_serve_handler* handler()
		{
			return sh_; 
		}

		// send a request to remote client
		int write_request(string& host, int port, nbl_request* r)
		{
			nbl_trace(("nbl_server::write_request()"));
			char serv[NI_MAXSERV];
			sprintf(serv, "%s", port);
			peer p;
			p.host_ = host;
			p.serv_ = serv;
			
			nbl_conn<nbl_server>* c = 0;
			{ // lock the conns_
				lock l(m_);
				IT it = conns_.find(p);
				if (it == conns_.end()) {
					nbl_error_r((CRIT, "the peer %s:%s no connection", host.c_str(), serv), -1);
				}
				c = it->second;
			}
			return c->write_request(r);
		}

		int handle(int type, nbl_serve_handler* sh)
		{
			nbl_trace(("nbl_server::handle()"));

			return sh_->handle(type, sh);
		}
	};

} //namespace nbcl

#endif //NBL_SERVER_H
