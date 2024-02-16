#include "q3bsp.h"

#include "crossline.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define TOKEN_MATCH(token, in) (strncmp(token, in, sizeof(token)-1) == 0)

void print_ivec3(struct ivec3* v) {
	printf("{ .x = %i, .y = %i, .z = %i }", v->x, v->y, v->z);
}

void print_vec3(struct vec3* v) {
	printf("{ .x = %f, .y = %f, .z = %f }", v->x, v->y, v->z);
}

void print_texture(struct q3bsp* bsp, struct q3texture* tex) {
	printf("Texture #%zu:\n", tex-bsp->textures);
	printf("    name:\"%s\"\n", tex->name);
	printf("    flags: 0x%x\n", tex->flags);
	printf("    contents: 0x%x\n", tex->contents);
}

void print_plane(struct q3bsp* bsp, struct plane* plane, bool indent) {
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
void print_node(struct q3bsp* bsp, struct q3node* node, bool verbose) {
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

void list(struct q3bsp* bsp, const char* input) {
	/* single character to list */
	switch(*input) {
		/* entities */
		case 'e':
			printf("Entities: %s\n", bsp->entities);
		break;
		/* textures */
		case 't':
			printf("Textures:\n");
			for(size_t i = 0; i < bsp->n_textures; i++)
				print_texture(bsp, bsp->textures+i);
		break;
		/* leafs */
		case 'p':
			printf("Planes:\n");
			for(size_t i = 0; i < bsp->n_planes; i++)
				print_plane(bsp, bsp->planes+i, false);
		break;
		case 'n':
			printf("Nodes: \n");
			for(size_t i = 0; i < bsp->n_nodes; i++)
				print_node(bsp, bsp->nodes+i, false);
		break;
		default:
			fprintf(stderr, "Unrecognized listable: '%c'\n", *input);
		break;
	}
}

void view(struct q3bsp* bsp, const char* input) {
	/* type to read in */
	char type;
	/* index number to read in */
	i32 i;

	/* literally the first time I've ever used scanf, in 7 years of programming in C */
	sscanf(input, "%c %d", &type, &i);

	/* single character to list */
	switch(type) {
		/* entities */
		case 'e':
			abort();
		break;
		/* textures */
		case 't':
		break;
		case 'p':
		break;
		/* nodes */
		case 'n':
			print_node(bsp, bsp->nodes+i, true);
		break;
		default:
			fprintf(stderr, "Unrecognized viewable: '%c'\n", *input);
		break;
	}	
}

int main(int argc, char* argv[]) {
	if(argc < 2 || argc > 2) {
		fprintf(stderr, "Usage: %s file.bsp\n", argv[0]);
		exit(1);
	}

	struct q3bsp* bsp = q3bsp_load(argv[1]);
	if(!bsp) {
		fprintf(stderr, "Error loading file: \"%s\": \"%s\"\n", argv[1], q3bsp_error == Q3BSP_NO_MAGIC? "Missing MAGIC" : "Failed to open file");
		return 1;
	}

	printf("# of Textures: %zu\n", bsp->n_textures);
	printf("# of Planes: %zu\n", bsp->n_planes);
	printf("# of Nodes: %zu\n", bsp->n_nodes);
	printf("# of Leaves: %zu\n", bsp->n_leafs);
	printf("# of Leaf Faces: %zu\n", bsp->n_leaf_faces);
	printf("# of Leaf Brushes: %zu\n", bsp->n_leaf_brushes);
	printf("# of Models: %zu\n", bsp->n_models);
	printf("# of Brushes: %zu\n", bsp->n_brushes);
	printf("# of Brush Sides: %zu\n", bsp->n_brush_sides);
	printf("# of Vertices: %zu\n", bsp->n_vertices);
	printf("# of Mesh Vertices: %zu\n", bsp->n_mesh_verts);
	printf("# of Effects: %zu\n", bsp->n_effects);
	printf("# of Faces: %zu\n", bsp->n_faces);
	printf("# of Lightmaps: %zu\n", bsp->n_lightmaps);
	printf("# of Light Volumes: %zu\n", bsp->n_lightvols);

		/* start shell loop */
	char buff[256];
	const char* prompt = "root> ";
	for(const char* input; (input = crossline_readline(prompt, &buff[0], sizeof(buff)));) {
		/* should be a very very simple command syntax: 
			command [object0, object1...]
		*/
		if(TOKEN_MATCH("quit", input) || TOKEN_MATCH("q", input))
			break;
		else if(TOKEN_MATCH("list", input) || TOKEN_MATCH("l", input)) {
			if(input[1] == 'i')
				input += sizeof("list");
			else
				input += 2;
			list(bsp, input);
		} else if(TOKEN_MATCH("view", input) || TOKEN_MATCH("v",input)) {
			if(input[1] == 'i')
				input += sizeof("view");
			else
				input += 2;
			view(bsp, input);
		} else {
			printf("not recognized\n");
		}
	}

	return 0;
}