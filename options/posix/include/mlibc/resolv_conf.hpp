#ifndef _MLIBC_RESOLV_CONF
#define _MLIBC_RESOLV_CONF

#include <frg/vector.hpp>
#include <mlibc/allocator.hpp>
#include <netinet/in.h>
#include <sys/socket.h>

namespace mlibc {

	struct resolv_conf_nameserver {
		int type; // AF_* types
		union {
			struct in_addr ipv4;
			struct in6_addr ipv6;
		};
	};

	frg::vector<struct resolv_conf_nameserver, MemoryAllocator> &resolv_conf_nameservers();

} // namespace mlibc

#endif // _MLIBC_RESOLV_CONF
