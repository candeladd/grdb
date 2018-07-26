
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "graph.h"

int
component_connected_strong(
        char *grdbdir, int gidx, int cidx, vertexid_t id1, vertexid_t id2)
{
	struct edge e;
    char s[BUFSIZE],
	neighbors_file_string[BUFSIZE],
	visited_file_string[BUFSIZE],
	path_file_string[BUFSIZE];
	struct component c;
	int fd, neighbors_fd, visited_fd, path_fd;
	int path_result;
	ssize_t  path_ok;

#if _DEBUG
	printf("cli_graph_component_connected_strong: ");
	printf("determine vertexe ids %llu and %llu are strongly connected\n",
	       id1, id2);
#endif
    /* Setup a component for searching */
	component_init(&c);

	/* Load enums */
	fd = enum_file_open(grdbdir, gidx, cidx);
	if (fd < 0) {
		printf("Open enum file failed\n");
		return -1;
	}
	enum_list_init(&(c.el));
	enum_list_read(&(c.el), fd);
	close(fd);

	/* Load the edge schema */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/se", grdbdir, gidx, cidx);
#if _DEBUG
	printf("get_path: read edge schema file %s\n", s);
#endif
	fd = open(s, O_RDWR | O_CREAT, 0644);
	if (fd < 0) {
		printf("Open edge schema file failed\n");
		return -1;
	}
	c.se = schema_read(fd, c.el);
	close(fd);

	//initialize the edge to the desired connected edge
	edge_init(&e);
	edge_set_vertices(&e, id1, id2);

	/* Create visited file */
	memset(visited_file_string, 0, BUFSIZE);
	sprintf(visited_file_string, "%s/%d/%d/visited", grdbdir, gidx, cidx);
#if _DEBUG
	printf("component_connected_strong: open visited file %s\n", s);
#endif
	visited_fd = open(visited_file_string, O_RDWR | O_CREAT, 0644);
	if (visited_fd < 0)
		printf("error opening visited_fd ");

	/* Create path file */
	memset(path_file_string, 0, BUFSIZE);
	sprintf(path_file_string, "%s/%d/%d/path/%llu", grdbdir, gidx, cidx, id1);
#if _DEBUG
	printf("component_connected_strong_: open path file %s\n", s);
#endif
	path_fd = open(path_file_string, O_RDWR | O_CREAT, 0644);
	if (path_fd < 0)
		printf("error opening path fd ");
	
	/*create neighbors file*/
	memset(neighbors_file_string, 0, BUFSIZE);
	sprintf(neighbors_file_string, "%s/%d/%d/neighbors/%llu", grdbdir, gidx, cidx, id1);
#if _DEBUG
	printf("component_connected_weak: open neighbors file %s\n", s);
#endif
	neighbors_fd = open(neighbors_file_string, O_RDWR | O_CREAT, 0644);
	if (neighbors_fd < 0) 
		printf("error opening neighbors_fd \n");
	
	/* Open the edge file and call find neighbors */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/e", grdbdir, gidx, cidx);
#if _DEBUG
	printf("cli_graph_tuple: open edge file %s\n", s);
#endif
	c.efd = open(s, O_RDWR | O_CREAT, 0644);
	
	if (c.efd < 0) {
		
		printf("Find edges ids (%llu,%llu) failed\n",
			id1, id2);
		return -1;
	}

	/*
	 *find the path if the path exist we should get a 1 
	 * if we hit an eof we get 0
	 * errors are returned as -1
	 */ 

	path_result = get_neighbors_path(&e, 
					 c,  
					 neighbors_fd, 
					 visited_fd, 
					 path_fd,
					 grdbdir, 
					 gidx, 
					 cidx);
#if _DEBUG
	printf("the result of the search was %d \n", path_result);
#endif
	if (path_result == 1)
	{
		path_ok = vertex_write_path(id1, path_fd);
		if (path_ok == 2 || path_ok <= 0)
			printf("bad path write");
		int pv = print_path(path_fd);
		if (pv  == 0)
		{
			printf("There is a path\n");
		}
	}
	if (path_result == 0)
	{
		printf("looks like theres no path, better luck next time\n");
	}
	/* need to close all fd opened*/
	close(c.efd);
	close(visited_fd);
	close(path_fd);
	close(neighbors_fd);

	
    /*clean up all temp files used to search*/
       
    delete_visited(grdbdir, gidx, cidx);
	delete_path(grdbdir, gidx, cidx, id1);

    delete_neighbors(grdbdir, gidx, cidx);
	return 0;
}

