#ifndef SWAY_RENDERER_H
#define SWAY_RENDERER_H

#include <stdbool.h>
#include <wlr/render/wlr_renderer.h>

// #define SWAY_CUSTOM_RENDERER

struct sway_output;

#ifdef SWAY_CUSTOM_RENDERER

#include <GLES2/gl2.h>
struct sway_renderer_shader {
    GLuint shader;
    GLint proj;
    GLint tex;
    GLint alpha;
    GLint pos_attrib;
    GLint tex_attrib;

    GLint width;
    GLint height;

    GLint padding_l;
    GLint padding_t;
    GLint padding_r;
    GLint padding_b;
    GLint cornerradius;
};

#endif

struct sway_renderer {
    struct sway_server* sway_server;
    struct wlr_renderer* wlr_renderer;

    struct sway_output* current;

#ifdef SWAY_CUSTOM_RENDERER
    /* Custom shaders */
    struct sway_renderer_shader shader_rgba;
    struct sway_renderer_shader shader_rgbx;
    struct sway_renderer_shader shader_ext;

    struct sway_renderer_shader shader_blurred_rgba;
    struct sway_renderer_shader shader_blurred_rgbx;
    struct sway_renderer_shader shader_blurred_ext;
#endif
};

void sway_renderer_init(struct sway_renderer *renderer, struct sway_server *server);
void sway_renderer_destroy(struct sway_renderer *renderer);

void sway_renderer_begin(struct sway_renderer *renderer, struct sway_output *output);
void sway_renderer_end(struct sway_renderer *renderer, pixman_region32_t *damage,
                     struct sway_output *output);
void sway_renderer_render_texture_at(struct sway_renderer *renderer,
                                   struct sway_output *output,
                                   pixman_region32_t *damage,
                                   struct wlr_texture *texture,
                                   struct wlr_box *box, double opacity,
                                   struct wlr_box *mask,
                                   double corner_radius,
                                   void* data);


#endif
