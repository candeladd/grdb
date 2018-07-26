#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "config.h"
#include "graph.h"
#include "tuple.h"

#define NO_OF_ELEMENTS BUFSIZE

void
component_init(component_t c)
{
	assert (c != NULL);
	memset(c, 0, sizeof(struct component));

	/* Initial values for file descriptors */
	c->vfd = (-1);
	c->efd = (-1);
}

void
component_file_init(int gidx, int cidx)
{
	char s[BUFSIZE];

	/* Assume graph file initialization is done */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d", GRDBDIR, gidx, cidx);
	mkdir(s, 0755);
}

vertex_t
component_find_vertex_by_id(component_t c, vertex_t v)
{
	int len;

	assert (c != NULL);
	assert (v != NULL);

	/* Assume v was allocated and v->id was set by the caller */
	len = vertex_read(v, c->sv, c->vfd);
	if (len > 0)
		return v;

	return NULL;
}

int component_load_neighbor(int fd)
{
	return 0;
}


void
component_insert_vertex(component_t c, vertex_t v)
{
	assert (c != NULL);
	assert (v != NULL);

	vertex_write(v, c->vfd);
}

edge_t
component_find_edge_by_ids(component_t c, edge_t e)
{
	int len;

	assert (c != NULL);
	assert (e != NULL);

	/*
	 * Assume e was allocated and e->id1 and e->id2 were set by
	 * the caller
	 */
	len = edge_read(e, c->se, c->efd);
	if (len > 0)
		return e;

	return NULL;
}

edge_t
component_find_neighbor_edges_by_id(component_t c, edge_t e, int neighbor_fd)
{
	int len;

	assert (c != NULL);
	assert (e != NULL);

	/*
	 * Assume e was allocated and e->id1 and e->id2 were set by
	 * the caller
	 */
	len = edge_neighbor_read(e, c->se, c->efd, neighbor_fd);
	if (len > 0)
	{
		return e;
	}
	return NULL;
}

int
component_get_path(component_t c, edge_t e, int neighbors_fd, int visited_fd, int path_fd)
{
	return 0;
}

void
component_insert_edge(component_t c, edge_t e)
{
	assert (c != NULL);
	assert (e != NULL);

	edge_write(e, c->efd);
}

void
component_print(FILE *out, component_t c, int with_tuples)
{
	off_t off;
	ssize_t len, size;
	vertexid_t id, id1, id2;
	struct tuple tuple;
	char *buf;
	int readlen;

	assert (c != NULL);

	fprintf(out, "({");

	/* Vertices */
	if (c->sv == NULL)
		size = 0;
	else
		size = schema_size(c->sv);

	readlen = sizeof(vertexid_t) + size;
	buf = malloc(readlen);
	assert (buf != NULL);
	memset(buf, 0, readlen);
	//printf("**************the buffer right now has %s", buf);

	for (off = 0;; off += readlen) {
		lseek(c->vfd, off, SEEK_SET);
		len = read(c->vfd, buf, readlen);
		if (len <= 0)
			break;

		if (off > 0)
			fprintf(out, ",");

		id = *((vertexid_t *) buf);
		fprintf(out, "%llu", id);

		if (c->sv != NULL && with_tuples) {
			memset(&tuple, 0, sizeof(struct tuple));
			tuple.s = c->sv;
			tuple.len = size;
			tuple.buf = buf + sizeof(vertexid_t);
			tuple_print(out, &tuple, c->el);
		}
	}
	fprintf(out, "},{");

	/* Edges */
	if (c->se == NULL)
		size = 0;
	else
		size = schema_size(c->se);

	readlen = (sizeof(vertexid_t) << 1) + size;
#if 0
	free(buf);
#endif
	buf = malloc(readlen);
	assert (buf != NULL);
	memset(buf, 0, readlen);

	for (off = 0;; off += readlen) {
		lseek(c->efd, off, SEEK_SET);
		len = read(c->efd, buf, readlen);
		if (len <= 0)
			break;

		if (off > 0)
			fprintf(out, ",");

		id1 = *((vertexid_t *) buf);
		id2 = *((vertexid_t *) (buf + sizeof(vertexid_t)));
		fprintf(out, "(%llu,%llu)", id1, id2);

		if (c->se != NULL && with_tuples) {
			memset(&tuple, 0, sizeof(struct tuple));
			tuple.s = c->se;
			tuple.len = size;
			tuple.buf = buf + (sizeof(vertexid_t) << 1);
			tuple_print(out, &tuple, c->el);
		}
	}
#if 0
	free(buf);
#endif
	fprintf(out, "})");
}



/*reads all the vertexid in the path file
 * and print them to the console
*/
int print_path(int path_fd)
{
	off_t off;
	ssize_t len;
	vertexid_t id;
	char buf[sizeof(vertexid_t)];

#if _DEBUG
	printf("*****the file that was read was %d\n", path_fd);
#endif
	printf("the path in reverse order is:\n");
	/* Search for vertex id in current component */
	for (off = 0;;) {
		lseek(path_fd, off, SEEK_SET);
		len = read(path_fd, buf, sizeof(vertexid_t));
		if (len == 0)
			break;
		if (len != sizeof(vertexid_t)) {
#if _DEBUG
			printf("vertex_read: ");
			printf("read %lu bytes of vertex id\n bad bad no good in print path\n", len);
#endif
			return (-1);
		}
		off += sizeof(vertexid_t);

		id = *((vertexid_t *) buf);

		
		printf(" %llu ", id);
	}
	printf("\n");
	return 0;
}



