#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "crossline.h"

#define TOKEN_MATCH(token, in) (strncmp(token, in, sizeof(token)-1) == 0)

typedef uint8_t  u8;
typedef  int8_t  i8;
typedef uint16_t u16;
typedef  int16_t i16;
typedef uint32_t u32;
typedef  int32_t i32;
typedef uint64_t u64;
typedef  int64_t i64;

/* Reference: http://mralligator.com/q3/ */

/* "IBSP" */
#define MAGIC 0x50534249U

typedef struct vec2 {
	float s,t;
} vec2;

typedef struct vec3 {
	float x,y,z;
} vec3;

typedef struct ivec2 {
	i32 s, t;
} ivec2;

typedef struct ivec3 {
	i32 x, y, z;
} ivec3;


#pragma pack(push, 1)
typedef struct bvec2 {
	u8 phi;
	u8 theta;
} bvec2;

typedef union rgb {
	u32 color;
	struct {
		u8 r;
		u8 g;
		u8 b;
	};
} rgb;
#pragma pack(pop)

typedef union rgba {
	u32 color;
	struct {
		u8 r;
		u8 g;
		u8 b;
		u8 a;
	};
} rgba;

struct lump {
	/* offset from beginning of the file */
	u32 offset;
	/* length in bytes */
	u32 len;
};

struct header {
	u32 magic;
	u32 version;

	union {
		struct lump lumps[17];
		struct {
			struct lump entities;
			struct lump textures;
			struct lump planes;
			struct lump nodes;
			struct lump leafs;
			struct lump leaf_faces;
			struct lump leaf_brushes;
			struct lump models;
			struct lump brushes;
			struct lump brush_sides;
			struct lump vertices;
			struct lump mesh_verts;
			struct lump effects;
			struct lump faces;
			struct lump lightmaps;
			struct lump lightvols;
			struct lump vis_data;
		};
	};
} header;

struct entities {
	const char* ents;
};

size_t n_textures = 0;
struct texture {
	char name[64];
	int flags;
	int contents;
};

size_t n_planes;
struct plane {
	struct vec3 norm;
	float dist;
};

size_t n_nodes;
struct node {
	u32 plane;
	/* children indices, negative numbers are leaf indices */
	i32 children[2];
	/* integer bounding box coord */
	ivec3 bb_mins;
	ivec3 bb_maxs;
};

size_t n_leafs;
struct leaf {
	i32 cluster_idx;
	u32 area;
	/* bounding coords */
	ivec3 bb_mins;
	ivec3 bb_maxs;
	i32 leaf_face;
	u32 n_leaf_faces;
	u32 leaf_brush;
	u32 n_leafbrushes;
};

/* stores lists of face indices (one list per leaf) */
size_t n_leaf_faces;
struct leaf_faces {
	i32 face;
};

/* stores brush indices */
size_t n_leaf_brushes;
struct leaf_brush {
	i32 brush;
};

/* "rigid groups of world geometry" */
size_t n_models;
struct model {
	vec3 mins;
	vec3 maxs;
	/* first face */
	i32 face_start_idx;
	u32 n_faces;
	i32 brush_start_idx;
	u32 n_brushes;
};

/* used for painting and collision testing */
size_t n_brushes;
struct brush {
	i32 first_brushside_idx;
	u32 n_brushsides;
	i32 texture_idx;
};

/* brush bounding surfaces */
size_t n_brush_sides;
struct brush_side {
	i32 plane_idx;
	i32 texture_idx;
};


size_t n_vertices;
struct vertex {
	vec3 pos;
	vec2 tex_coords;
	vec2 lightmap_coords;
	vec3 norm;
	rgba color;
};

size_t n_mesh_verts;
struct mesh_vert {
	i32 idx;
};

size_t n_effects;
struct effect {
	char name[64];
	i32 brush_idx;
	/* always 5, but in q3dm8 there's one effect with a value of -1 */
	i32 unknown;
};

