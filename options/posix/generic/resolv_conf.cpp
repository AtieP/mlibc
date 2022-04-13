#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>
#include <mlibc/allocator.hpp>
#include <mlibc/debug.hpp>
#include <mlibc/resolv_conf.hpp>
#include <netinet/in.h>
#include <sys/socket.h>


namespace mlibc {

static frg::vector<struct resolv_conf_nameserver, MemoryAllocator> nameservers(getAllocator());

struct resolv_conf_nameserver ns;


static void parse_nameserver_conf(const char *params, size_t len) {
	(void) len;


	auto ip = frg::string(params, len, getAllocator());
	struct resolv_conf_nameserver ns;

	if (inet_pton(AF_INET, ip.data(), &ns.ipv4))
		ns.type = AF_INET;
	else if (inet_pton(AF_INET6, ip.data(), &ns.ipv6))
		ns.type = AF_INET6;
	else
		return;

	nameservers.push(ns);
}


static void parse_search_conf(const char *params, size_t len) {
	(void) params;
	(void) len;
	mlibc::infoLogger() << "parse_search_conf(): unimplemented" << frg::endlog;
}


static void parse_sortlist_conf(const char *params, size_t len) {
	(void) params;
	(void) len;
	mlibc::infoLogger() << "parse_sortlist_conf(): unimplemented" << frg::endlog;
}


static void parse_options_conf(const char *params, size_t len) {
	(void) params;
	(void) len;
	mlibc::infoLogger() << "parse_options_conf(): unimpkenented" << frg::endlog;
}


static struct resolv_conf_conf {
	const char *name;
	void (*func)(const char *params, size_t len);
} configurations[] = {
	{
		.name = "nameserver",
		.func = parse_nameserver_conf
	},
	{
		.name = "search",
		.func = parse_search_conf
	},
	{
		.name = "sortlist",
		.func = parse_sortlist_conf
	},
	{
		.name = "options",
		.func = parse_options_conf
	}
};


static bool parse_conf(const char *conf, size_t len) {
	for (size_t i = 0; i < sizeof(configurations) / sizeof(struct resolv_conf_conf); i++) {
		size_t conf_len = strlen(configurations[i].name);
		if (len >= conf_len) {
			if (!strncmp(conf, configurations[i].name, conf_len)) {

				const char *cur = conf;
				size_t len2 = len;
				len2 -= conf_len;
				cur += conf_len;

				if (!len2) {
					mlibc::infoLogger()
						<< "parse_conf(): expected parameter(s)" << frg::endlog;
					return true;
				}

				if (*cur != ' ')
					continue;	// Wrong command, didn't catch the correct one

				cur++;
				len2--;
				configurations[i].func(cur, len2);
				return true;
			}
		}
	}
	return false;
}


static void resolv_conf_init();
static struct resolv_conf_constructor {
	resolv_conf_constructor() {
		resolv_conf_init();
	}
} resolv_conf_constructor;



static void resolv_conf_init() {
	auto file = fopen("/etc/resolv.conf", "r");
	if (!file) {
		mlibc::infoLogger()
			<< "resolv_conf_init(): could not open /etc/resolv.conf"
			<< frg::endlog;
		return;
	}

	char *buf = nullptr;
	ssize_t ret = 0;
	size_t len = 0;
	while ((ret = getline(&buf, &len, file)) > 0) {
		len = (size_t) ret - 1; // Ignore delimiter
		char *cur = buf;

		// Ignore any empty lines and comments
		if (!len || *cur == '#') {
			free(buf);
			buf = nullptr;
			len = 0;
			continue;
		}

		// Ignore any leading white space or tabs
		while (*cur == ' ' || *cur == '\t') {
			cur++;
			len--;
		}

		// Ignore lines with no real value other than random spaces
		if (!len) {
			free(buf);
			buf = nullptr;
			continue;
		}

		// Parse the configuration file
		if (!parse_conf(cur, len)) {
			mlibc::infoLogger()
				<< "resolv_conf_init(): unknown configuration " << cur << frg::endlog;
		}

		free(buf);
		buf = nullptr;
		len = 0;
	}

	if (ret == -1 && buf != nullptr && errno != ENOMEM)
		free(buf);

	fclose(file);
}


frg::vector<struct resolv_conf_nameserver, MemoryAllocator> &resolv_conf_nameservers() {
	return nameservers;
}


} // namespace mlibc
