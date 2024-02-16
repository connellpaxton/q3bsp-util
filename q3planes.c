#include <float.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>

#include "q3bsp.h"
#include "q3util.h"

#include <SDL2/SDL.h>

/* control detail of raymarching */
#define MAX_STEPS 64
#define MAX_DIST 200.0

struct vec3 cam_pos = { 0.0, -1.0, 0.0 };

size_t image_width = 256;
size_t image_height = 256;

/* raymarch planes */
/* make everything a copy and let the optimizer take care of it */
static float dot(struct vec3 v1, struct vec3 v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

static struct vec3 add(struct vec3 v1, struct vec3 v2) {
	return (struct vec3) {
		v1.x+v2.x, v1.y+v2.y, v1.z+v2.z
	};
}

static struct vec3 scalar(struct vec3 v, float s) {
	return (struct vec3) {
		v.x*s, v.y*s, v.z*s
	};
}

static float length(struct vec3 v) {
	return sqrtf(dot(v,v));
}

static vec3 normalize(struct vec3 v) {
	float d = 1.0 / length(v);
	return scalar(v, d);
}

static vec3 cross(struct vec3 v1, struct vec3 v2) {
	return (vec3) {
		.x = v1.y*v2.z - v1.z*v2.y,
		.y = v1.z*v2.x - v1.x*v2.z,
		.z = v1.x*v2.y - v1.y*v2.x
	};
}

static float sdf_plane(struct vec3 p, struct vec3 norm, float dist) {
	return dot(p, norm) + dist;
}

static float sdf(const struct q3bsp* bsp, const struct vec3 p) {
	float dist = FLT_MAX;
	dist = fminf(dist, length(p)-0.5);

	/*for(size_t i = 0; i < bsp->n_planes; i++) {
		dist = fminf(dist, sdf_plane(p, bsp->planes[i].norm, bsp->planes[i].dist));
	}*/

	return dist;
}

float cast(struct q3bsp* bsp, const struct vec3 dir) {
	float t = 0.0;

	for(size_t i = 0; i < MAX_STEPS; i++) {
		float dt = sdf(bsp, add(cam_pos, scalar(dir, t)));

		if(dt < 0.0001f*t)
			return t;
		else if(dt > MAX_DIST)
			return -1.0;
		
		t += dt;
	}

	return -1.0;
}

struct vec3 raygen(vec3 cam_dir, size_t x, size_t y) {
	vec2 ndc = {
		.x = 2.0 * (((float)x)/(float)image_width) - 1.0,
		.y = 1.0 - (2.0* (float)y/(float)image_height),
	};

	const float ratio = (float)image_width / (float)image_height;
	ndc.x *= ratio;

	const vec3 forward = cam_dir;
	const vec3 right = normalize(cross(forward, (vec3){0.0, 0.0, 1.0}));
	const vec3 up = normalize(cross(right, forward));

	return normalize(
		add(
			add(
				scalar(right, ndc.x),
				scalar(up, ndc.y)
			),
			scalar(forward, 2.0)
		)
	);
}

u32 rgbfromu8(u8 r, u8 g, u8 b) {
	return r << 16 | g << 8 | b;
}

u32 color(vec3 in) {
	return rgbfromu8(in.x*255.0, in.y*255.0, in.z*255.0);
}

u32* framebuffer = NULL;
/* temporary SDL context, so using globals */
SDL_Window* win;
SDL_Renderer* ren;
SDL_Texture* tex;
void init_video() {
	if(SDL_Init(SDL_INIT_VIDEO))
		goto err;

	if(SDL_CreateWindowAndRenderer(image_width, image_height, 0, &win, &ren))
		goto err;
	
	SDL_SetWindowTitle(win, "Q3 BSP Viewer\n");

	tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, image_width, image_height);

	if(!tex)
		goto defer1;

	return;

defer1:
	SDL_DestroyWindow(win);
err:
    fprintf(stderr, "%s\n", SDL_GetError());
	SDL_Quit();
    exit(EXIT_FAILURE);
}

void draw(struct q3bsp* bsp) {
	const vec3 cam_dir = { 0.0, -1.0, 0.0 };
	for(size_t y = 0; y < image_height; y++) {
		for(size_t x = 0; x < image_width; x++) {
			vec3 ray = raygen(cam_dir, x, y);
			framebuffer[y*image_width+x] = cast(bsp, cam_dir) >= 0.0? 0xFFFFFFFF : 0;
		}
	}

	SDL_UpdateTexture(tex, NULL, framebuffer, image_width*sizeof(uint32_t));

	SDL_RenderCopy(ren, tex, NULL, NULL);
	SDL_RenderPresent(ren);
}

void deinit_video() {
	SDL_DestroyTexture(tex);
	SDL_DestroyWindow(win);
	SDL_DestroyRenderer(ren);
	SDL_Quit();
}

int main(int argc, char* argv[]) {
	if(argc < 2 || argc > 2) {
		fprintf(stderr, "Usage: %s file.bsp\n", argv[0]);
		exit(1);
	}

	init_video();

	struct q3bsp* bsp = q3bsp_load(argv[1]);
	if(!bsp) {
		fprintf(stderr, "Error loading file: \"%s\": \"%s\"\n", argv[1], q3bsp_error == Q3BSP_NO_MAGIC? "Missing MAGIC" : "Failed to open file");
		return 1;
	}

	for(size_t i = 0; i < bsp->n_planes; i++) {
		print_plane(bsp, &bsp->planes[i], false);
	}

	framebuffer = calloc(image_height * image_width, sizeof(u32));

	SDL_Event e;
	while(SDL_WaitEvent(&e)) {
		if(e.type == SDL_QUIT)
			break;
		draw(bsp);

		//cam_pos.y -= 0.01;
	}

	free(framebuffer);
	q3bsp_free(bsp);

	deinit_video();
}