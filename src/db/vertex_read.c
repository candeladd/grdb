#include <assert.h>
#if _DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"

/*
 * The vertex file is arranged as a packed list of vertex records.  Each
 * record contains a 64-bit vertex id followed by the vertex tuple.  If
 * an error or some sort occurs, the value (-1) is returned.  The value
 * zero means the end-of-file was reached.  Otherwise, the number of bytes
 * read in for the vertex tuple are returned.
 */
ssize_t
vertex_read(vertex_t v, schema_t schema, int fd)
{
	off_t off;
	ssize_t len, size;
	vertexid_t id;
	char buf[sizeof(vertexid_t)];

	assert(v != NULL);
#if _DEBUG
	printf("the file that was read was %d\n", fd);
	printf("vertex_read: read vertex %llu\n", v->id);
#endif
	if (schema == NULL)
		size = 0;
	else
		size = schema_size(schema);
#if _DEBUG
	printf("vertex_read: schema size = %lu bytes\n", size);
#endif
	/* Search for vertex id in current component */
	for (off = 0;;) {
		lseek(fd, off, SEEK_SET);
		len = read(fd, buf, sizeof(vertexid_t));
		if (len != sizeof(vertexid_t)) {
#if _DEBUG
			printf("vertex_read: ");
			printf("read %lu bytes of vertex id\n", len);
#endif
			return (-1);
		}
		off += sizeof(vertexid_t);

		id = *((vertexid_t *) buf);

		/* Read tuple buffer if there is one */
		if (size > 0) {
			if (v->tuple == NULL)
				tuple_init(&(v->tuple), schema);
			memset(v->tuple->buf, 0, size);
			lseek(fd, off, SEEK_SET);
			len = read(fd, v->tuple->buf, size);
#if _DEBUG
			printf("vertex_read: ");
			printf("read %lu bytes to tuple buffer\n", len);
#endif
			off += size;
		}
		if (id == v->id)
			return len;
	}
	return 0;
}

/*
 * The visited file is arranged as a packed list of vertexid_t. If
 * an error or some sort occurs, the value (-1) is returned.  The value
 * zero means the end-of-file was reached.  Otherwise, the number of bytes
 * read in for the vertex tuple are returned.
 */
vertexid_t
vertex_read_visited(int fd)
{
	off_t off;
	ssize_t len, size;
	vertexid_t id;
	char buf[sizeof(vertexid_t)];

#if _DEBUG
	printf("*****the file that was read was %d\n", fd);
#endif

	/* Search for vertex id in current component */
	for (off = 0;;) {
		lseek(fd, off, SEEK_SET);
		len = read(fd, buf, sizeof(vertexid_t));
		if (len != sizeof(vertexid_t)) {
#if _DEBUG
			printf("vertex_read: ");
			printf("read %lu bytes of vertex id\n bad bad no good in vertex_read_neighbor\n", len);
#endif
			return (-1);
		}
		off += sizeof(vertexid_t);

		id = *((vertexid_t *) buf);
		

		return id;
	}
	return 0;
}
