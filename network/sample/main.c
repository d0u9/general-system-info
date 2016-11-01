#include <stdio.h>
#include <net.h>

int main(void)
{
	struct net *net = NULL;
	net = net_init();
	net_update(net);
	net_exit(net);
	return 0;
}