size_t n_faces;
struct face {
	i32	texture_idx;
	i32 effect_value; /* index into effects lump, or -1 */
	enum {
		POLYGON = 1,
		PATCH = 2,
		MESH = 3,
		BILLBOARD = 4,
	} type;
	i32 first_vertex_idx;
	u32 n_vertices;
	i32 first_mesh_vertex_idx;
	u32 n_mesh_vertices;
	i32 lightmap_idx;
	vec2 lightmap_start;
	vec2 lightmap_size;
	/* worldspace origin */
	vec3 lightmap_origin;
	vec3 lightmap_unit_vectors[2];
	vec3 norm;
	ivec2 patch_dimensions;
};

size_t n_lightmaps;
struct lightmap {
	rgb map[128][128];
};

/* uniform grid of lightin info for non-map objects */
size_t n_lightvols;
struct lightvol {
	rgb ambient;
	rgb directional;
	/* direction to light in spherical coordinates */
	bvec2 dir;
};

/* cluster-to-cluster visibility info, and there is only ONE record */
struct vis_data {
	u32 n_vectors;
	u32 sz_vectors;
	/* one bit per cluster per vector */
	/* size = n_vectors*sz_vectors */
	u8 vectors[];
};

char* entities;
struct texture* textures = NULL;
struct plane* planes = NULL;
struct node* nodes = NULL;
struct leaf* leafs = NULL;
struct leaf_face* leaf_faces = NULL;
struct leaf_brush* leaf_brushes = NULL;
struct model* models = NULL;
struct brush* brushes = NULL;
struct brush_side* brush_sides = NULL;
struct vertex* vertices = NULL;
struct mesh_vert* mesh_verts = NULL;
struct effect* effects = NULL;
struct face* faces = NULL;
struct lightmap* lightmaps = NULL;
struct lightvol* lightvols = NULL;
struct vis_data* vis_data = NULL;

u8* file_data;

/* load lumps, first calculating size and offset */
void load_lumps(const u8* data) {
	entities = calloc(header.entities.len+1, 1);
	memcpy(entities, file_data+header.entities.offset, header.entities.len);
	
	n_textures = header.textures.len / sizeof(struct texture);
	textures = (struct texture*)(data + header.textures.offset);

	n_planes = header.planes.len / sizeof(struct plane);
	planes = (struct plane*)(data + header.planes.offset);

	n_nodes = header.nodes.len / sizeof(struct node);
	nodes = (struct node*)(data + header.nodes.offset);

	n_leafs = header.leafs.len / sizeof(struct leaf);
	leafs = (struct leaf*)(struct leaf*)(data + header.leafs.offset);
	 
	n_leaf_faces = header.leaf_faces.len / sizeof(struct leaf_faces);
	faces = (struct face*)(data + header.faces.offset);
	 
	n_leaf_brushes = header.leaf_brushes.len / sizeof(struct leaf_brush);
	leaf_brushes = (struct leaf_brush*)(data + header.brushes.offset);
	 
	n_models = header.models.len / sizeof(struct model);
	models = (struct model*)(data + header.models.offset);
	 
	n_brushes = header.brushes.len / sizeof(struct brush);
	brushes = (struct brush*)(data + header.brushes.offset);
	 
	n_brush_sides = header.brush_sides.len / sizeof(struct brush_side);
	brush_sides = (struct brush_side*)(data + header.brush_sides.offset);
	 
	n_vertices = header.vertices.len / sizeof(struct vertex);
	vertices = (struct vertex*)(data + header.vertices.offset);
	 
	n_mesh_verts = header.mesh_verts.len / sizeof(struct mesh_vert);
	mesh_verts = (struct mesh_vert*)(data + header.mesh_verts.offset);
	 
	n_effects = header.effects.len / sizeof(struct effect);
	effects = (struct effect*)(data + header.effects.offset);
	 
	n_faces = header.faces.len / sizeof(struct face);
	faces = (struct face*)(data + header.faces.offset);
	 
	n_lightmaps = header.lightmaps.len / sizeof(struct lightmap);
	lightmaps = (struct lightmap*)(data + header.lightmaps.offset);
	 
	n_lightvols = header.lightvols.len / sizeof(struct lightvol);
	lightvols = (struct lightvol*)(data + header.lightvols.offset);

		 
	vis_data = (struct vis_data*)(data + header.vis_data.offset);
}

