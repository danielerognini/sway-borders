#define _POSIX_C_SOURCE 200112L

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_matrix.h>
#include <render/gles2.h>
#include <render/egl.h>

#include "sway/server.h"
#include "sway/renderer.h"
#include "sway/output.h"

#ifdef SWAY_CUSTOM_RENDERER

const GLchar custom_tex_vertex_src[] =
"uniform mat3 proj;\n"
"attribute vec2 pos;\n"
"attribute vec2 texcoord;\n"
"varying vec2 v_texcoord;\n"
"\n"
"void main() {\n"
"	gl_Position = vec4(proj * vec3(pos, 1.0), 1.0);\n"
"	v_texcoord = texcoord;\n"
"}\n";


const GLchar custom_tex_fragment_src_rgba[] =
"precision mediump float;\n"
"varying vec2 v_texcoord;\n"
"uniform sampler2D tex;\n"
"uniform float alpha;\n"
"\n"
"uniform float width;\n"
"uniform float height;\n"
"uniform float cornerradius;\n"
"uniform float border_thickness;\n"
"uniform vec4 color;\n"
"\n"
"void main() {\n"
"   if(v_texcoord.x*width < cornerradius && v_texcoord.y*height < cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(cornerradius, cornerradius)) > cornerradius + border_thickness) discard;\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(cornerradius, cornerradius)) > cornerradius) {\n"
"           gl_FragColor = color;\n"
"           return;\n"
"       }\n"
"   }\n"
"   if(v_texcoord.x*width > width - cornerradius && v_texcoord.y*height < cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(width - cornerradius, cornerradius)) > cornerradius + border_thickness) discard;\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(width - cornerradius, cornerradius)) > cornerradius) {\n"
"           gl_FragColor = color;\n"
"           return;\n"
"       }\n"
"   }\n"
"   if(v_texcoord.x*width < cornerradius && v_texcoord.y*height > height - cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(cornerradius, height - cornerradius)) > cornerradius + border_thickness) discard;\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(cornerradius, height - cornerradius)) > cornerradius) {\n"
"           gl_FragColor = color;\n"
"           return;\n"
"       }\n"
"   }\n"
"   if(v_texcoord.x*width > width - cornerradius && v_texcoord.y*height > height - cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(width - cornerradius, height - cornerradius)) > cornerradius + border_thickness) discard;\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(width - cornerradius, height - cornerradius)) > cornerradius) {\n"
"           gl_FragColor = color;\n"
"           return;\n"
"       }\n"
"   }\n"
"   gl_FragColor = texture2D(tex, v_texcoord) * alpha;\n"
"}\n";



const GLchar custom_tex_fragment_src_rgbx[] =
"precision mediump float;\n"
"varying vec2 v_texcoord;\n"
"uniform sampler2D tex;\n"
"uniform float alpha;\n"
"\n"
"uniform float width;\n"
"uniform float height;\n"
"uniform float cornerradius;\n"
"uniform float border_thickness;\n"
"uniform vec4 color;\n"
"\n"
"void main() {\n"
"   if(v_texcoord.x*width < cornerradius && v_texcoord.y*height < cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(cornerradius, cornerradius)) > cornerradius) discard;\n"
"   }\n"
"   if(v_texcoord.x*width > width - cornerradius && v_texcoord.y*height < cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(width - cornerradius, cornerradius)) > cornerradius) discard;\n"
"   }\n"
"   if(v_texcoord.x*width < cornerradius && v_texcoord.y*height > height - cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(cornerradius, height - cornerradius)) > cornerradius) discard;\n"
"   }\n"
"   if(v_texcoord.x*width > width - cornerradius && v_texcoord.y*height > height - cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(width - cornerradius, height - cornerradius)) > cornerradius) discard;\n"
"   }\n"
"	gl_FragColor = vec4(texture2D(tex, v_texcoord).rgb, 1.0) * alpha;\n"
"}\n";

