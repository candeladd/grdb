#if _DEBUG
#include <errno.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "cli.h"
#include "graph.h"

static void
cli_graph_component_new()
{
	struct vertex v;
	char s[BUFSIZE];
	int fd, n;

	n = graph_next_cno(grdbdir, gno);
	if (n < 0) {
#if _DEBUG
		printf("cli_graph_component_new: bad next cno\n");
#endif
		return;
	}
	/* Create first vertex in component */
	vertex_init(&v);
	vertex_set_id(&v, 1);

	/* Persistence... */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d", grdbdir, gno, n);
	mkdir(s, 0755);

	/*create component neighbors dir */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/neighbors", grdbdir, gno, n);
	mkdir(s, 0755);

	/*create component neighbors dir */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/path", grdbdir, gno, n);
	mkdir(s, 0755);

	/* Create component vertex file */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/v", grdbdir, gno, n);
#if _DEBUG
	printf("cli_graph_component_new: open vertex file %s\n", s);
#endif
	fd = open(s, O_RDWR | O_CREAT, 0644);
	if (fd < 0)
		return;

	/* Write first vertex tuple */
	vertex_write(&v, fd);
	close(fd);
}

static void
cli_graph_component_neighbors(char *cmdline, int *pos)
{
	vertexid_t id1, id2, visited_id;
	vertex_t v1, visited_v1;
	struct vertex v, visited_v;
	struct edge e;
	edge_t e1;
	char s[BUFSIZE];
	char neighbors_file_string[BUFSIZE];
	struct component c;
	int fd, i, neighbors_fd;

	component_init(&c);
    
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	id1 = (vertexid_t) atoi(s);
	i = atoi(s);

	vertex_init(&v);
	vertex_init(&visited_v);
	vertex_set_id(&v, i);

	/* Setup a component for searching */
	component_init(&c);

	/* Load enums */
	fd = enum_file_open(grdbdir, gno, cno);
	if (fd < 0) {
		printf("Open enum file failed\n");
		return;
	}
	enum_list_init(&(c.el));
	enum_list_read(&(c.el), fd);
	close(fd);

	/* Load the edge schema */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/se", grdbdir, gno, cno);
#if _DEBUG
	printf("cli_graph_component: read edge schema file %s\n", s);
#endif
	fd = open(s, O_RDWR | O_CREAT, 0644);
	if (fd < 0) {
		printf("Open edge schema file failed\n");
		return;
	}
	c.se = schema_read(fd, c.el);
	close(fd);



	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/v", grdbdir, gno, cno);
#if _DEBUG
	printf("cli_graph_edge: open vertex file %s\n", s);
#endif
	c.vfd = open(s, O_RDWR);
	v1 = component_find_vertex_by_id(&c, &v);
	close(c.vfd);
#if _DEBUG
	if (v1 == NULL)

		printf("cli_graph_edge: vertex %d not found\n", i);
#endif
	if(v1 != NULL)
		printf("we have found a vertex using these crazy methods now we need to find all the verticies %llu \n", v1->id);

	/* Create visited file */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/visited", grdbdir, gno, cno);
#if _DEBUG
	printf("cli_graph_component_new: open vertex file %s\n", s);
#endif
	fd = open(s, O_RDWR | O_CREAT, 0644);
	if (fd < 0)
		return;

	/* Write first vertex to the visited file we don't need schema */
	int visited_value = vertex_write_visited(&v, fd);
	if (visited_value == 2)
		printf("I visited this one before");
	
	/* read it back develope how I will do this */
	visited_id = component_load_visited_id(fd);
#if _DEBUG
	if (visited_id <= 0)

		printf("cli_graph_component: something is wrong in the visited loading");
#endif
	close(fd);
	if (visited_id > 0)
		vertex_set_id(&visited_v, visited_id);
		visited_v1 = &visited_v;
		printf("the visited idea works*******************visited %llu \n", visited_v1->id);


	//I am attempting to bastardize edge read to pick up the first vertex that matches and print out the edge
	
	/* create the neighbors file for this vertex id*/
	memset(neighbors_file_string, 0, BUFSIZE);
	sprintf(neighbors_file_string, "%s/%d/%d/neighbors/%d", grdbdir, gno, cno, i);
#if _DEBUG
	printf("cli_graph_component: read neighbors file %s\n", s);
#endif
	neighbors_fd = open(neighbors_file_string, O_RDWR | O_CREAT, 0644);
	if (neighbors_fd < 0) 
	{
		printf("blew it here and blew it there");
	}

	/* Open the edge file and call find neighbors */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/e", grdbdir, gno, cno);
#if _DEBUG
	printf("cli_graph_tuple: open edge file %s\n", s);
#endif
	c.efd = open(s, O_RDWR | O_CREAT, 0644);
	
	if (c.efd < 0) {
		
		printf("Find edges ids (%llu,%llu) failed\n",
			id1, id2);
		return;
	}
	edge_init(&e);
	edge_set_vertices(&e, id1, id1);
	e1 = component_find_neighbor_edges_by_id(&c, &e, neighbors_fd);
	//printf("trying to catch a fault\n");
	if (e1 == NULL) {
		printf("Illegal edge id(s)\n");
		return;
	}
	close(c.efd);

	close(neighbors_fd);
#if _DEBUG
	printf("cli_graph_component_neighbors: \n ");
	printf("determine neighbors of vertex id %llu\n", id1);
#endif
}


