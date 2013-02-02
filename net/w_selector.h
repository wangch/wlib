/*
 * w_lock.h
 *
 * Copyright (c) 2007 by NetBox, Inc.
 *
 * 2011-09-09 created by wangch
 *
 */

#ifndef W_SELETECTOR_H_
#define W_SELETECTOR_H_  

#include <map>
#include "../util/w_thread.h"
#include "../util/w_type.h"
#include "../util/w_singleton.h"
#include "../util/w_log.h"
#include "w_net.h"

namespace wlib {

	class selector {
		bool run_;

		w_thread<selector> thr_;
		std::map<w_socket, w_sock_handler*> map_;
		int ret_val_;

		int select() {
			w_trace(("selector::select()"));

			// TODO fdwrite and fdexcept support?
			fd_set fdread/*, fdwrite, fdexcept*/ ;
			struct timeval timeout;
			typedef std::map<w_socket, w_sock_handler*>::iterator IT;

			while(run_) {
				FD_ZERO(&fdread);
				//FD_ZERO(&fdwrite);
				//FD_ZERO(&fdexcept);

				if(map_.empty())
					break;

				IT it = map_.begin(); 
				int nfds = 0;
				for(; it != map_.end(); ++it) {
					FD_SET(it->first, &fdread);
					//FD_SET(it->first, &fdwrite);
					//FD_SET(it->first, &fdexcept);
					nfds = it->first;
				}
				timeout.tv_sec = 5;
				timeout.tv_usec = 0;

				nfds += 1; // 

				int rc = ::select(nfds, &fdread, NULL/*&fdwrite*/, NULL/*&fdexcept*/, &timeout);
				if(rc == -1)
					w_error_r((CRIT, "select() error"), -1);
				else if(rc == 0) { // timeout
					//w_debug((INFO, "select() timeout"));
					continue;
				}

				for(it = map_.begin(); it != map_.end(); ++it) {
					w_socket s = it->first;
					w_sock_handler* h = it->second;
					int e = 0;
					if(FD_ISSET(s, &fdread)) {
						if(it->second->listening()) { // tcp listrening and accept remote peer connect
							struct sockaddr_storage addr;
							socklen_t addrlen = sizeof(addr);
							w_socket cs = ::accept(s, (struct sockaddr*)&addr, &addrlen);
							if(cs == -1) {
								w_error((WARN, "accept() error."));
								close(cs);
							} else {
								e = h->accept_handler(cs, addr);
								break;
							}
						} else { // read
							e = h->read_handler(s);
						}
					}
					/*
					if(FD_ISSET(s, &fdwrite)) {
						e = h->write_handler(s);
					}
					if(FD_ISSET(s, &fdexcept)) {
						w_error((WARN, "fd except."));
						e = h->except_handler(s);
					}
					*/
					if (e == -1) {
						h->error_handler(s);
					}
				} // for
			} //while

#if defined(WINDOWS)
			WSACleanup();
#endif
			return 0;
		}

	public:
		selector() : run_(true) {
			w_trace(("selector::selector()"));
		}

		~selector() {
			w_trace(("selector::~selector()"));
			run_ = false;
			//for_each(map_.begin(), map_.end(), w_deleter());
		}

		// the threadfunc should call exce
		int thr_exec() {
			w_trace(("selector::exec()"));
#if defined(_WINDOWS)
			WSADATA wsd;
			if(WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
				w_error_r((CRIT, "unable to load Winsock!"), -1);
#endif
			return this->select();
		}

		// create a new thread and call exec
		int run(bool joinable = false) {
			w_trace(("selector::run()"));
			return thr_.run(this, joinable);
		}

		// register the sock handler 
		int handle(w_sock_handler* h, w_socket s) {
			w_trace(("selector::handle()"));
			
			map_.insert(pair<w_socket, w_sock_handler*>(s, h));
			return 0;
		}

		int unhandle(w_socket s) {
			w_trace(("selector::unhandle()"));
			
			map_.erase(s);
			shutdown(s, 2);
			close(s);
			return 0;
		}
	};


	// Selector is singleton
	typedef w_singleton<selector> w_selector;


} // namespace wlib

#endif // W_SELETECTOR_H_ 
