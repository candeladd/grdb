#include <assert.h>
#if _DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"

/*
 * The edge file is arranged as a packed list of edge records.  Each
 * record contains two 64-bit vertex ids followed by the edge tuple.  If
 * an error or some sort occurs, the value (-1) is returned.  The value
 * zero means the end-of-file was reached.  Otherwise, the number of bytes
 * read in for the vertex tuple are returned.
 */
ssize_t
edge_read(edge_t e, schema_t schema, int fd)
{
	off_t off;
	ssize_t len, size;
	vertexid_t id1, id2;
	char buf[sizeof(vertexid_t) << 1];

	assert(e != NULL);
#if _DEBUG
	printf("edge_read: read edge (%llu,%llu)\n", e->id1, e->id2);
#endif
        if (schema == NULL)
                size = 0;
        else
                size = schema_size(schema);
#if _DEBUG
	printf("edge_read: schema size = %lu bytes\n", size);
#endif
	/* Search for edge in current component */
	for (off = 0;; off += (sizeof(vertexid_t) << 1) + size) {
		lseek(fd, off, SEEK_SET);
		len = read(fd, buf, sizeof(vertexid_t) << 1);
		if (len != sizeof(vertexid_t) << 1) {
#if _DEBUG
			printf("edge_read: ");
			printf("read %lu bytes to tuple buffer\n", len);
#endif
			return (-1);
		}
		id1 = *((vertexid_t *) buf);
		id2 = *((vertexid_t *) (buf + sizeof(vertexid_t)));
		
		if (id1 == e->id1 && id2 == e->id2) {
			if (e->tuple == NULL)
				tuple_init(&(e->tuple), schema);

			memset(e->tuple->buf, 0, size);
			len = read(fd, e->tuple->buf, size);
		
#if _DEBUG
			printf("edge_read: ");
			printf("read %lu bytes to tuple buffer\n", len);
#endif
			return len;
		}
	}
	return 0;
}



/*if we find any neighbors we need to write them to the neighbors file
  so we need to create a file in the neighbors directory titled for the vertexid
  that we are seaching then we need to write the vertices that we find*/
ssize_t
edge_neighbor_read(edge_t e, 
				   schema_t schema,
				   int fd, 
				   int neighbor_fd)
{
	off_t off;
	ssize_t len, size;
	vertexid_t id1, id2;
	char buf[sizeof(vertexid_t) << 1];

	assert(e != NULL);
#if _DEBUG
	printf("edge_read: read edge (%llu,%llu)\n", e->id1, e->id2);
#endif
        if (schema == NULL)
                size = 0;
        else
                size = schema_size(schema);
#if _DEBUG
	printf("edge_read: schema size = %lu bytes\n", size);
#endif
	int cycle=0;
	printf("neighbors: \n");
	//if we are looking for all edges (i, j) such that i== our vertex
	//loop through the edges file and look for edges where the
	//vertex we are checking for is in the ()
	/* Search for edge in current component */
	for (off = 0;; off += (sizeof(vertexid_t) << 1) + size) {
		cycle+=1;
		//printf("**** I am on cycle %d \n", cycle);
		lseek(fd, off, SEEK_SET);
		len = read(fd, buf, sizeof(vertexid_t) << 1);
		if(len == 0)
			break;
		if (len != sizeof(vertexid_t) << 1) {
#if _DEBUG
			printf("edge_read:");
			printf("read %lu bytes to tuple buffer\n", len);
#endif
			return (-1);
		}
		id1 = *((vertexid_t *) buf);
		id2 = *((vertexid_t *) (buf + sizeof(vertexid_t)));

		if (id1 == e->id1) {
									
			if (e->tuple == NULL && size != 0)
			{
				
				tuple_init(&(e->tuple), schema);
				memset(e->tuple->buf, 0, size);
				len = read(fd, e->tuple->buf, size);
			}

			
			
#if _DEBUG
			printf("edge_read: ");
			printf("read %lu bytes to tuple buffer\n", len);
			
			printf("I got an edge for (%llu)\n", id1);		
#endif
			/* print out each neighbor as it comes around*/
			printf(" (%llu),", id2);
			len = vertex_write_neighors(id2, neighbor_fd);
		}
		
	}
	printf("\n");
	return 0;
}