#include <assert.h>
#if _DEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"

#define BUFSIZE		(1 << 12)

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
	//if we are looking for all edges (i, j) such that i== our vertex
	//loop through the edges file and look for edges where the
	//vertex we are checking for is in the ()
	/* Search for edge in current component */
	for (off = 0;; off += (sizeof(vertexid_t) << 1) + size) {
		cycle+=1;
		//printf("**** I am on cycle %d \n", cycle);
		lseek(fd, off, SEEK_SET);
		len = read(fd, buf, sizeof(vertexid_t) << 1);
		if (len != sizeof(vertexid_t) << 1) {
#if _DEBUG
			printf("edge_read:");
			printf("read %lu bytes to tuple buffer\n", len);
#endif
			return (-1);
		}
		id1 = *((vertexid_t *) buf);
		//printf("hoping!!! (%llu)\n", id1);
		id2 = *((vertexid_t *) (buf + sizeof(vertexid_t)));

		if (id1 == e->id1) {
									
			if (e->tuple == NULL && size != 0)
			{
				
				tuple_init(&(e->tuple), schema);
				memset(e->tuple->buf, 0, size);
				len = read(fd, e->tuple->buf, size);
			}

			
			
#if _DEBUG
			//printf("edge_read: ");
			
			//printf("read %lu bytes to tuple buffer\n", len);
			
			//printf("I got an edge for (%llu)\n", id1);		
#endif
			printf("hi neighbor (%llu)\n", id2);
			len = vertex_write_neighors(id2, neighbor_fd);
		}
		
	}
	return 0;
}

/*if we find any neighbors we need to write them to the neighbors file
  so we need to create a file in the neighbors directory titled for the vertexid
  that we are seaching then we need to write the vertices that we find*/
ssize_t
get_neighbors_path(edge_t e, 
				   struct component c, 
				   int neighbors_fd, 
				   int visited_fd, 
				   int path_fd,
				   char *grdbdir,
        		   int gidx,
        		   int cidx)
{
	schema_t schema = c.se;
	int fd = c.efd;
	struct edge s_edge;
	off_t off;
	ssize_t len, size, path_ok, visited_value;
	vertexid_t id1, id2;
	struct vertex v;
	char buf[sizeof(vertexid_t) << 1];
	vertex_init(&v);
	char neighbors_file_string[BUFSIZE];
	int cur_neighbors_fd;
	

	assert(e != NULL);
#if _DEBUG
	printf("edge_read: current search source (%llu) dest (%llu)\n", e->id1, e->id2);
#endif
        if (schema == NULL)
                size = 0;
        else
                size = schema_size(schema);
#if _DEBUG
	printf("edge_read: schema size = %lu bytes\n", size);
#endif
	int cycle=0;
	//if we are looking for all edges (i, j) such that i== our vertex
	//loop through the edges file and look for edges where the
	//vertex we are checking for is in the ()
	/* Search for edge in current component */
	for (off = 0;; off += (sizeof(vertexid_t) << 1) + size) 
	{
		cycle+=1;
		//printf("**** I am on cycle %d \n", cycle);
		lseek(fd, off, SEEK_SET);
		len = read(fd, buf, sizeof(vertexid_t) << 1);
		//if (len == 0)
			/* EOF reached */
			//break;
		if (len != sizeof(vertexid_t) << 1) {
#if _DEBUG
			printf("edge_read:");
			printf("read %lu bytes to tuple buffer\n", len);
#endif
			return (-1);
		}
		printf("cycle %d \n", cycle);
		id1 = *((vertexid_t *) buf);
		
		id2 = *((vertexid_t *) (buf + sizeof(vertexid_t)));
		printf("looking for edges of (%llu) \n", e->id1);
		printf("looking at edge (%llu)->(%llu) \n", id1, id2);

		if (id1 == e->id1) 
		{

			printf("hi neighbor (%llu)\n", id2);
			printf("my destination is (%llu)\n", e->id2);
			len = vertex_write_neighors(id2, neighbors_fd);
			if (len < 0 || len == 4)
			{
				printf("something is odd in here\n");
			}

			if (id2 == e->id2)
			{
				printf("Shit we found a path yeah boi\n");
				path_ok = vertex_write_path(e->id2, path_fd);
				//succes in writing to the path file 
				if (path_ok > 0)
				{  
					return 1;
				}
			}
			printf("write vertex to visited %llu \n", id2);
			vertex_set_id(&v, id2);
			visited_value = vertex_write_visited(&v, visited_fd);
			printf("visited value %ld\n", visited_value);
			if(visited_value != 2 && visited_value >= 0)
			{
				
				//init our search_edge
				edge_init(&s_edge);
				edge_set_vertices(&s_edge, id2, e->id2);
				printf("push (%llu)\n", s_edge.id1);

				/*I am going to search an new node I will get its neighbors
				 * so create a neighbors file for it
				*/
				memset(neighbors_file_string, 0, BUFSIZE);
				sprintf(neighbors_file_string, "%s/%d/%d/neighbors/%llu", grdbdir, gidx, cidx, id2);
#if _DEBUG
	printf("cli_graph_component: open new neighbors file %s\n", s);
#endif
				printf("cli_graph_component: open new neighbors file %s\n", neighbors_file_string);
				cur_neighbors_fd = open(neighbors_file_string, O_RDWR | O_CREAT, 0644);
				if (cur_neighbors_fd < 0) 
					printf("error opening neighbors_fd \n");
				
				// call get neighbors on this node to do dfs
				// if we reach a node with no neighbors we should return before we get here
				//then we will just go on to the next neighbor from the current example.
				// if we return a one we need to write the current id to the path and return 1
				len = get_neighbors_path(&s_edge, c, cur_neighbors_fd, visited_fd, path_fd, grdbdir, gidx, cidx);
				printf("???? the result from get path was (%ld) \n ", len);
				if (len == 1)
				{
					printf("we need to append to the path (%llu)\n", id1);
					path_ok = vertex_write_path(s_edge.id1, path_fd);

					printf("the path_ok val came back (%ld)\n", path_ok);
					//succes in writing to the path file 
					if (path_ok != 2 && path_ok > 0)
					{  
						return 1;
					}
				
				
				}
				printf("pop (%llu)\n", s_edge.id1);

			}
		}
		
	}
	printf("we reached the end of the file and we did not find a path\n"); 
	return 0;
}