/*finds weakly connected nodes if a path exists*/
int
component_connected_weak(
        char *grdbdir, int gidx, int cidx, vertexid_t id1, vertexid_t id2)
{
#if _DEBUG
	printf("cli_graph_component_connected_weak: ");
	printf("determine vertex ids %llu and %llu are weakly connected\n",
	       id1, id2);
#endif
	struct edge e;
    char s[BUFSIZE],
	neighbors_file_string[BUFSIZE],
	visited_file_string[BUFSIZE],
	path_file_string[BUFSIZE];
	struct component c;
	int fd, neighbors_fd, visited_fd, path_fd;
	int path_result;
	ssize_t  path_ok;


    /* Setup a component for searching */
	component_init(&c);

	/* Load enums */
	fd = enum_file_open(grdbdir, gidx, cidx);
	if (fd < 0) {
		printf("Open enum file failed\n");
		return -1;
	}
	enum_list_init(&(c.el));
	enum_list_read(&(c.el), fd);
	close(fd);

	/* Load the edge schema */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/se", grdbdir, gidx, cidx);
#if _DEBUG
	printf("get_path: read edge schema file %s\n", s);
#endif
	fd = open(s, O_RDWR | O_CREAT, 0644);
	if (fd < 0) {
		printf("Open edge schema file failed\n");
		return -1;
	}
	c.se = schema_read(fd, c.el);
	close(fd);

	//initialize the edge to the desired connected edge
	edge_init(&e);
	edge_set_vertices(&e, id1, id2);

	/* Create visited file */
	memset(visited_file_string, 0, BUFSIZE);
	sprintf(visited_file_string, "%s/%d/%d/visited", grdbdir, gidx, cidx);
#if _DEBUG
	printf("get_path: open visited file %s\n", s);
#endif
	visited_fd = open(visited_file_string, O_RDWR | O_CREAT, 0644);
	if (visited_fd < 0)
		printf("error opening visited_fd ");

	/* Create path file */
	memset(path_file_string, 0, BUFSIZE);
	sprintf(path_file_string, "%s/%d/%d/path/%llu", grdbdir, gidx, cidx, id1);
#if _DEBUG
	printf("component_connected_weak_new: open path file %s\n", s);
#endif
	path_fd = open(path_file_string, O_RDWR | O_CREAT, 0644);
	if (path_fd < 0)
		printf("error opening path fd ");
	
	/*create neighbors file*/
	memset(neighbors_file_string, 0, BUFSIZE);
	sprintf(neighbors_file_string, "%s/%d/%d/neighbors/%llu", grdbdir, gidx, cidx, id1);
#if _DEBUG
	printf("component_connected_weak: open neighbors file %s\n", s);
#endif
	neighbors_fd = open(neighbors_file_string, O_RDWR | O_CREAT, 0644);
	if (neighbors_fd < 0) 
		printf("error opening neighbors_fd \n");
	
	/* Open the edge file and call find neighbors */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/undir_e", grdbdir, gidx, cidx);
#if _DEBUG
	printf("component_connected_weak: open edge file %s\n", s);
#endif
	c.efd = open(s, O_RDWR | O_CREAT, 0644);
	
	if (c.efd < 0) {
		
		printf("Find edges ids (%llu,%llu) failed\n",
			id1, id2);
		return -1;
	}

	/*
	 *find the path if the path exist we should get a 1 
	 * if we hit an eof we get 0
	 * errors are returned as -1
	 */ 

	path_result = get_neighbors_path(&e, 
					 c,  
					 neighbors_fd, 
					 visited_fd, 
					 path_fd,
					 grdbdir, 
					 gidx, 
					 cidx);

#if _DEBUG
	printf("the result of the weak search was %d \n", path_result);
#endif
	if (path_result == 1)
	{
		path_ok = vertex_write_path(id1, path_fd);
		if (path_ok == 2 || path_ok <= 0)
			printf("bad path write");
		int pv = print_path(path_fd);
		if (pv  == 0)
		{
			printf("There is a path\n");
		}
	}
	if (path_result == 0)
	{
		printf("looks like theres no path, better luck next time\n");
	}
	/* need to close all fd opened*/
	close(c.efd);
	close(visited_fd);
	close(path_fd);
	close(neighbors_fd);

	
    /*clean up all temp files used to search*/
       
    delete_visited(grdbdir, gidx, cidx);
	delete_path(grdbdir, gidx, cidx, id1);
    /*need to change this to delete everything in the dir*/
    delete_neighbors(grdbdir, gidx, cidx);
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

	/* make the current source into a v and write it to visitd*/
	vertex_init(&v);
	vertex_set_id(&v, e->id1);
	visited_value = vertex_write_visited(&v, visited_fd);
#if _DEBUG
	printf("write vertex to visited %llu \n", e->id1);
	printf("visited value %ld\n", visited_value);
#endif
	if(visited_value == 2 || visited_value < 0)
	{
#if _DEBUG
	printf("Already visited %llu \n", e->id1);
#endif
		return 2;
	}

	/* start looping through edge file*/
	for (off = 0;; off += (sizeof(vertexid_t) << 1) + size) 
	{
	    cycle+=1;
		lseek(fd, off, SEEK_SET);
		len = read(fd, buf, sizeof(vertexid_t) << 1);
		if (len == 0)
			/* EOF reached */
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

#if _DEBUG
		printf("cycle %d \n", cycle);
		printf("looking for edges of (%llu) \n", e->id1);
		printf("looking at edge (%llu)->(%llu) \n", id1, id2);
#endif
		if (id1 == e->id1) 
		{

#if _DEBUG
			printf("hi neighbor (%llu)\n", id2);
			printf("my destination is (%llu)\n", e->id2);
#endif
			len = vertex_write_neighors(id2, neighbors_fd);
			if (len < 0 || len == 4)
			{
#if _DEBUG
				printf("Im seeing my neighbors again\n");
#endif
			}

			if (id2 == e->id2)
			{
				path_ok = vertex_write_path(e->id2, path_fd);
				//succes in writing to the path file 
				if (path_ok > 0)
				{  
					return 1;
				}
			}	
			//init our search_edge
			edge_init(&s_edge);
			edge_set_vertices(&s_edge, id2, e->id2);
#if _DEBUG
			printf("push (%llu)\n", s_edge.id1);
#endif
			/*I am going to search an new node I will get its neighbors
				* so create a neighbors file for it
			*/
			memset(neighbors_file_string, 0, BUFSIZE);
			sprintf(neighbors_file_string, "%s/%d/%d/neighbors/%llu", grdbdir, gidx, cidx, id2);
#if _DEBUG
	printf("get_path: open new neighbors file %s\n", neighbors_file_string);
#endif
			cur_neighbors_fd = open(neighbors_file_string, O_RDWR | O_CREAT, 0644);
			if (cur_neighbors_fd < 0) 
				printf("error opening neighbors_fd \n");
			
			/* recursively call get neigbors on the first neighbor to do dfs*/
			len = get_neighbors_path(&s_edge, c, cur_neighbors_fd, visited_fd, path_fd, grdbdir, gidx, cidx);
#if _DEBUG
			printf("the result from get_neighbors_path was (%ld) \n ", len);
#endif
			if (len == 1)
			{

		
				path_ok = vertex_write_path(s_edge.id1, path_fd);
#if _DEBUG
				printf("append to the path (%llu)\n", id1);
				printf("the path_ok val came back (%ld)\n", path_ok);
#endif
				//succes in writing to the path file 
				if (path_ok != 2 && path_ok > 0)
				{  
					return 1;
				}

			}
#if _DEBUG
				printf("pop (%llu)\n", s_edge.id1);
#endif
			close(cur_neighbors_fd);
		}	
	}
	return 0;
}