const GLchar custom_tex_fragment_src_external[] =
"#extension GL_OES_EGL_image_external : require\n\n"
"precision mediump float;\n"
"varying vec2 v_texcoord;\n"
"uniform samplerExternalOES texture0;\n"
"uniform float alpha;\n"
"\n"
"uniform float width;\n"
"uniform float height;\n"
"uniform float border_thickness;\n"
"uniform float cornerradius;\n"
"\n"
"void main() {\n"
"   if(v_texcoord.x*width < cornerradius && v_texcoord.y*height < cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(cornerradius, cornerradius)) > cornerradius) discard;\n"
"   }\n"
"   if(v_texcoord.x*width > width - cornerradius && v_texcoord.y*height < cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(width - cornerradius, cornerradius)) > cornerradius) discard;\n"
"   }\n"
"   if(v_texcoord.x*width < cornerradius && v_texcoord.y*height > height - cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(cornerradius, height - cornerradius)) > cornerradius) discard;\n"
"   }\n"
"   if(v_texcoord.x*width > width - cornerradius && v_texcoord.y*height > height - cornerradius){\n"
"       if(length(vec2(v_texcoord.x*width, v_texcoord.y*height) - vec2(width - cornerradius, height - cornerradius)) > cornerradius) discard;\n"
"   }\n"
"	gl_FragColor = texture2D(texture0, v_texcoord) * alpha;\n"
"}\n";

static const GLfloat verts[] = {
	1, 0, // top right
	0, 0, // top left
	1, 1, // bottom right
	0, 1, // bottom left
};

static GLuint compile_shader(struct wlr_gles2_renderer *renderer,
		GLuint type, const GLchar *src) {
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);

	GLint ok;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (ok == GL_FALSE) {
		glDeleteShader(shader);
		shader = 0;
	}

	return shader;
}

static GLuint link_program(struct wlr_gles2_renderer *renderer,
		const GLchar *vert_src, const GLchar *frag_src) {
	GLuint vert = compile_shader(renderer, GL_VERTEX_SHADER, vert_src);
	if (!vert) {
		goto error;
	}

	GLuint frag = compile_shader(renderer, GL_FRAGMENT_SHADER, frag_src);
	if (!frag) {
		glDeleteShader(vert);
		goto error;
	}

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glLinkProgram(prog);

	glDetachShader(prog, vert);
	glDetachShader(prog, frag);
	glDeleteShader(vert);
	glDeleteShader(frag);

	GLint ok;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (ok == GL_FALSE) {
		glDeleteProgram(prog);
		goto error;
	}

	return prog;

error:
	return 0;
}

struct wlr_gles2_renderer *gles2_get_renderer(
		struct wlr_renderer *wlr_renderer) {
	assert(wlr_renderer_is_gles2(wlr_renderer));
	return (struct wlr_gles2_renderer *)wlr_renderer;
}

struct wlr_gles2_texture *gles2_get_texture(
		struct wlr_texture *wlr_texture) {
	assert(wlr_texture_is_gles2(wlr_texture));
	return (struct wlr_gles2_texture *)wlr_texture;
}


//static GLfloat points[] = {
//	0.0f, 0.0f, 0.0f,  // 1
//	1.0f, 0.0f, 0.0f,
//	0.0f, 1.0f, 0.0f,
//	1.0f, 1.0f, 0.0f,  // 2
//	1.0f, 0.0f, 0.0f,
//	0.0f, 1.0f, 0.0f,
//	0.0f, 0.0f, 1.0f,  // 3
//	1.0f, 0.0f, 1.0f,
//	0.0f, 1.0f, 1.0f,
//	1.0f, 1.0f, 1.0f,  // 4
//	1.0f, 0.0f, 1.0f,
//	0.0f, 1.0f, 1.0f,
//	0.0f, 0.0f, 0.0f,  // 5
//	0.0f, 0.0f, 1.0f,
//	1.0f, 0.0f, 0.0f,
//	1.0f, 0.0f, 1.0f,  // 6
//	0.0f, 0.0f, 1.0f,
//	1.0f, 0.0f, 0.0f,
//	0.0f, 1.0f, 0.0f,  // 7
//	0.0f, 1.0f, 1.0f,
//	1.0f, 1.0f, 0.0f,
//	1.0f, 1.0f, 1.0f,  // 8
//	0.0f, 1.0f, 1.0f,
//	1.0f, 1.0f, 0.0f,
//	0.0f, 0.0f, 0.0f,  // 9
//	0.0f, 0.0f, 1.0f,
//	0.0f, 1.0f, 0.0f,
//	0.0f, 1.0f, 1.0f,  // 10
//	0.0f, 0.0f, 1.0f,
//	0.0f, 1.0f, 0.0f,
//	1.0f, 0.0f, 0.0f,  // 11
//	1.0f, 0.0f, 1.0f,
//	1.0f, 1.0f, 0.0f,
//	1.0f, 1.0f, 1.0f,  // 12
//	1.0f, 0.0f, 1.0f,
//	1.0f, 1.0f, 0.0f,
//};

