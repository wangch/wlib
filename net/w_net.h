/*
 * w_net.h
 */

#ifndef W_NET_H_
#define W_NET_H_

#ifdef _WINDOWS
#include <WINDOWS.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else // _WINDOWS
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif // _WINDOWS

#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "../util/w_log.h"
#include "w_reqres.h"

namespace wlib  {
	using namespace std;

#ifdef _WINDOWS
	typedef SOCKET w_socket;
	typedef int socklen_t;
inline int close(w_socket s) {
	return ::closesocket(s);
}
#else // _WINDOWS
	typedef int w_socket;
#endif // _WINDOWS

	class w_sock_handler {
		bool listening_;
	public:
		w_sock_handler() : listening_(false) {
		}

		virtual ~w_sock_handler() {
		}

		bool listening() {
		  	return listening_;
	  	}

		w_socket listen(int port, bool istcp = true) {
			w_trace(("w_sock_handler::listen()"));

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
				w_error_r((CRIT, "getaddinfo() error"), -1);

			w_socket fd; 
			for(ptr = res; ptr != NULL; ptr = ptr->ai_next) {
				fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if(fd == -1) {
					w_error((WARN, "socket() error"));
					continue;
				}

				if(bind(fd, ptr->ai_addr, ptr->ai_addrlen) == 0) {
					if(istcp) {
						listening_ = true;
						if(::listen(fd, 49) != 0) { // tcp listen
							w_error((WARN, "listen() error"));
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
				w_error((CRIT, "no socket listen success"));
				fd = -1;
			}
			freeaddrinfo(res);
			return fd;
		}

		w_socket connect(std::string& server, int port, bool istcp = true) {
			w_trace(("w_sock_handler::connect()"));
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
				w_error_r((CRIT, "getaddinfo() error"), -1);
			}

			w_socket fd; 
			for(ptr = res; ptr != NULL; ptr = ptr->ai_next) {
				fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
				if(fd == -1) {
					w_error((WARN, "socket() error"));
					continue;
				}
				if(::connect(fd, ptr->ai_addr, ptr->ai_addrlen) != -1) {
					break; // ok
				}
				close(fd);
			}

			if (ptr == NULL) {
				w_error((CRIT, "no socket connect success"));
				fd = -1;
			}

			freeaddrinfo(res);
			return fd;
		}

		virtual int accept_handler(w_socket s, struct sockaddr_storage peer_addr) { return 0; }
		virtual int read_handler(w_socket s) { return 0; }
		virtual int write_handler(w_socket s) { return 0; }
		virtual int except_handler(w_socket s) { return 0; }
		virtual int timeout_handler(w_socket s) { return 0; }
		virtual int error_handler(w_socket s) { return 0; }
	};
	// reader and writer on a socket 
	class w_reader_writer {
		w_socket fd_;
	public:
		w_reader_writer() {
			w_trace(("w_reader_writer::w_reader_writer()"));
		}

		w_reader_writer(int fd) : fd_(fd) {
			w_trace(("w_reader_writer::w_reader_writer(int)"));
		}

		~w_reader_writer() {
			w_trace(("w_reader_writer::~w_reader_writer()"));
		}

		w_socket fd() {
		  	return fd_;
	  	}

		int read(char* buf, int len) {
			w_trace(("w_reader_writer::read(char*, int)"));
			int r = 0;
			do {
				int n = recv(fd_, buf + r, len - r, 0);
				if (n == 0) {
					return 0;
				}
				if (n == -1) {
					w_error_r((CRIT, "recv() error int w_reader_writer::read()"), -1);
				}
				r += n;
			} while (r < len);

			return r;
		}

		int write(char* buf, int len) {
			w_trace(("w_reader_writer::write(char*, int)"));
			int s = 0;
			do {
				int n = send(fd_, buf + s, len - s, 0);
				if (n == -1) {
					w_error_r((CRIT, "send() error int w_reader_writer::write()"), -1);
				}
				s += n;
			} while (s < len);

			return s;
		}

	};

} // namespace wlib

#endif //W_NET_H_
