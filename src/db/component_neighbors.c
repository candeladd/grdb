#include "graph.h"
#define BUFSIZE		(1 << 12)


void
component_neighbors(char *grdbdir, int gidx, int cidx, vertexid_t id)
{
	DIR *dirfd;
	struct dirent *de;
	char s[BUFSIZE];
	FILE *out;
	char ch;
	int len, ret;

	if ((dirfd = opendir(grdbdir)) == NULL)
		return;
	memset(s, 0, BUFSIZE);
	sprintf(s, "/tmp/grdbGraphs");
	out = fopen(s, "w");
	if (out == NULL) {
		printf("fopen %s failed\n", s);
		return;
	}
	
	return;
}