void print_ivec3(struct ivec3* v) {
	printf("{ .x = %i, .y = %i, .z = %i }", v->x, v->y, v->z);
}

void print_vec3(struct vec3* v) {
	printf("{ .x = %f, .y = %f, .z = %f }", v->x, v->y, v->z);
}

void print_texture(struct texture* tex) {
	printf("Texture #%zu:\n", tex-textures);
	printf("    name:\"%s\"\n", tex->name);
	printf("    flags: 0x%x\n", tex->flags);
	printf("    contents: 0x%x\n", tex->contents);
}

void print_plane(struct plane* plane, bool indent) {
	printf("Plane #%zu:\n", plane-planes);
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
void print_node(struct node* node, bool verbose) {
	printf("Node #%zu:\n", node-nodes);
	struct plane* plane = planes + node->plane;
	if(verbose) {
		printf("    ");
		print_plane(plane, true);
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

void list(const char* input) {
	/* single character to list */
	switch(*input) {
		/* entities */
		case 'e':
			printf("Entities: %s\n", entities);
		break;
		/* textures */
		case 't':
			printf("Textures:\n");
			for(size_t i = 0; i < n_textures; i++)
				print_texture(textures+i);
		break;
		/* leafs */
		case 'p':
			printf("Planes:\n");
			for(size_t i = 0; i < n_planes; i++)
				print_plane(planes+i, false);
		break;
		case 'n':
			printf("Nodes: \n");
			for(size_t i = 0; i < n_nodes; i++)
				print_node(nodes+i, false);
		break;
		default:
			fprintf(stderr, "Unrecognized listable: '%c'\n", *input);
		break;
	}
}

void view(const char* input) {
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
			print_node(nodes+i, true);
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

	FILE* in = fopen(argv[1], "rb");

	if(!in) {
		perror("open");
		exit(1);
	}

	fread(&header, sizeof(header), 1, in);

	if(header.magic != MAGIC) {
		fprintf(stderr, "File \"%s\" did not have BSP magic!\n", argv[1]);
		exit(1);
	}

	/* calculate file size, and read it into file_data */
	fseek(in, 0, SEEK_END);
	size_t file_sz = ftell(in);
	file_data = malloc(file_sz);
	fseek(in, 0, SEEK_SET);
	fread(file_data, file_sz, 1, in);
	fclose(in);

	printf("Read in %zu bytes\n", file_sz);

	load_lumps(file_data);

	printf("Entities Length: %u\n", header.entities.len);

	printf("# of Textures: %zu\n", n_textures);
	printf("# of Planes: %zu\n", n_planes);
	printf("# of Nodes: %zu\n", n_nodes);
	printf("# of Leaves: %zu\n", n_leafs);
	printf("# of Leaf Faces: %zu\n", n_leaf_faces);
	printf("# of Leaf Brushes: %zu\n", n_leaf_brushes);
	printf("# of Models: %zu\n", n_models);
	printf("# of Brushes: %zu\n", n_brushes);
	printf("# of Brush Sides: %zu\n", n_brush_sides);
	printf("# of Vertices: %zu\n", n_vertices);
	printf("# of Mesh Vertices: %zu\n", n_mesh_verts);
	printf("# of Effects: %zu\n", n_effects);
	printf("# of Faces: %zu\n", n_faces);
	printf("# of Lightmaps: %zu\n", n_lightmaps);
	printf("# of Light Volumes: %zu\n", n_lightvols);
	
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
			list(input);
		} else if(TOKEN_MATCH("view", input) || TOKEN_MATCH("v",input)) {
			if(input[1] == 'i')
				input += sizeof("view");
			else
				input += 2;
			view(input);
		} else {
			printf("not recognized\n");
		}
	}

	return 0;
}