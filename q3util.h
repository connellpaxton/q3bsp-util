#ifndef Q3_UTIL
#define Q3_UTIL

#include "q3bsp.h"
#include <stdbool.h>

static void print_ivec3(struct ivec3* v) {
	printf("{ .x = %i, .y = %i, .z = %i }", v->x, v->y, v->z);
}

static void print_vec3(struct vec3* v) {
	printf("{ .x = %f, .y = %f, .z = %f }", v->x, v->y, v->z);
}

static void print_texture(struct q3bsp* bsp, struct q3texture* tex) {
	printf("Texture #%zu:\n", tex-bsp->textures);
	printf("    name:\"%s\"\n", tex->name);
	printf("    flags: 0x%x\n", tex->flags);
	printf("    contents: 0x%x\n", tex->contents);
}

static void print_plane(struct q3bsp* bsp, struct plane* plane, bool indent) {
	printf("Plane #%zu:\n", plane-bsp->planes);
	if(indent)
		printf("    ");
	printf("    Normal: ");
		print_vec3(&plane->norm);
	printf("\n");
	if(indent)
		printf("    ");
	printf("    Distance: %f\n", plane->dist);
}

/* v mode is verbose, by default we won't be */
static void print_node(struct q3bsp* bsp, struct q3node* node, bool verbose) {
	printf("Node #%zu:\n", node-bsp->nodes);
	struct plane* plane = bsp->planes + node->plane;
	if(verbose) {
		printf("    ");
		print_plane(bsp, plane, true);
	} else {
	printf("    Plane: %u\n", node->plane);
	}

	printf("    Bounding Boxes:\n        ");
	printf("mins: ");
	print_ivec3(&node->bb_mins);
	printf("\n        maxes: ");
	print_ivec3(&node->bb_maxs);
	printf("\n");
}

#endif