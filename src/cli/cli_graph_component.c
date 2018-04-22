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
	vertex_t v1, v2;
	struct vertex v;
	struct edge e;
	edge_t e1;
	char s[BUFSIZE];
	char ch;
	char buf[sizeof(vertexid_t) << 1];
	struct component c;
	int fd, i;
	off_t off;
	ssize_t len, size;


	component_init(&c);
    
	memset(s, 0, BUFSIZE);
	nextarg(cmdline, pos, " ", s);
	id1 = (vertexid_t) atoi(s);
	i = atoi(s);

	vertex_init(&v);
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
	printf("cli_graph_tuple: read edge schema file %s\n", s);
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
	
	//I am attempting to bastardize edge read to pick up the first vertex that matches and print out the edge
	



	/* Open the edge file */
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
	e1 = component_find_edge_by_id(&c, &e);
	//printf("trying to catch a fault\n");
	if (e1 == NULL) {
		printf("Illegal edge id(s)\n");
		return;
	}
	close(c.efd);

	printf("we got one edge !!!!  the return value was %ld", e1);

	/*pull back all the verticies for the current graph
	for (;;)
	{
		// s2 is a vertex id for an edge 
		id2 = (vertexid_t) atoi(s2);

		// Open the edge file 
		memset(s, 0, BUFSIZE);
		sprintf(s, "%s/%d/%d/e", grdbdir, gno, cno);
#if _DEBUG
		printf("cli_graph_tuple: open edge file %s\n", s);
#endif
		c.efd = open(s, O_RDWR | O_CREAT, 0644);
		if (c.efd < 0) {
			printf("Find edge ids (%llu,%llu) failed\n",
				id1, id2);
			return;
		}
		edge_init(&e);
		edge_set_vertices(&e, id1, id2);
		e1 = component_find_edge_by_ids(&c, &e);
		if (e1 == NULL) {
			printf("Illegal edge id(s)\n");
			return;
		}
		close(c.efd);
		//playing around with a few concepts.
		if (e <=0)
		{
			//this means the egdge exits
		}
	}
	*/
	/*Load the appropriate schema 
	memset(s, 0, BUFSIZE);
	sprintf(s, "%s/%d/%d/%s",
		grdbdir, gno, cno, (st == VERTEX ? "sv" : "se"));
	#if _DEBUG
		printf("cli_graph_schema_add: read schema file %s\n", s);
	#endif
	fd = open(s, O_RDWR | O_CREAT, 0644);
	if (fd < 0)
		return;

	//create a vertex to pass to the read func
	
	vertex_init(&v);
	vertex_set_id(&v, 1);
	
	printf("fuck me running");
	int len = vertex_read(&v, NULL, 0);
	if (len >= 0)
		printf("len is ****** (%d)", len);
	we can try this horse shit later
	*/

#if _DEBUG
	printf("cli_graph_component_neighbors: \n ");
	printf("determine neighbors of vertex id %llu\n", id1);
#endif
}


static int
cli_graph_component_connected()
{
	
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