static int
cli_graph_component_connected(char *cmdline, int *pos)
{
	vertexid_t id1, id2;
	struct edge e;
	char s[BUFSIZE];
	char neighbors_file_string[BUFSIZE];
	struct component c;
	int fd, i, neighbors_fd, visited_fd, path_fd;
	int path_result;
	ssize_t  path_ok;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	id1 = (vertexid_t) atoi(s);
	i = atoi(s);
	
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	id2 = (vertexid_t) atoi(s);

	/* Setup a component for searching */
	component_init(&c);

	/* Load enums */
	fd = enum_file_open(grdbdir, gno, cno);
	if (fd < 0) {
		printf("Open enum file failed\n");
		return -1;
	}
	enum_list_init(&(c.el));
	enum_list_read(&(c.el), fd);
	close(fd);

	/* Load the edge schema */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/se", grdbdir, gno, cno);
#if _DEBUG
	printf("cli_graph_component: read edge schema file %s\n", s);
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
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/visited", grdbdir, gno, cno);
#if _DEBUG
	printf("cli_graph_component_new: open visited file %s\n", s);
#endif
	visited_fd = open(s, O_RDWR | O_CREAT, 0644);
	if (visited_fd < 0)
		printf("error opening visited_fd ");

	/* Create path file */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/path/%d", grdbdir, gno, cno, i);
#if _DEBUG
	printf("cli_graph_component_new: open path file %s\n", s);
#endif
	path_fd = open(s, O_RDWR | O_CREAT, 0644);
	if (path_fd < 0)
		printf("error opening path fd ");
	
	/*create neighbors file*/
	memset(neighbors_file_string, 0, BUFSIZE);
	sprintf(neighbors_file_string, "%s/%d/%d/neighbors/%d", grdbdir, gno, cno, i);
#if _DEBUG
	printf("cli_graph_component: open neighbors file %s\n", s);
#endif
	neighbors_fd = open(neighbors_file_string, O_RDWR | O_CREAT, 0644);
	if (neighbors_fd < 0) 
		printf("error opening neighbors_fd \n");
	
	/* Open the edge file and call find neighbors */
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/e", grdbdir, gno, cno);
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
									 gno, 
									 cno);
	printf("the result of the search was %d \n", path_result);
	if (path_result == 1)
	{
		path_ok = vertex_write_path(id1, path_fd);
		if (path_ok == 2 || path_ok <= 0)
			printf("bad path write");
		int pv = print_path(path_fd);
		printf("it seems like there is no way to get a zero %d\n ", pv);
		if (pv  == 0)
		{
			printf("great work you found a path\n");
		}
	}
	if (path_result == 0)
	{
		printf("looks like theres no path better luck next time\n");
	}

	//need to clean up the path file and the visited file

	delete_visited(grdbdir, gno, cno);
	printf("really really\n");
	delete_path(grdbdir, gno, cno, i);
	printf("getting somewhere now\n");

	return 0;
}

static void
cli_graph_component_sssp(char *cmdline, int *pos)
{
	vertexid_t v1, v2;
	char s[BUFSIZE];
	int result;

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	v1 = (vertexid_t) atoi(s);

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	v2 = (vertexid_t) atoi(s);

	/* XXX Need to do some error checking on v1 and v2 here */

	/* Get the name of the attribute that contains the weights */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);

#if _DEBUG
	printf("cli_graph_component_sssp: ");
	printf("dijkstra on vertex ids %llu and %llu using weight %s\n",
		v1, v2, s);
#endif
	/* Setup and run Dijkstra */
	result = component_sssp(grdbdir, gno, cno, v1, v2, s);
	if (result < 0) {
		/* Failure... */
	}
}

static void
cli_graph_component_project(char *cmdline, int *pos)
{
	char s[BUFSIZE];

	/* Get list of attributes */

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
}

static void
cli_graph_component_select(char *cmdline, int *pos)
{
#if _DEBUG
	printf("cli_graph_component_select: select failed\n");
#endif
}

static void
cli_graph_component_union(char *cmdline, int *pos)
{
	int result;
	char s[BUFSIZE];
	int cidx1 = (-1), cidx2 = (-1);

	/* Get first component argument */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
#if _DEBUG
	printf("cli_graph_component_union: first component %s\n", s);
#endif
	cidx1 = atoi(s);

	/* Get second component argument */
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
#if _DEBUG
	printf("cli_graph_component_union: second component %s\n", s);
#endif
	cidx2 = atoi(s);

	result = component_union(cidx1, cidx2, grdbdir, gno);
	if (result < 0) {
#if _DEBUG
		printf("cli_graph_component_union: union failed\n");
#endif
	}
}



void
cli_graph_component(char *cmdline, int *pos)
{
	char s[BUFSIZE];

	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);

	if (strcmp(s, "new") == 0 || strcmp(s, "n") == 0)
		cli_graph_component_new();

	else if (strcmp(s, "neighbors") == 0)
		cli_graph_component_neighbors(cmdline, pos);

	else if (strcmp(s, "sssp") == 0)
		cli_graph_component_sssp(cmdline, pos);

	else if (strcmp(s, "project") == 0 || strcmp(s, "p") == 0)
		cli_graph_component_project(cmdline, pos);

	else if (strcmp(s, "select") == 0 || strcmp(s, "s") == 0)
		cli_graph_component_select(cmdline, pos);

	else if (strcmp(s, "union") == 0 || strcmp(s, "u") == 0)
		cli_graph_component_union(cmdline, pos);

	else if (strcmp(s, "connected") == 0 || strcmp(s, "con") == 0)
		cli_graph_component_connected(cmdline, pos);

	else if (strlen(s) == 0) {
		FILE *out;
		char s[BUFSIZE];

		memset(s, 0, BUFSIZE);
		sprintf(s, "/tmp/grdbGraphs");
		out = fopen(s, "w");
		if (out == NULL) {
			printf("cli_graph_component: fopen %s failed\n", s);
			return;
		}
		memset(s, 0, BUFSIZE);
		sprintf(s, "%d", gno);
		cli_components_print(out, s, 0); /* no tuples */

		fclose(out);
	}
}