static bool render_subtexture_with_matrix(
		struct sway_renderer *renderer, struct wlr_texture *wlr_texture,
		const struct wlr_fbox *box, const float matrix[static 9],
		float alpha, const struct wlr_box *display_box,
		float corner_radius, int border_thickness, float color[4]) {
	struct wlr_gles2_renderer *gles2_renderer =
		gles2_get_renderer(renderer->wlr_renderer);
	struct wlr_gles2_texture *texture =
		gles2_get_texture(wlr_texture);

	struct sway_renderer_shader *shader = NULL;

	switch (texture->target) {
	case GL_TEXTURE_2D:
		/* Same condition as below! Important! */
		if (texture->has_alpha) {
			shader = &renderer->shader_rgba;
		} else {
			shader = &renderer->shader_rgbx;
		}
		break;
	case GL_TEXTURE_EXTERNAL_OES:
		shader = &renderer->shader_ext;

		if (!gles2_renderer->exts.OES_egl_image_external) {
			wlr_log(WLR_ERROR, "Failed to render texture: "
				"GL_TEXTURE_EXTERNAL_OES not supported");
			return false;
		}
		break;
	default:
		abort();
	}

	float gl_matrix[9];
	wlr_matrix_multiply(gl_matrix, gles2_renderer->projection, matrix);

	// OpenGL ES 2 requires the glUniformMatrix3fv transpose parameter to be set
	// to GL_FALSE
	wlr_matrix_transpose(gl_matrix, gl_matrix);

	if (!texture->has_alpha && alpha == 1.0) {
		glDisable(GL_BLEND);
	} else {
		glEnable(GL_BLEND);
	}

	glBindTexture(texture->target, texture->tex);

	glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glUseProgram(shader->shader);

	glUniformMatrix3fv(shader->proj, 1, GL_FALSE, gl_matrix);
	glUniform1i(shader->tex, 0);
	glUniform1f(shader->alpha, alpha);
	glUniform1f(shader->width, display_box->width);
	glUniform1f(shader->height, display_box->height);
	glUniform1f(shader->cornerradius, corner_radius - 10);
	glUniform1f(shader->border_thickness, border_thickness + 2);
	glUniform4f(shader->color, color[0], color[1], color[2], color[3]);

	/*const*/ GLfloat x1 = box->x / wlr_texture->width;
	/*const*/ GLfloat y1 = box->y / wlr_texture->height;
	/*const*/ GLfloat x2 = (box->x + box->width) / wlr_texture->width;
	/*const*/ GLfloat y2 = (box->y + box->height) / wlr_texture->height;
	/*const*/ GLfloat texcoord[] = {
		x2, y1, // top right
		x1, y1, // top left
		x2, y2, // bottom right
		x1, y2, // bottom left
	};

	glVertexAttribPointer(shader->pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glVertexAttribPointer(shader->tex_attrib, 2, GL_FLOAT, GL_FALSE, 0, texcoord);

	glEnableVertexAttribArray(shader->pos_attrib);
	glEnableVertexAttribArray(shader->tex_attrib);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(shader->pos_attrib);
	glDisableVertexAttribArray(shader->tex_attrib);

	glBindTexture(texture->target, 0);

	/*******************/
	/*     Borders     */
	/*******************/

	glBindTexture(texture->target, texture->tex);

	glTexParameteri(texture->target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(texture->target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glUseProgram(shader->shader);

	glUniformMatrix3fv(shader->proj, 1, GL_FALSE, gl_matrix);
	glUniform1i(shader->tex, 0);
	glUniform1f(shader->alpha, alpha);
	glUniform1f(shader->width, display_box->width);
	glUniform1f(shader->height, display_box->height);
	glUniform1f(shader->cornerradius, corner_radius - 10);
	glUniform1f(shader->border_thickness, border_thickness + 2);
	glUniform4f(shader->color, color[0], color[1], color[2], color[3]);

	x1 = (box->x - 10) / wlr_texture->width;
	y1 = box->y / wlr_texture->height;
	x2 = (box->x + box->width) / wlr_texture->width;
	y2 = (box->y + box->height) / wlr_texture->height;
	const GLfloat texcoord2[] = {
		x2, y1, // top right
		x1, y1, // top left
		x2, y2, // bottom right
		x1, y2, // bottom left
	};

	glVertexAttribPointer(shader->pos_attrib, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glVertexAttribPointer(shader->tex_attrib, 2, GL_FLOAT, GL_FALSE, 0, texcoord2);

	glEnableVertexAttribArray(shader->pos_attrib);
	glEnableVertexAttribArray(shader->tex_attrib);

	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(shader->pos_attrib);
	glDisableVertexAttribArray(shader->tex_attrib);

	return true;
}
#endif

void sway_renderer_init(struct sway_renderer* renderer, struct sway_server* server){
    renderer->sway_server = server;

    renderer->wlr_renderer = server->renderer->wlr_renderer;
		renderer->wlr_renderer = wlr_renderer_autocreate(server->backend);
    assert(renderer->wlr_renderer);

    wlr_renderer_init_wl_display(renderer->wlr_renderer, server->wl_display);

    renderer->current = NULL;

#ifdef SWAY_CUSTOM_RENDERER
	struct wlr_gles2_renderer *r =
		gles2_get_renderer(renderer->wlr_renderer);

	//assert(wlr_egl_make_current(r->egl));
	eglMakeCurrent(r->egl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, r->egl->context);

	/* shader_rgba */
	renderer->shader_rgba.shader = 
		link_program(r, custom_tex_vertex_src, custom_tex_fragment_src_rgba);
	assert(renderer->shader_rgba.shader);

	renderer->shader_rgba.proj = glGetUniformLocation(renderer->shader_rgba.shader, "proj");
	renderer->shader_rgba.tex = glGetUniformLocation(renderer->shader_rgba.shader, "tex");
	renderer->shader_rgba.alpha = glGetUniformLocation(renderer->shader_rgba.shader, "alpha");
	renderer->shader_rgba.width = glGetUniformLocation(renderer->shader_rgba.shader, "width");
	renderer->shader_rgba.height = glGetUniformLocation(renderer->shader_rgba.shader, "height");
	renderer->shader_rgba.cornerradius = glGetUniformLocation(renderer->shader_rgba.shader, "cornerradius");
	renderer->shader_rgba.border_thickness = glGetUniformLocation(renderer->shader_rgba.shader, "border_thickness");
	renderer->shader_rgba.color = glGetUniformLocation(renderer->shader_rgba.shader, "color");

	renderer->shader_rgba.pos_attrib = glGetAttribLocation(renderer->shader_rgba.shader, "pos");
	renderer->shader_rgba.tex_attrib = glGetAttribLocation(renderer->shader_rgba.shader, "texcoord");

	/* shader_rgbx */
	renderer->shader_rgbx.shader = 
		link_program(r, custom_tex_vertex_src, custom_tex_fragment_src_rgbx);
	assert(renderer->shader_rgbx.shader);

	renderer->shader_rgbx.proj = glGetUniformLocation(renderer->shader_rgbx.shader, "proj");
	renderer->shader_rgbx.tex = glGetUniformLocation(renderer->shader_rgbx.shader, "tex");
	renderer->shader_rgbx.alpha = glGetUniformLocation(renderer->shader_rgbx.shader, "alpha");
	renderer->shader_rgbx.width = glGetUniformLocation(renderer->shader_rgbx.shader, "width");
	renderer->shader_rgbx.height = glGetUniformLocation(renderer->shader_rgbx.shader, "height");
	renderer->shader_rgbx.cornerradius = glGetUniformLocation(renderer->shader_rgbx.shader, "cornerradius");
	renderer->shader_rgbx.border_thickness = glGetUniformLocation(renderer->shader_rgbx.shader, "border_thickness");
	renderer->shader_rgbx.color = glGetUniformLocation(renderer->shader_rgbx.shader, "color");

	renderer->shader_rgbx.pos_attrib = glGetAttribLocation(renderer->shader_rgbx.shader, "pos");
	renderer->shader_rgbx.tex_attrib = glGetAttribLocation(renderer->shader_rgbx.shader, "texcoord");

	/* shader_ext */
	if(r->exts.OES_egl_image_external){
		renderer->shader_ext.shader =
			link_program(r, custom_tex_vertex_src, custom_tex_fragment_src_external);
		assert(renderer->shader_ext.shader);

		renderer->shader_ext.proj = glGetUniformLocation(renderer->shader_ext.shader, "proj");
		renderer->shader_ext.tex = glGetUniformLocation(renderer->shader_ext.shader, "tex");
		renderer->shader_ext.alpha = glGetUniformLocation(renderer->shader_ext.shader, "alpha");
		renderer->shader_ext.width = glGetUniformLocation(renderer->shader_ext.shader, "width");
		renderer->shader_ext.height = glGetUniformLocation(renderer->shader_ext.shader, "height");
		renderer->shader_ext.cornerradius = glGetUniformLocation(renderer->shader_ext.shader, "cornerradius");
		renderer->shader_ext.border_thickness = glGetUniformLocation(renderer->shader_ext.shader, "border_thickness");
		renderer->shader_ext.color = glGetUniformLocation(renderer->shader_ext.shader, "color");

		renderer->shader_ext.pos_attrib = glGetAttribLocation(renderer->shader_ext.shader, "pos");
		renderer->shader_ext.tex_attrib = glGetAttribLocation(renderer->shader_ext.shader, "texcoord");
	}

	wlr_egl_unset_current(r->egl);
#endif
}

void sway_renderer_destroy(struct sway_renderer* renderer){
    wlr_renderer_destroy(renderer->wlr_renderer);
}

void sway_renderer_begin(struct sway_renderer* renderer, struct sway_output* output){
	wlr_renderer_begin(renderer->wlr_renderer, output->wlr_output->width, output->wlr_output->height);
    renderer->current = output;
}

void sway_renderer_end(struct sway_renderer* renderer, pixman_region32_t* damage, struct sway_output* output){
    wlr_renderer_scissor(renderer->wlr_renderer, NULL);
    wlr_output_render_software_cursors(output->wlr_output, damage);
	wlr_renderer_end(renderer->wlr_renderer);

    renderer->current = NULL;
}

void sway_renderer_render_texture_at(struct sway_renderer *renderer,
																	 struct sway_output *output,
                                   pixman_region32_t *damage,
                                   struct wlr_texture *texture,
                                   struct wlr_box *box, double opacity,
                                   struct wlr_box *mask,
                                   double corner_radius,
								   int border_thickness,
								   float color[4]) {
	int ow, oh;
	wlr_output_transformed_resolution(output->wlr_output, &ow, &oh);

	enum wl_output_transform transform =
		wlr_output_transform_invert(output->wlr_output->transform);

    float matrix[9];
    wlr_matrix_project_box(matrix, box,
            WL_OUTPUT_TRANSFORM_NORMAL, 0,
            output->wlr_output->transform_matrix);

	struct wlr_fbox fbox = {
		.x = 0,
		.y = 0,
		.width = texture->width,
		.height = texture->height,
	};


	int nrects;
	pixman_box32_t* rects = pixman_region32_rectangles(damage, &nrects);
	for(int i=0; i<nrects; i++){
        struct wlr_box damage_box = {
            .x = rects[i].x1,
            .y = rects[i].y1,
            .width = rects[i].x2 - rects[i].x1,
            .height = rects[i].y2 - rects[i].y1
        };
		struct wlr_box inters;
		wlr_box_intersection(&inters, box, &damage_box);
		if(wlr_box_empty(&inters)) continue;

		wlr_box_transform(&inters, &inters, transform, ow, oh);
        wlr_renderer_scissor(renderer->wlr_renderer, &inters);

#ifdef SWAY_CUSTOM_RENDERER
		render_subtexture_with_matrix(
				renderer,
				texture,
				&fbox, matrix, opacity,
				box,
				corner_radius, border_thickness, color);
#else
		wlr_render_subtexture_with_matrix(
				renderer->wlr_renderer,
				texture,
				&fbox, matrix, opacity);
#endif
	}
}
