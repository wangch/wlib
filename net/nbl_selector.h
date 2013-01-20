/*
 * nbl_lock.h
 *
 * Copyright (c) 2007 by NetBox, Inc.
 *
 * 2011-09-09 created by wangch
 *
 */

#ifndef NBL_SELETECTOR_H
#define NBL_SELETECTOR_H  

#include <map>
#include "nbl_thread.h"
#include "nbl_type.h"
#include "nbl_singleton.h"
#include "nbl_log.h"
#include "nbl_net.h"

namespace nbcl
{
	class selector
	{
		bool run_;
		mutex m_;

		nbl_thread<selector> thr_;
		std::map<nbl_socket, nbl_sock_handler*> map_;
		int ret_val_;

		int select()
		{
			nbl_trace(("selector::select()"));

			// TODO fdwrite and fdexcept support?
			fd_set fdread/*, fdwrite, fdexcept*/ ;
			struct timeval timeout;
			typedef std::map<nbl_socket, nbl_sock_handler*>::iterator IT;

			while(run_) {
				FD_ZERO(&fdread);
				//FD_ZERO(&fdwrite);
				//FD_ZERO(&fdexcept);

				//lock l(m_);
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
					nbl_error_r((CRIT, "select() error"), -1);
				else if(rc == 0) { // timeout
					//nbl_debug((INFO, "select() timeout"));
					continue;
				}

				for(it = map_.begin(); it != map_.end();) {
					nbl_socket s = it->first;
					nbl_sock_handler* h = it->second;
					int e = 0;
					if(FD_ISSET(s, &fdread)) {
						if(it->second->listening()) { // tcp listrening and accept remote peer connect
							struct sockaddr_storage addr;
							socklen_t addrlen = sizeof(addr);
							nbl_socket cs = ::accept(s, (struct sockaddr*)&addr, &addrlen);
							if(cs == -1) {
								nbl_error((WARN, "accept() error."));
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
						nbl_error((WARN, "fd except."));
						e = h->except_handler(s);
					}
					*/
					if (e == -1) {
						close(s); // close the socket
						h->error_handler(s);
						map_.erase(it++);
					} else {
						it++;
					}
				} // for
			} //while

#if defined(WINDOWS)
			WSACleanup();
#endif
			return 0;
		}
	public:
		selector() : run_(true)
		{
			nbl_trace(("selector::selector()"));
		}
		~selector()
		{
			nbl_trace(("selector::~selector()"));
			lock l(m_);
			run_ = false;
			//for_each(map_.begin(), map_.end(), nbl_deleter());
		}
		// the threadfunc should call exce
		int thr_exec()
		{
			nbl_trace(("selector::exec()"));
#if defined(WINDOWS)
			WSADATA wsd;
			if(WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
				nbl_error_r((CRIT, "unable to load Winsock!"), -1);
#endif
			return this->select();
		}

		// create a new thread and call exec
		int run(bool joinable = false)
		{
			nbl_trace(("selector::run()"));
			return thr_.run(this, joinable);
		}
		// register the sock handler 
		int handle(nbl_sock_handler* h, nbl_socket s)
		{
			nbl_trace(("selector::handle()"));
			//lock l(m_);
			map_.insert(pair<nbl_socket, nbl_sock_handler*>(s, h));
			return 0;
		}
		int unhandle(nbl_socket s)
		{
			nbl_trace(("selector::unhandle()"));
			//lock l(m_);
			map_.erase(s);
			shutdown(s, 2);
			close(s);
			return 0;
		}
	};


	// Selector is singleton
	typedef nbl_singleton<selector> nbl_selector;


} // namespace nbcl

#endif //NBL_SELETECTOR_H 
