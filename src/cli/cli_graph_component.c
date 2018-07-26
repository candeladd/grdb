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
	vertexid_t id1, id2;
	struct vertex v, visited_v;
	struct edge e;
	char s[BUFSIZE];
	char neighbors_file_string[BUFSIZE];
	struct component c;
	int fd, i, neighbors_fd;

#if _DEBUG
	printf("cli_graph_component_neighbors: \n ");
	printf("determine neighbors of vertex id %llu\n", id1);
#endif

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
	
	/* create the neighbors file for this vertex id*/
	memset(neighbors_file_string, 0, BUFSIZE);
	sprintf(neighbors_file_string, "%s/%d/%d/neighbors/%d", grdbdir, gno, cno, i);
#if _DEBUG
	printf("cli_graph_component: read neighbors file %s\n", s);
#endif
	neighbors_fd = open(neighbors_file_string, O_RDWR | O_CREAT, 0644);
	if (neighbors_fd < 0) 
	{
		printf("error opening neighbors file");
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
	component_find_neighbor_edges_by_id(&c, &e, neighbors_fd);
	close(c.efd);

	close(neighbors_fd);
	delete_neighbors(grdbdir, gno, cno);
}


/*check that the search vertices exist then calls weak connection*/
static void
cli_graph_component_connected_weak(char *cmdline, int *pos)
{
	struct component c;
	int result, fd;
	vertexid_t id1, id2;
	struct vertex v, w;
	vertex_t v1, w1;
	char s[BUFSIZE];
	
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	id1 = (vertexid_t) atoi(s);
	
	
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	id2 = (vertexid_t) atoi(s);

	/*check that our search vertices exist*/
	vertex_init(&v);
	vertex_set_id(&v, id1);

	component_init(&c);

	/* Load enums */
	fd = enum_file_open(grdbdir, gno, cno);
	if (fd >= 0) {
		enum_list_init(&(c.el));
		c.el = enum_list_read(&(c.el), fd);
		close(fd);
	}
	/* Load vertex schema */
	schema_init(&(c.sv));
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/sv", grdbdir, gno, cno);
	fd = open(s, O_RDWR);
	if (fd >= 0) {
		c.sv = schema_read(fd, c.el);
		close(fd);
	}

	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/v", grdbdir, gno, cno);
#if _DEBUG
	printf("cli_graph_edge: open vertex file %s\n", s);
#endif
	c.vfd = open(s, O_RDWR);
	v1 = component_find_vertex_by_id(&c, &v);
	close(c.vfd);


	vertex_init(&w);
	vertex_set_id(&w, id2);

	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/v", grdbdir, gno, cno);
#if _DEBUG
	printf("cli_graph_edge: ");
	printf("open vertex file %s\n", s);
#endif
	c.vfd = open(s, O_RDWR);
	w1 = component_find_vertex_by_id(&c, &w);
	close(c.vfd);


	if (v1 == NULL)  {
		printf("Invalid search %llu not found in component\n", id1);
		return;
	}
	else if(w1 == NULL)
	{
		printf("Invalid search %llu not found in component\n", id2);
		return;
	}

	result = component_connected_weak( grdbdir, gno, cno, id1, id2);

	

}

static void
cli_graph_component_connected_strong(char *cmdline, int *pos)
{
	struct component c;
	int result, fd;
	vertexid_t id1, id2;
	struct vertex v, w;
	vertex_t v1, w1;
	char s[BUFSIZE];
	

	
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
		if (strlen(s) == 0) {
		printf("Missing vertex id\n");
		return;
	}
	id1 = (vertexid_t) atoi(s);
	
	
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
		if (strlen(s) == 0) {
		printf("Missing vertex id\n");
		return;
	}
	id2 = (vertexid_t) atoi(s);


	/*check that the vertices exist in component*/
	vertex_init(&v);
	vertex_set_id(&v, id1);

	component_init(&c);

	/* Load enums */
	fd = enum_file_open(grdbdir, gno, cno);
	if (fd >= 0) {
		enum_list_init(&(c.el));
		c.el = enum_list_read(&(c.el), fd);
		close(fd);
	}
	/* Load vertex schema */
	schema_init(&(c.sv));
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/sv", grdbdir, gno, cno);
	fd = open(s, O_RDWR);
	if (fd >= 0) {
		c.sv = schema_read(fd, c.el);
		close(fd);
	}

	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/v", grdbdir, gno, cno);
#if _DEBUG
	printf("cli_graph_edge: open vertex file %s\n", s);
#endif
	c.vfd = open(s, O_RDWR);
	v1 = component_find_vertex_by_id(&c, &v);
	close(c.vfd);

	vertex_init(&w);
	vertex_set_id(&w, id2);

	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/v", grdbdir, gno, cno);
#if _DEBUG
	printf("cli_graph_edge: ");
	printf("open vertex file %s\n", s);
#endif
	c.vfd = open(s, O_RDWR);
	w1 = component_find_vertex_by_id(&c, &w);
	close(c.vfd);

	if (v1 == NULL)  {
		printf("Invalid search %llu not found in component\n", id1);
		return;
	}
	else if(w1 == NULL)
	{
		printf("Invalid search %llu not found in component\n", id2);
		return;
	}
#if _DEBUG
	printf("calling strong connection");
#endif
	result = component_connected_strong( grdbdir, gno, cno, id1, id2);
}

static void
cli_graph_component_connected(char *cmdline, int *pos)
{
	char arg[BUFSIZE];

	memset(arg, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", arg);

	if (strcmp(arg, "weak") == 0 || strcmp(arg, "w") == 0)
		cli_graph_component_connected_weak(cmdline, pos);
	else if (strcmp(arg, "strong") == 0|| strcmp(arg, "s") == 0)
		cli_graph_component_connected_strong(cmdline, pos);
	else
	{
		printf("Please select strong (s) or weak (w) connectedness test.\n");
		printf("Example: graph component connected weak v1 v2 \n");
	}

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

	else if (strcmp(s, "connected") == 0 || strcmp(s, "c") == 0)
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
