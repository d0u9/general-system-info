#include <stdio.h>
#include <net.h>
#include <unistd.h>

int main(void)
{
	struct net *net = NULL;
	net = net_init();
	net_update(net);
	printf("tx bytes: %lu, rx bytes %lu\n", net->total_tx_bytes, net->total_rx_bytes);
	printf("----------------------\n");
	sleep(2);
	net_update(net);
	printf("tx bytes: %lu, rx bytes %lu\n", net->total_tx_bytes, net->total_rx_bytes);
	printf("----------------------\n");
	sleep(2);
	net_update(net);
	printf("tx bytes: %lu, rx bytes %lu\n", net->total_tx_bytes, net->total_rx_bytes);
	net_exit(net);
	return 0;
}
