/*
 * client.cc
 *
 * Copyright (c) 2007-2011 by NetBox, Inc.
 *
 * 2011-09-11 created by wangch
 *
 */


#include "nbl_log.h"
#include "nbl_net.h"
#include "nbl_selector.h"
#include "nbl_conn.h"

namespace nbcl
{
	class nbl_client
	{
		nbl_serve_mux* h_;
		nbl_conn<nbl_client>* c_;
	public:

		nbl_client() : 
			c_(0)
		{
			nbl_trace(("nbl_client()"));
			h_ = new nbl_serve_mux;
		}

		~nbl_client()
		{
			nbl_trace(("~nbl_client()"));
			if (c_) {
				c_->unserve();
			}
			delete c_;
			delete h_;
		}

		nbl_serve_handler* handler()
		{
			return h_;
		}

		int connect(string& host, int port)
		{
			nbl_trace(("nbl_connect::connect()"));

			nbl_conn<nbl_client>* c = new nbl_conn<nbl_client>;
			c->peer_.host_ = host;
			char serv[NI_MAXSERV];
			sprintf(serv, "%d", port);
			c->peer_.serv_ = serv;
			c->s_ = this;

			nbl_socket s = c->connect(host, port);
			if (s == -1) {
				c_ = 0;
				delete c;
				nbl_error_r((CRIT, "connect error in nbl_conncet()"), -1);
			}
			nbl_reader_writer rw(s);
			c->rw_ = rw;
			c_ = c;

			nbl_debug((INFO, "%s:%s", host.c_str(), serv));

			c->serve();

			return nbl_selector::instance()->run();
		}

		int write_request(nbl_request* r)
		{
			nbl_trace(("nbl_client::write_request()"));
			return c_->write_request(r);
		}

		int handle(int type, nbl_serve_handler* sh)
		{
			nbl_trace(("nbl_client::handle()"));
			return h_->handle(type, sh);
		}
	};

} //namespace nbcl

