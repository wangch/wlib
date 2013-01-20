/*
 * nbl_net.h
 *
 * Copyright (c) 2007-2011 by NetBox, Inc.
 *
 * 2011-09-09 created by wangch
 *
 */

#ifndef NBL_NET_H
#define NBL_NET_H

#ifdef WINDOWS
#include <WINDOWS.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else //WINDOWS
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif //WINDOWS

#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "nbl_log.h"
#include "nbl_reqres.h"

namespace nbcl 
{
	using namespace std;

#ifdef WINDOWS
	typedef SOCKET nbl_socket;
	typedef int socklen_t;
inline int close(nbl_socket s)
{
	return ::closesocket(s);
}
#else //WINDOWS
	typedef int nbl_socket;
#endif //WINDOWS

	class nbl_sock_handler
	{
		bool listening_;
	public:
		nbl_sock_handler()
			: listening_(false)
		{
		}
		virtual ~nbl_sock_handler()
		{
		}
		bool listening() 
		{
		  	return listening_;
	  	}
		nbl_socket listen(int port, bool istcp = true)
		{
			nbl_trace(("nbl_sock_handler::listen()"));

			struct addrinfo hints, *res = NULL, *ptr = NULL;
			memset(&hints, 0, sizeof(hints));

			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = istcp ? SOCK_STREAM : SOCK_DGRAM;
			hints.ai_protocol = 0; 
			hints.ai_flags = AI_PASSIVE;
			hints.ai_canonname = NULL;
			hints.ai_addr = NULL;
			hints.ai_next = NULL;

			char svc[10];
			sprintf(svc, "%d", port);

			int rc = getaddrinfo(NULL, svc, &hints, &res);
			if(rc != 0)
				nbl_error_r((CRIT, "getaddinfo() error"), -1);

			nbl_socket fd; 
			for(ptr = res; ptr != NULL; ptr = ptr->ai_next) {
				fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if(fd == -1) {
					nbl_error((WARN, "socket() error"));
					continue;
				}

				if(bind(fd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
					if(istcp) {
						listening_ = true;
						if(::listen(fd, 49) != 0) { // tcp listen
							nbl_error((WARN, "listen() error"));
							close(fd);
							continue;
						}
					}
					break; // OK
				} else {
					close(fd);
				}
			}
			if (ptr == NULL) {
				nbl_error((CRIT, "no socket listen success"));
				fd = -1;
			}
			freeaddrinfo(res);
			return fd;
		}
		nbl_socket connect(std::string& server, int port, bool istcp = true)
		{
			nbl_trace(("nbl_sock_handler::connect()"));
			struct addrinfo hints, *res = NULL, *ptr = NULL;
			memset(&hints, 0, sizeof(hints));

			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = istcp ? SOCK_STREAM : SOCK_DGRAM;
			hints.ai_protocol = 0; // istcp ? IPPROTO_TCP : IPPROTO_UDP;
			hints.ai_flags = 0;

			char svc[10];
			sprintf(svc, "%d", port);

			int rc = getaddrinfo(server.c_str(), svc, &hints, &res);
			if (rc != 0) {
				nbl_error_r((CRIT, "getaddinfo() error"), -1);
			}

			nbl_socket fd; 
			for(ptr = res; ptr != NULL; ptr = ptr->ai_next) {
				fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if(fd == -1) {
					nbl_error((WARN, "socket() error"));
					continue;
				}
				if(::connect(fd, ptr->ai_addr, ptr->ai_addrlen) != -1) {
					break; // ok
				}
				close(fd);
			}

			if (ptr == NULL) {
				nbl_error((CRIT, "no socket connect success"));
				fd = -1;
			}

			freeaddrinfo(res);
			return fd;
		}
		virtual int accept_handler(nbl_socket s, struct sockaddr_storage peer_addr) { return 0; }
		virtual int read_handler(nbl_socket s) { return 0; }
		virtual int write_handler(nbl_socket s) { return 0; }
		virtual int except_handler(nbl_socket s) { return 0; }
		virtual int timeout_handler(nbl_socket s) { return 0; }
		virtual int error_handler(nbl_socket s) { return 0; }
	};
	// reader and writer on a socket 
	class nbl_reader_writer
	{
		nbl_socket fd_;
	public:
		nbl_reader_writer()
		{
			nbl_trace(("nbl_reader_writer::nbl_reader_writer()"));
		}
		nbl_reader_writer(int fd) : fd_(fd)
		{
			nbl_trace(("nbl_reader_writer::nbl_reader_writer(int)"));
		}

		~nbl_reader_writer()
		{
			nbl_trace(("nbl_reader_writer::~nbl_reader_writer()"));
		}

		nbl_socket fd()
	  	{
		  	return fd_;
	  	}

		int read(char* buf, int len)
		{
			nbl_trace(("nbl_reader_writer::read(char*, int)"));
			int r = 0;
			do {
				int n = recv(fd_, buf + r, len - r, 0);
				if (n == 0) {
					return 0;
				}
				if (n == -1) {
					nbl_error_r((CRIT, "recv() error int nbl_reader_writer::read()"), -1);
				}
				r += n;
			} while (r < len);

			return r;
		}

		int write(char* buf, int len)
		{
			nbl_trace(("nbl_reader_writer::write(char*, int)"));
			int s = 0;
			do {
				int n = send(fd_, buf + s, len - s, 0);
				if (n == -1) {
					nbl_error_r((CRIT, "send() error int nbl_reader_writer::write()"), -1);
				}
				s += n;
			} while (s < len);

			return s;
		}

	};

} // namespace nbcl

#endif //NBL_NET_H
