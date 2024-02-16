#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "q3bsp.h"

enum q3bsp_errorcode q3bsp_error = Q3BSP_NO_ERROR;

static void q3bsp_load_lumps(struct q3bsp_header* header,  struct q3bsp* bsp) {
	/* load lumps, first calculating size and offset */
	bsp->entities = calloc(header->entities.len+1, 1);
	memcpy(bsp->entities, bsp->file_data+header->entities.offset, header->entities.len);
	
	bsp->n_textures = header->textures.len / sizeof(struct q3texture);
	bsp->textures = (struct q3texture*)(bsp->file_data + header->textures.offset);

	bsp->n_planes = header->planes.len / sizeof(struct plane);
	bsp->planes= (struct plane*)(bsp->file_data + header->planes.offset);

	bsp->n_nodes = header->nodes.len / sizeof(struct q3node);
	bsp->nodes = (struct q3node*)(bsp->file_data + header->nodes.offset);

	bsp->n_leafs = header->leafs.len / sizeof(struct q3leaf);
	bsp->leafs = (struct q3leaf*)(struct leaf*)(bsp->file_data + header->leafs.offset);
	 
	bsp->n_leaf_faces = header->leaf_faces.len / sizeof(struct q3leaf_faces);
	bsp->faces = (struct q3face*)(bsp->file_data + header->faces.offset);
	 
	bsp->n_leaf_brushes = header->leaf_brushes.len / sizeof(struct q3leaf_brush);
	bsp->leaf_brushes = (struct q3leaf_brush*)(bsp->file_data + header->brushes.offset);
	 
	bsp->n_models = header->models.len / sizeof(struct q3model);
	bsp->models = (struct q3model*)(bsp->file_data + header->models.offset);
	 
	bsp->n_brushes = header->brushes.len / sizeof(struct q3brush);
	bsp->brushes = (struct q3brush*)(bsp->file_data + header->brushes.offset);
	 
	bsp->n_brush_sides = header->brush_sides.len / sizeof(struct q3brush_side);
	bsp->brush_sides = (struct q3brush_side*)(bsp->file_data + header->brush_sides.offset);
	 
	bsp->n_vertices = header->vertices.len / sizeof(struct q3vertex);
	bsp->vertices = (struct q3vertex*)(bsp->file_data + header->vertices.offset);
	 
	bsp->n_mesh_verts = header->mesh_verts.len / sizeof(struct q3mesh_vert);
	bsp->mesh_verts = (struct q3mesh_vert*)(bsp->file_data + header->mesh_verts.offset);
	 
	bsp->n_effects = header->effects.len / sizeof(struct q3effect);
	bsp->effects = (struct q3effect*)(bsp->file_data + header->effects.offset);
	 
	bsp->n_faces = header->faces.len / sizeof(struct q3face);
	bsp->faces = (struct q3face*)(bsp->file_data + header->faces.offset);
	 
	bsp->n_lightmaps = header->lightmaps.len / sizeof(struct q3lightmap);
	bsp->lightmaps = (struct q3lightmap*)(bsp->file_data + header->lightmaps.offset);
	 
	bsp->n_lightvols = header->lightvols.len / sizeof(struct q3lightvol);
	bsp->lightvols = (struct q3lightvol*)(bsp->file_data + header->lightvols.offset);

		 
	bsp->vis_data = (struct q3vis_data*)(bsp->file_data + header->vis_data.offset);
}

struct q3bsp* q3bsp_load(const char* fname) {
	struct q3bsp* bsp = malloc(sizeof(struct q3bsp));

	FILE* in = fopen(fname, "rb");

	if(!in) {
		q3bsp_error = Q3BSP_NO_OPEN;
		return NULL;
	}

	struct q3bsp_header header;

	fread(&header, sizeof(header), 1, in);

	if(header.magic != Q3BSP_MAGIC) {
		q3bsp_error = Q3BSP_NO_MAGIC;
		return NULL;
	}

	/* calculate file size, and read it into file_data */
	fseek(in, 0, SEEK_END);
	size_t file_sz = ftell(in);
	bsp->file_data = malloc(file_sz);
	fseek(in, 0, SEEK_SET);
	fread(bsp->file_data, file_sz, 1, in);
	fclose(in);

	q3bsp_load_lumps(&header, bsp);

	return bsp;
}


void q3bsp_free(struct q3bsp* bsp) {
	free(bsp->file_data);
	free(bsp->entities);
	free(bsp);
}