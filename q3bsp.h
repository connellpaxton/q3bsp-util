#ifndef Q3_LOAD_H_
#define Q3_LOAD_H_

#include <stdint.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t  u8;
typedef  int8_t  i8;
typedef uint16_t u16;
typedef  int16_t i16;
typedef uint32_t u32;
typedef  int32_t i32;
typedef uint64_t u64;
typedef  int64_t i64;

/* Reference: http://mralligator.com/q3/ */

extern enum q3bsp_errorcode {
	Q3BSP_NO_ERROR,
	Q3BSP_NO_MAGIC,
	Q3BSP_NO_OPEN,
} q3bsp_error;

/* "IBSP" */
#define Q3BSP_MAGIC 0x50534249U

typedef struct vec2 {
	union {
		float s;
		float x;
	};
	union {
		float t;
		float y;
	};
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

struct q3bsp_lump {
	/* offset from beginning of the file */
	u32 offset;
	/* length in bytes */
	u32 len;
};

struct q3bsp_header {
	u32 magic;
	u32 version;

	union {
		struct q3bsp_lump lumps[17];
		struct {
			struct q3bsp_lump entities;
			struct q3bsp_lump textures;
			struct q3bsp_lump planes;
			struct q3bsp_lump nodes;
			struct q3bsp_lump leafs;
			struct q3bsp_lump leaf_faces;
			struct q3bsp_lump leaf_brushes;
			struct q3bsp_lump models;
			struct q3bsp_lump brushes;
			struct q3bsp_lump brush_sides;
			struct q3bsp_lump vertices;
			struct q3bsp_lump mesh_verts;
			struct q3bsp_lump effects;
			struct q3bsp_lump faces;
			struct q3bsp_lump lightmaps;
			struct q3bsp_lump lightvols;
			struct q3bsp_lump vis_data;
		};
	};
};

struct q3texture {
	char name[64];
	int flags;
	int contents;
};

struct plane {
	struct vec3 norm;
	float dist;
};

struct q3node {
	u32 plane;
	/* children indices, negative numbers are leaf indices */
	i32 children[2];
	/* integer bounding box coord */
	ivec3 bb_mins;
	ivec3 bb_maxs;
};

struct q3leaf {
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
struct q3leaf_faces {
	i32 face;
};

/* stores brush indices */
struct q3leaf_brush {
	i32 brush;
};

/* "rigid groups of world geometry" */
struct q3model {
	vec3 mins;
	vec3 maxs;
	/* first face */
	i32 face_start_idx;
	u32 n_faces;
	i32 brush_start_idx;
	u32 n_brushes;
};

/* used for painting and collision testing */
struct q3brush {
	i32 first_brushside_idx;
	u32 n_brushsides;
	i32 texture_idx;
};

/* brush bounding surfaces */
struct q3brush_side {
	i32 plane_idx;
	i32 texture_idx;
};

struct q3vertex {
	vec3 pos;
	vec2 tex_coords;
	vec2 lightmap_coords;
	vec3 norm;
	rgba color;
};

struct q3mesh_vert {
	i32 idx;
};

struct q3effect {
	char name[64];
	i32 brush_idx;
	/* always 5, but in q3dm8 there's one effect with a value of -1 */
	i32 unknown;
};

struct q3face {
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

struct q3lightmap {
	rgb map[128][128];
};

/* uniform grid of lightin info for non-map objects */
struct q3lightvol {
	rgb ambient;
	rgb directional;
	/* direction to light in spherical coordinates */
	bvec2 dir;
};

/* cluster-to-cluster visibility info, and there is only ONE record */
struct q3vis_data {
	u32 n_vectors;
	u32 sz_vectors;
	/* one bit per cluster per vector */
	/* size = n_vectors*sz_vectors */
	u8 vectors[];
};

struct q3bsp {
	char*	file_data;
	char*	entities;
	size_t	n_textures;
	struct	q3texture* textures;
	size_t	n_planes;
	struct	plane* planes;
	size_t	n_nodes;
	struct	q3node* nodes;
	size_t	n_leafs;
	struct	q3leaf* leafs;
	size_t	n_leaf_faces;
	struct	q3leaf_face* leaf_faces;
	size_t	n_leaf_brushes;
	struct	q3leaf_brush* leaf_brushes;
	size_t	n_models;
	struct	q3model* models;
	size_t	n_brushes;
	struct	q3brush* brushes;
	size_t	n_brush_sides;
	struct	q3brush_side* brush_sides;
	size_t	n_vertices;
	struct	q3vertex* vertices;
	size_t	n_mesh_verts;
	struct	q3mesh_vert* mesh_verts;
	size_t	n_effects;
	struct	q3effect* effects;
	size_t	n_faces;
	struct	q3face* faces;
	size_t	n_lightmaps;
	struct	q3lightmap* lightmaps;
	size_t	n_lightvols;
	struct	q3lightvol* lightvols;
	struct	q3vis_data* vis_data;
};

struct q3bsp* q3bsp_load(const char* fname);
void q3bsp_free(struct q3bsp* bsp);

#ifdef __cplusplus
}
#endif
#endif