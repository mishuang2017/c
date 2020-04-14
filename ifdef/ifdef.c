#include <stdio.h>
#include <net/netevent.h>

#if 0
enum netevent_notif_type {
        NETEVENT_NEIGH_UPDATE = 1, /* arg is struct neighbour ptr */
        NETEVENT_REDIRECT,         /* arg is struct netevent_redirect ptr */
        NETEVENT_DELAY_PROBE_TIME_UPDATE, /* arg is struct neigh_parms ptr */
};
#endif

int main(int argc, char *argv[])
{
#ifdef NETEVENT_DELAY_PROBE_TIME_UPDATE
	printf("hello, ifdef\n");
#endif
	printf("hello 2, ifdef\n");
	return 0;
}
