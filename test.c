#include "util.h"


int main(){
	char ip_address[INET6_ADDRSTRLEN];
	dnslookup("bogus", ip_address, INET6_ADDRSTRLEN);
	return 0;
}