int file_open_helper(char *grdbdir, int gidx, int cidx, char *dir, vertexid_t id)
{
	char s[BUFSIZE];
	memset(s, 0, BUFSIZE);
	if (id > 0)
		sprintf(s, "%s/%d/%d/%s/%llu", grdbdir, gidx, cidx, dir, id);
	else
		sprintf(s, "%s/%d/%d/%s", grdbdir, gidx, cidx, dir);
	return open(s, O_RDWR | O_CREAT, 0644);

}

/* we need to clean up the visited file  after 
 * searching for a path
 */
void
delete_visited(char *grdbdir, int gidx, int cidx)
{
	int ret;
	char s[BUFSIZE];
	memset(s, 0, BUFSIZE);
	sprintf(s, "/bin/rm %s/%d/%d/visited", grdbdir, gidx, cidx);
	ret = system(s);
	if (ret < 0)
		printf("deleting visited failed\n");
}

/* we need to clean up the visited file  after 
 * searching for a path
 */
void delete_path(char *grdbdir, int gidx, int cidx, vertexid_t node)
{
	int ret;
	char s[BUFSIZE];
	memset(s, 0, BUFSIZE);
	sprintf(s, "/bin/rm %s/%d/%d/path/%llu", grdbdir, gidx, cidx, node);
	ret = system(s);
	if (ret < 0)
		printf("deleting visited failed\n");
}

/* we need to clean up the neighbors file  after 
 * searching for a path
 */
void delete_neighbors(char *grdbdir, int gidx, int cidx)
{
	int ret;
	char s[BUFSIZE];
	memset(s, 0, BUFSIZE);
	sprintf(s, "/bin/rm %s/%d/%d/neighbors/*", grdbdir, gidx, cidx);
	ret = system(s);
	if (ret < 0)
		printf("deleting neighbors failed\n");
}