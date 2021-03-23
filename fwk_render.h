// naive rendering framework
// - rlyeh, public domain
//
// IQM skeletal meshes by @lsalzman (public domain) - https://bit.ly/2OQh0Me
// SH code by @ands (public domain) - https://github.com/ands/spherical_harmonics_playground

#ifndef RENDER_H
#define RENDER_H

typedef unsigned handle; // GLuint

// -----------------------------------------------------------------------------
// colors

uint32_t rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a );
float    alpha( uint32_t rgba );

#define RGB_HEX(rgb) (255<<24|((0x##rgb>>16)&255)<<16|((0x##rgb>>8)&255)<<8|((0x##rgb>>0)&255)<<0)

#define BLACK   RGB_HEX(000000)
#define RED     RGB_HEX(FF004D)
#define GREEN   RGB_HEX(00B543)
#define BLUE    RGB_HEX(065AB5)
#define ORANGE  RGB_HEX(FF6C24)
#define PURPLE  RGB_HEX(7E2553)
#define YELLOW  RGB_HEX(FFEC27)
#define WHITE   RGB_HEX(FFF1E8)
#define GRAY    RGB_HEX(725158)

// -----------------------------------------------------------------------------
// images

enum {
    IMAGE_R    = 0x01000,
    IMAGE_RG   = 0x02000,
    IMAGE_RGB  = 0x04000,
    IMAGE_RGBA = 0x08000,
    IMAGE_FLIP = 0x10000,
};

typedef struct image_t {
    union { unsigned x, w; };
    union { unsigned y, h; };
    union { unsigned n, comps; };
    union { void *pixels; unsigned char *pixels8; unsigned short *pixels16; unsigned *pixels32; float *pixelsf; };
} image_t;

image_t image(const char *pathfile, int flags);
image_t image_from_mem(const char *ptr, int len, int flags);
void    image_destroy(image_t *img);

// -----------------------------------------------------------------------------
// textures

enum {
    // UNIT[0..7]

    TEXTURE_BC1 = 8,  // DXT1, RGB with 8:1 compression ratio (+ optional 1bpp for alpha)
    TEXTURE_BC2 = 16, // DXT3, RGBA with 4:1 compression ratio (BC1 for RGB + 4bpp for alpha)
    TEXTURE_BC3 = 32, // DXT5, RGBA with 4:1 compression ratio (BC1 for RGB + BC4 for A)
//  TEXTURE_BC4,     // Alpha

    TEXTURE_NEAREST = 0,
    TEXTURE_LINEAR = 64,
    TEXTURE_MIPMAPS = 128,

    TEXTURE_EDGE = 0,
    TEXTURE_BORDER = 0x100,
    TEXTURE_REPEAT = 0x200,

    TEXTURE_BYTE = 0,
    TEXTURE_FLOAT = 0x400,

    TEXTURE_COLOR = 0,
    TEXTURE_DEPTH = 0x800,

    TEXTURE_R = IMAGE_R,
    TEXTURE_RG = IMAGE_RG,
    TEXTURE_RGB = IMAGE_RGB,
    TEXTURE_RGBA = IMAGE_RGBA,
    TEXTURE_FLIP = IMAGE_FLIP,

    // @fixme
    TEXTURE_SRGB = 1 << 24,
    TEXTURE_BGR = 1 << 25,
    TEXTURE_ARRAY = 1 << 26,
};

typedef struct {
    union { unsigned x, w; };
    union { unsigned y, h; };
    union { unsigned n, bpp; };
    handle id;
    unsigned flags;
} texture_t;

texture_t texture(const char* filename, int flags);
texture_t texture_from_mem(const char* ptr, int len, int flags);
texture_t texture_create(unsigned w, unsigned h, unsigned n, void *pixels, int flags);
texture_t texture_checker();
void      texture_destroy(texture_t *t);
// textureLod(filename, dir, lod);
//void texture_add_loader( int(*loader)(const char *filename, int *w, int *h, int *bpp, int reqbpp, int flags) );
unsigned  texture_update(texture_t *t, unsigned w, unsigned h, unsigned n, void *pixels, int flags);

// -----------------------------------------------------------------------------
// fullscreen quads

void  fullscreen_rgb_quad( texture_t texture_rgb, float gamma );
void  fullscreen_ycbcr_quad( texture_t texture_YCbCr[3], float gamma );

// -----------------------------------------------------------------------------
// sprites

void sprite( texture_t texture, float px, float py, float pz, float rot );
void sprite_ex( texture_t texture,
    float px, float py, float pz, float rotation, // position(x,y,depth sort), angle
    float ox, float oy, float sx, float sy,       // offset(x,y), scale(x,y)
    int additive, uint32_t rgba,                  // is_additive, tint color
    float frame, float xcells, float ycells       // frame_number in a 8x4 spritesheet
);

// -----------------------------------------------------------------------------
// cubemaps

typedef struct cubemap_t {
    unsigned id;    // texture id
    vec3 sh[9];     // precomputed spherical harmonics coefficients
} cubemap_t;

cubemap_t  cubemap( const image_t image, int flags ); // 1 equirectangular panorama
cubemap_t  cubemap6( const image_t images[6], int flags ); // 6 cubemap faces
void       cubemap_destroy(cubemap_t *c);
cubemap_t* cubemap_get_active();

// -----------------------------------------------------------------------------
// fbos

unsigned fbo( unsigned texture_color, unsigned texture_depth, int wr_flags );
void     fbo_bind(unsigned id);
void     fbo_unbind();
void     fbo_destroy(unsigned id);

// -----------------------------------------------------------------------------
// shadowmaps

unsigned shadowmap(unsigned w, unsigned h, int flags);

// -----------------------------------------------------------------------------
// shaders

unsigned shader(const char *vs, const char *fs, const char *attribs, const char *fragcolor);
unsigned shader_bind(unsigned program);
    void shader_int(const char *uniform, int i);
    void shader_float(const char *uniform, float f);
    void shader_vec2(const char *uniform, vec2 v);
    void shader_vec3(const char *uniform, vec3 v);
    void shader_vec4(const char *uniform, vec4 v);
    void shader_mat44(const char *uniform, mat44 m);
    void shader_texture(const char *sampler, unsigned texture, unsigned unit);
unsigned shader_get_active();
void     shader_destroy(unsigned shader);

// -----------------------------------------------------------------------------
// meshes (@fixme: deprecate?)

enum MESH_FLAGS {
    MESH_STATIC = 0, // STATIC, DYNAMIC, STREAM // zero|single|many updates per frame
    MESH_STREAM = 1,
    MESH_TRIANGLE_STRIP = 2,
};

typedef struct mesh_t {
    handle vao, vbo, ibo;
    unsigned vertex_count;
    unsigned index_count;
    unsigned flags;
} mesh_t;

mesh_t mesh_create(const char *format, int vertex_stride,int vertex_count,const void *interleaved_vertex_data, int index_count,const void *index_data, int flags);
  void mesh_upgrade(mesh_t *m, const char *format, int vertex_stride,int vertex_count,const void *interleaved_vertex_data, int index_count,const void *index_data, int flags);
  void mesh_push_state(mesh_t *m, unsigned program, unsigned texture_id, float model[16], float view[16], float proj[16], unsigned billboard);
  void mesh_pop_state(mesh_t *m);
  void mesh_render(mesh_t *m);
  void mesh_destroy(mesh_t *m);
  aabb mesh_bounds(mesh_t *m);

// -----------------------------------------------------------------------------
// materials (@todo)
//
// typedef struct material_t {
//     const char *name;
//     texture_t texture;
//     uint32_t color;
// } material_t;

// -----------------------------------------------------------------------------
// models

enum {
    MODEL_NO_ANIMATIONS = 1,
    MODEL_NO_MESHES = 2,
    MODEL_NO_TEXTURES = 4,
};

typedef struct model_t {
    struct iqm_t *iqm;
    unsigned num_meshes;
    unsigned num_triangles;
    unsigned num_joints; // num_poses;
    unsigned num_anims;
    unsigned num_frames;
    float curframe;
    mat44 pivot;
} model_t;

model_t  model(const char *filename, int flags);
model_t  model_from_mem(const void *mem, int sz, int flags);
float    model_animate(model_t, float curframe);
float    model_animate_clip(model_t, float curframe, int minframe, int maxframe, bool loop);
aabb     model_aabb(model_t, mat44 transform);
void     model_render(model_t, mat44 mvp);
void     model_destroy(model_t);

// -----------------------------------------------------------------------------
// skyboxes

typedef struct skybox_t {
    handle program;
    mesh_t geometry;
    cubemap_t cubemap;
} skybox_t;

skybox_t skybox(const char *panorama_or_cubemap_folder, int flags);
int      skybox_push_state(skybox_t *sky, float mvp[16]);
int      skybox_pop_state(skybox_t *sky);
void     skybox_destroy(skybox_t *sky);

// -----------------------------------------------------------------------------
// post-processes

void     viewport_color(vec3 color);
void     viewport_clear(bool color, bool depth);
void     viewport_clip(vec2 from, vec2 to);

void     fx_load(const char *file);
void     fx_begin();
void     fx_end();
void     fx_enable(int pass, int enabled);
int      fx_enabled(int pass);
void     fx_enable_all(int enabled);
char *   fx_name(int pass);

// -----------------------------------------------------------------------------
// utils

void*    screenshot(unsigned components); // 3 RGB, -3 BGR, 4 RGBA, -4 BGRA

#endif

#ifdef RENDER_C
#pragma once

// -----------------------------------------------------------------------------
// opengl

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3

#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B
#define GL_DEBUG_SOURCE_API               0x8246
#define GL_DEBUG_TYPE_ERROR               0x824C
//
void glDebugCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int32_t length, const char * message, void * userdata) {
    // whitelisted codes (also: 131169, 131204).
    if( id == 131154 ) return; // Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
    if( id == 131185 ) return; // Buffer object 2 (bound to GL_ELEMENT_ARRAY_BUFFER_ARB, usage hint is GL_STATIC_DRAW) will use VIDEO memory as the source for buffer object operations
    if( id == 131218 ) return; // Program/shader state performance warning: Vertex shader in program 9 is being recompiled based on GL state.
    if( id == 2 ) return; // INFO: API_ID_RECOMPILE_FRAGMENT_SHADER performance warning has been generated. Fragment shader recompiled due to state change. [ID: 2]

    const char * GL_ERROR_SOURCE[] = { "API", "WINDOW SYSTEM", "SHADER COMPILER", "THIRD PARTY", "APPLICATION", "OTHER" };
    const char * GL_ERROR_SEVERITY[] = { "HIGH", "MEDIUM", "LOW", "NOTIFICATION" };
    const char * GL_ERROR_TYPE[] = { "ERROR", "DEPRECATED BEHAVIOR", "UNDEFINED DEHAVIOUR", "PORTABILITY", "PERFORMANCE", "OTHER" };

    severity = severity == GL_DEBUG_SEVERITY_NOTIFICATION ? 3 : severity - GL_DEBUG_SEVERITY_HIGH;
    source = source - GL_DEBUG_SOURCE_API;
    type = type - GL_DEBUG_TYPE_ERROR;

    PRINTF( "!%s [ID: %u]\n", message, id );
//    PANIC( "!%s [ID: %u]\n", message, id );
}
void glDebugEnable() {
    ONCE {
    typedef void (*GLDEBUGPROC)(uint32_t, uint32_t, uint32_t, uint32_t, int32_t, const char *, const void *);
    typedef void (*GLDEBUGMESSAGECALLBACKPROC)(GLDEBUGPROC, const void *);
    void (*glDebugMessageCallback)(GLDEBUGPROC, const void *) = (GLDEBUGMESSAGECALLBACKPROC)glfwGetProcAddress("glDebugMessageCallback");
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    glDebugMessageCallback((GLDEBUGPROC)glDebugCallback, NULL);
    }
}

void glNewFrame() {
    glViewport(0, 0, window_width(), window_height());
    //glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
}

// ----------------------------------------------------------------------------
// embedded shaders (@fixme: promote to files?)

static const char* const vertex_shader_332_32 =
    ""
    //"uniform mat4 u_model, u_view, u_proj;\n"
    "uniform mat4 u_mvp;\n"

    "in vec3 att_position;\n"
    "in vec3 att_normal;\n"
    "in vec2 att_texcoord;\n"
    "out vec3 v_normal;\n"
    "out vec2 v_texcoord;\n"

    "void main() {\n"
    //"    gl_Position = proj * view * model * vec4(att_position, 1.0);\n"
    "    gl_Position = u_mvp * vec4(att_position, 1.0);\n"
    "    v_normal = att_normal;\n"
    "    v_texcoord = att_texcoord;\n"
    "}";

static const char* const fragment_shader_32_4 =
    ""
    "uniform mat4 M;\n"
    "uniform sampler2D u_texture2d;\n"
    "uniform vec3 u_coefficients_sh[9];\n"
    "uniform bool u_textured = true;\n"
    "uniform bool u_lit = false;\n"

    "#ifdef RIM\n"
    "in vec3 v_position;\n"
    "#endif\n"
    "in vec3 v_normal;\n"
    "in vec2 v_texcoord;\n"
    "out vec4 fragcolor;\n"

    "void main() {\n"
    "    vec3 n = /*normalize*/(v_normal);\n"

    "    vec3 SHLightResult[9];\n"
    "    SHLightResult[0] =  0.282095f * u_coefficients_sh[0];\n"
    "    SHLightResult[1] = -0.488603f * u_coefficients_sh[1] * n.y;\n"
    "    SHLightResult[2] =  0.488603f * u_coefficients_sh[2] * n.z;\n"
    "    SHLightResult[3] = -0.488603f * u_coefficients_sh[3] * n.x;\n"
    "    SHLightResult[4] =  1.092548f * u_coefficients_sh[4] * n.x * n.y;\n"
    "    SHLightResult[5] = -1.092548f * u_coefficients_sh[5] * n.y * n.z;\n"
    "    SHLightResult[6] =  0.315392f * u_coefficients_sh[6] * (3.0f * n.z * n.z - 1.0f);\n"
    "    SHLightResult[7] = -1.092548f * u_coefficients_sh[7] * n.x * n.z;\n"
    "    SHLightResult[8] =  0.546274f * u_coefficients_sh[8] * (n.x * n.x - n.y * n.y);\n"
    "    vec3 result = vec3(0.0);\n"
    "    for (int i = 0; i < 9; ++i)\n"
    "        result += SHLightResult[i];\n"

    // lighting
    "    if(u_textured && u_lit) fragcolor = texture(u_texture2d, v_texcoord) * vec4(result, 1.0);\n" // diffuse + lit
    "    else if(u_textured)     fragcolor = texture(u_texture2d, v_texcoord);\n"  // diffuse only
    "    else                    fragcolor = vec4(result, 1.0);\n" // lit only

    // rimlight
    "#ifdef RIM\n"
    "   {vec3 n = normalize(mat3(M) * v_normal);  // convert normal to view space\n"
    "    vec3 p = (M * vec4(v_position,1.0)).xyz; // convert position to view space\n"
    "    vec3 v = normalize(-p);                  // eye vector\n"
    "    float rim = 1.0 - max(dot(v, n), 0.0);   // rimlight\n"
    "    rim = smoothstep(1.0-0.01, 1.0, rim);    // intensity (0.01)\n"
    "    fragcolor += vec4(0.0, 0.0, rim, 1.0);}  // blue\n"
    "#endif\n"

    "}\n";

static const char * const skybox_vs_3_3 =
    ""
    "uniform mat4 u_mvp;\n"

    "in vec3 att_position;\n"
    "out vec3 v_direction;\n"

    "void main() {\n"
    "    vec4 position = u_mvp * vec4(att_position, 0.0);\n"
    "    gl_Position = position.xyww;\n"
    "    v_direction = att_position;\n"
    "}\n";

static const char * const skybox_fs_3_4 =
    ""
    "uniform samplerCube u_cubemap;\n"

    "in vec3 v_direction;\n"
    "out vec4 fragcolor;\n"

    "void main() {\n"
    "    fragcolor = vec4(texture(u_cubemap, v_direction).rgb, 1.0);\n"
    "}\n";

// ----------------------------------------------------------------------------
// shaders

void shader_print(const char *source) {
    for(int line = 0, i = 0; source[i] > 0; ) {
        printf("\t%03d: ", line+1);
        while( source[i] >= 32 || source[i] == '\t' ) fputc(source[i++], stdout);
        while( source[i] > 0 && source[i] < 32 ) line += source[i++] == '\n';
        puts("");
    }
}

static
GLuint shader_compile( GLenum type, const char *source ) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char **)&source, NULL);
    glCompileShader(shader);

    GLint status = GL_FALSE, length;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if( status == GL_FALSE ) {
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        ASSERT(length < 2048);
        char buf[2048] = { 0 };
        glGetShaderInfoLog(shader, length, NULL, buf);

        // dump log with line numbers
        shader_print( source );
        PANIC("ERROR: shader_compile(): %s\n%s\n", type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", buf);
        return 0;
    }

    return shader;
}

unsigned shader(const char *vs, const char *fs, const char *attribs, const char *fragcolor) {
    PRINTF("Compiling shader\n");

    fs = fs[0] == '#' && fs[1] == 'v' ? fs : stringf("#version 130\n%s", fs);
    vs = vs[0] == '#' && vs[1] == 'v' ? vs : stringf("#version 130\n%s", vs);

    GLuint vert = shader_compile(GL_VERTEX_SHADER, vs);
    GLuint frag = shader_compile(GL_FRAGMENT_SHADER, fs);
  //GLuint geom = shader_compile(GL_GEOMETRY_SHADER, gs);
    GLuint program = 0;

    if( vert && frag ) {
        program = glCreateProgram();

        glAttachShader(program, vert);
        glAttachShader(program, frag);
        // glAttachShader(program, geom);

        for( int i = 0; attribs && attribs[0]; ++i ) {
            char attrib[128] = {0};
            sscanf(attribs, "%127[^,]", attrib);
            while( attribs[0] && attribs[0] != ',' ) { attribs++; }
            while( attribs[0] && attribs[0] == ',' ) { attribs++; break; }
            if(!attrib[0]) continue;
            glBindAttribLocation(program, i, attrib);
            PRINTF("Shader.attribute[%d]=%s\n", i, attrib);
        }

        glBindFragDataLocation(program, 0, fragcolor);
        glLinkProgram(program);

        GLint status = GL_FALSE, length;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            ASSERT(length < 2048);
            char buf[2048] = { 0 };
            glGetProgramInfoLog(program, length, NULL, buf);
            shader_print(vs);
            shader_print(fs);
            PANIC("ERROR: shader(): Shader/program link: %s\n", buf);
            return 0;
        }

        // glDetachShader(program, vert);
        // glDetachShader(program, frag);
        // glDetachShader(program, geom);

        glDeleteShader(vert);
        glDeleteShader(frag);
        // glDeleteShader(geom);

#ifdef DEBUG_SHADER
        PRINTF("Shader #%d:\n", program);
        shader_print(vs);
        shader_print(fs);
#endif
    }

    return program;
}

void shader_destroy(unsigned program){
    glDeleteProgram(program);
}

unsigned last_shader = -1;
static
int shader_uniform(const char *name) {
    int ret = glGetUniformLocation(last_shader, name);
    if( ret < 0 ) fprintf(stderr, "cannot find uniform '%s' in shader program %d\n", name, (int)last_shader );
    return ret;
}
unsigned shader_get_active() { return last_shader; }
unsigned shader_bind(unsigned program) { unsigned ret = last_shader; return glUseProgram(last_shader = program), ret; }
void shader_int(const char *uniform, int i)     { glUniform1i(shader_uniform(uniform), i); }
void shader_float(const char *uniform, float f) { glUniform1f(shader_uniform(uniform), f); }
void shader_vec2(const char *uniform, vec2 v)   { glUniform2fv(shader_uniform(uniform), 1, &v.x); }
void shader_vec3(const char *uniform, vec3 v)   { glUniform3fv(shader_uniform(uniform), 1, &v.x); }
void shader_vec4(const char *uniform, vec4 v)   { glUniform4fv(shader_uniform(uniform), 1, &v.x); }
void shader_mat44(const char *uniform, mat44 m) { glUniformMatrix4fv(shader_uniform(uniform), 1, GL_FALSE/*GL_TRUE*/, m); }
void shader_texture(const char *sampler, unsigned texture, unsigned unit) { glBindTexture(GL_TEXTURE_2D, texture); glActiveTexture(GL_TEXTURE0 + unit); glUniform1i(shader_uniform(sampler), unit); }
void shader_cubemap(const char *sampler, unsigned texture) { glUniform1i(shader_uniform(sampler), 0); glBindTexture(GL_TEXTURE_CUBE_MAP, texture); }

// -----------------------------------------------------------------------------
// colors

uint32_t rgba( uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
    return r << 24 | g << 16 | b << 8 | a;
}
float alpha( uint32_t rgba ) {
    return ( rgba & 255 ) / 255.f;
}

// -----------------------------------------------------------------------------
// images

image_t image_create(int x, int y, int flags) {
    int n = 3; // defaults to RGB
    if(flags & IMAGE_R) n = 1;
    if(flags & IMAGE_RG) n = 2;
    if(flags & IMAGE_RGB) n = 3;
    if(flags & IMAGE_RGBA) n = 4;
    image_t img; img.x = x; img.y = y; img.n = n;
    img.pixels = REALLOC(0, x * y * n ); // @fixme: image_destroy() requires stbi allocator to match REALLOC
    return img;
}

image_t image_from_mem(const char *data, int size, int flags) {
    image_t img = {0};
    if( data && size ) {
        stbi_set_flip_vertically_on_load(flags & IMAGE_FLIP ? 1 : 0);

        int n = 0;
        if(flags & IMAGE_R) n = 1;
        if(flags & IMAGE_RG) n = 2;
        if(flags & IMAGE_RGB) n = 3;
        if(flags & IMAGE_RGBA) n = 4;
        img.pixels = stbi_load_from_memory(data, size, &img.x,&img.y,&img.n, n);
        if( img.pixels ) {
            PRINTF("Loaded image (%dx%d %.*s->%.*s)\n",img.w,img.h,img.n,"RGBA",n?n:img.n,"RGBA");
        } else {
            // PANIC("Error loading image (%s)\n", pathfile);
        }
        img.n = n ? n : img.n;
    }
    return img;
}

image_t image(const char *pathfile, int flags) {
    int size = 0;
    char *data = file_load(file_find(pathfile/*stringf("%s", pathfile)*/), &size);
#if 1
    if( !size ) data = file_load(file_find(stringf("%s.png",pathfile)), &size);
    if( !size ) data = file_load(file_find(stringf("%s.jpg",pathfile)), &size);
    if( !size ) data = file_load(file_find(stringf("%s.tga",pathfile)), &size);
    if( !size ) data = file_load(file_find(stringf("%s.jpg.png",pathfile)), &size);
    if( !size ) data = file_load(file_find(stringf("%s.tga.png",pathfile)), &size);
    if( !size ) data = file_load(file_find(stringf("%s.png.jpg",pathfile)), &size);
    if( !size ) data = file_load(file_find(stringf("%s.tga.jpg",pathfile)), &size);
#endif
    return image_from_mem(data, size, flags);
}

void image_destroy(image_t *img) {
    if(img->pixels) stbi_image_free(img->pixels);
    img->pixels = 0; // *img = (image_t){0}; // do not clear fields yet. might be useful in the future.
}

// bilinear interpolation (uv must be in image coords, range [0..w-1,0..h-1])
static
vec3 bilinear(image_t in, vec2 uv) {
    float w = in.x, h = in.y, u = uv.x, v = uv.y;
    float u1 = (int)u, v1 = (int)v, u2 = minf(u1+1, w-1), v2 = minf(v1+1, h-1);
    float c1 = u - u1, c2 = v - v1;
    uint8_t *p1 = &in.pixels8[ in.n * (int)(u1 + v1 * in.w) ];
    uint8_t *p2 = &in.pixels8[ in.n * (int)(u2 + v1 * in.w) ];
    uint8_t *p3 = &in.pixels8[ in.n * (int)(u1 + v2 * in.w) ];
    uint8_t *p4 = &in.pixels8[ in.n * (int)(u2 + v2 * in.w) ];
    vec3 A = vec3( p1[0], p1[1], p1[2] );
    vec3 B = vec3( p2[0], p2[1], p2[2] );
    vec3 C = vec3( p3[0], p3[1], p3[2] );
    vec3 D = vec3( p4[0], p4[1], p4[2] );
    return mix3(mix3(A, B, c1), mix3(C, D, c1), c2);
}

// -----------------------------------------------------------------------------
// textures

unsigned texture_update(texture_t *t, unsigned w, unsigned h, unsigned n, void *pixels, int flags) {
    ASSERT( t && t->id );
    ASSERT( n <= 4 );
    GLuint pixel_types[] = { GL_RED, GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_R32F, GL_R32F, GL_RG32F, GL_RGB32F, GL_RGBA32F };
    GLenum pixel_storage = flags & TEXTURE_FLOAT ? GL_FLOAT : GL_UNSIGNED_BYTE;
    GLuint pixel_type = pixel_types[ n ];
    GLuint texel_type = pixel_types[ n + 5 * !!(flags & TEXTURE_FLOAT) ];
    GLenum wrap = GL_CLAMP_TO_EDGE;
    GLenum min_filter = GL_NEAREST, mag_filter = GL_NEAREST;
//    GLfloat color = (flags&7)/7.f, border_color[4] = { color, color, color, 1.f };

    if( flags & TEXTURE_BGR )  if( pixel_type == GL_RGB )  pixel_type = GL_BGR;
    if( flags & TEXTURE_BGR )  if( pixel_type == GL_RGBA ) pixel_type = GL_BGRA;
    if( flags & TEXTURE_SRGB ) if( texel_type == GL_RGB )  texel_type = GL_SRGB;
    if( flags & TEXTURE_SRGB ) if( texel_type == GL_RGBA ) texel_type = GL_SRGB_ALPHA;

    if( flags & TEXTURE_BC1 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    if( flags & TEXTURE_BC2 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    if( flags & TEXTURE_BC3 ) texel_type = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    if( flags & TEXTURE_DEPTH ) texel_type = pixel_type = GL_DEPTH_COMPONENT;

    if( flags & TEXTURE_REPEAT ) wrap = GL_REPEAT;
    if( flags & TEXTURE_BORDER ) wrap = GL_CLAMP_TO_BORDER;
    if( flags & TEXTURE_LINEAR ) min_filter = GL_LINEAR, mag_filter = GL_LINEAR;
    if( flags & TEXTURE_MIPMAPS  ) min_filter = flags & TEXTURE_LINEAR ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
    if( flags & TEXTURE_MIPMAPS  ) mag_filter = flags & TEXTURE_LINEAR ? GL_LINEAR : GL_NEAREST;

    if( 0 ) { // flags & TEXTURE_PREMULTIPLY_ALPHA )
        uint8_t *p = pixels;
        if(n == 2) for( unsigned i = 0; i < 2*w*h; i += 2 ) {
            p[i] = (p[i] * p[i+1] + 128) >> 8;
        }
        if(n == 4) for( unsigned i = 0; i < 4*w*h; i += 4 ) {
            p[i+0] = (p[i+0] * p[i+3] + 128) >> 8;
            p[i+1] = (p[i+1] * p[i+3] + 128) >> 8;
            p[i+2] = (p[i+2] * p[i+3] + 128) >> 8;
        }
    }

GLenum texture_type = t->flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D; // @fixme: test GL_TEXTURE_2D_ARRAY

//glPixelStorei( GL_UNPACK_ALIGNMENT, n < 4 ? 1 : 4 ); // for framebuffer reading
//glActiveTexture(GL_TEXTURE0 + (flags&7));
    glBindTexture(texture_type, t->id);
    glTexImage2D(texture_type, 0, texel_type, w, h, 0, pixel_type, pixel_storage, pixels);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, wrap);
    glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, mag_filter);
    if( flags & TEXTURE_DEPTH )   glTexParameteri(texture_type, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    if( flags & TEXTURE_DEPTH )   glTexParameteri(texture_type, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
//  if( flags & TEXTURE_BORDER )  glTexParameterfv(texture_type, GL_TEXTURE_BORDER_COLOR, border_color);
    if( flags & TEXTURE_MIPMAPS ) glGenerateMipmap(texture_type);

if( flags & TEXTURE_MIPMAPS ) {
        GLfloat max_aniso = 0;
//        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_aniso);
max_aniso = 4;
//        glTexParameterf(texture_type, GL_TEXTURE_MAX_ANISOTROPY, max_aniso);
}

    // glBindTexture(texture_type, 0); // do not unbind. current code expects texture to be bound at function exit

    t->w = w;
    t->h = h;
    t->n = n;
    t->flags = flags;
    return t->id;
}

texture_t texture_create(unsigned w, unsigned h, unsigned n, void *pixels, int flags) {
    texture_t texture = {0};
    glGenTextures( 1, &texture.id );
    texture_update( &texture, w, h, n, pixels, flags );
    return texture;
}

texture_t texture_checker() {
    static texture_t texture = {0};
    if( !texture.id ) {
#if 0
        float pixels[] = { 1,0.5,0.5,1 };
        texture = texture_create(2,2,1, pixels, TEXTURE_FLOAT|TEXTURE_MIPMAPS|TEXTURE_REPEAT|TEXTURE_BORDER);
#else
        unsigned char *pixels = REALLOC(0, 256*256*3);
        for (int y = 0, i = 0; y < 256; y++) {
            for (int x = 0; x < 256; x++) {
                int v = 255 * !!((x ^ y) & 0x8);
                pixels[i++] = v * (x > 127);
                pixels[i++] = v * (y > 127);
                pixels[i++] = v * (x > y);
            }
        }
        texture = texture_create(256,256,3, pixels, TEXTURE_RGB|TEXTURE_MIPMAPS|TEXTURE_REPEAT|TEXTURE_BORDER);
        FREE(pixels);
#endif
    }
    return texture;
}

texture_t texture_from_mem(const char *ptr, int len, int flags) {
    image_t img = image_from_mem(ptr, len, flags);
    if( img.pixels ) {
        texture_t t = texture_create(img.x, img.y, img.n, img.pixels, flags);
        image_destroy(&img);
        return t;
    }
    return texture_checker();
}

texture_t texture(const char *pathfile, int flags) {
    // PRINTF("Loading file %s\n", pathfile);
    image_t img = image(pathfile, flags);
    if( img.pixels ) {
        texture_t t = texture_create(img.x, img.y, img.n, img.pixels, flags);
        image_destroy(&img);
        return t;
    }
    return texture_checker();
}

void texture_destroy( texture_t *t ) {
    if(t->id) glDeleteTextures(1, &t->id);
    t->id = 0;
}

unsigned shadowmap( unsigned w, unsigned h, int flags ) {
    unsigned id = texture_create(w, h, 1, NULL, TEXTURE_DEPTH | TEXTURE_FLOAT | TEXTURE_LINEAR | TEXTURE_BORDER ).id;
    return id;
}


// usage: bind empty vao & commit call for 6 (quad) or 3 vertices (tri).
// ie, glBindVertexArray(empty_vao); glDrawArrays(GL_TRIANGLES, 0, 3);

const char *fullscreen_quad_vertex_shader(int flip);
const char *fullscreen_triangle_vertex_shader();

#if 1
const char *fullscreen_quad_vertex_shader(int flip) {
    static threadlocal char buf[512] = {0};
    snprintf(buf, 512,
        ""
        "out vec2 uv;\n"
        "void main() {\n"
        "    float x = float(((uint(gl_VertexID) + 2u) / 3u)%%2u); \n"
        "    float y = float(((uint(gl_VertexID) + 1u) / 3u)%%2u); \n"
        "    gl_Position = vec4(-1.0 + x*2.0, 0.0%c(-1.0+y*2.0), 0.0, 1.0);\n" // normal(0+),flipped(0-)
        "    uv = vec2(x, y);\n" // normal(y),flipped(1.0-y)
        "}\n", flip ? '-':'+'
    );
    return buf;
}
#else
const char *fullscreen_quad_vertex_shader() {
    return
        ""
        "out vec2 uv;\n"
        "void main() {\n"
        "   float x = gl_VertexID / 2;\n"
        "   float y = gl_VertexID % 2;\n"
        "   uv = vec2(x, y);\n"
        "   gl_Position = vec4(2.0*uv - 1.0, 0.0, 1.0);\n"
        "}\n";
}
#endif

const char *fullscreen_triangle_vertex_shader() {
    return
        ""
        "out vec2 texcoord;\n"
        "void main() {\n"
        "   texcoord = vec2( (gl_VertexID << 1) & 2, gl_VertexID & 2 );\n"
        "   gl_Position = vec4( texCoord * 2.0 - 1.0, 0.0, 1.0 );\n"
        "}\n";
}

void fullscreen_rgb_quad( texture_t texture, float gamma ) {
    static int program = -1, vao = -1, u_inv_gamma = -1;
    if( program < 0 ) {
        const char* vs = fullscreen_quad_vertex_shader(1);
        const char* fs =
            ""
            "in vec2 uv;\n"
            "out vec4 fragcolor;\n"
            "uniform sampler2D texture0; /*unit0*/\n"
            "uniform float u_inv_gamma;\n"
            "void main() {\n"
            "    vec4 texel = texture( texture0, uv );\n"
            "    fragcolor = texel;\n"
            "    fragcolor.rgb = pow( fragcolor.rgb, vec3( u_inv_gamma ) );\n" // defaults: 1.0/2.2 gamma
            "}\n";

        program = shader(vs, fs, "", "fragcolor" );
        u_inv_gamma = glGetUniformLocation(program, "u_inv_gamma");
        glGenVertexArrays( 1, &vao );
    }

    GLenum texture_type = texture.flags & TEXTURE_ARRAY ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_inv_gamma, 1.0f / (gamma + !gamma) );

    glBindVertexArray( vao );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( texture_type, texture.id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("drawcalls", +1);
    profile_incstat("triangles", +2);

    glBindTexture( texture_type, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

void fullscreen_ycbcr_quad( texture_t textureYCbCr[3], float gamma ) {
    static int program = -1, vao = -1, u_gamma = -1, uy = -1, ucb = -1, ucr = -1;
    if( program < 0 ) {
        const char* vs = fullscreen_quad_vertex_shader(1);
        const char* fs =
            ""
            "in vec2 uv;\n"
            "out vec4 fragcolor;\n"
            "uniform sampler2D u_texture_y;  /*unit0*/\n"
            "uniform sampler2D u_texture_cb; /*unit1*/\n"
            "uniform sampler2D u_texture_cr; /*unit2*/\n"
            "uniform float u_gamma;\n"
            "void main() {\n"
            "    float y = texture(u_texture_y, uv).r;\n"
            "    float cb = texture(u_texture_cb, uv).r;\n"
            "    float cr = texture(u_texture_cr, uv).r;\n"

            "    const mat4 to_rgb = mat4(\n"
            "       1.0000, 1.0000, 1.0000, 0.0000,\n"
            "       0.0000, -0.3441, 1.7720, 0.0000,\n"
            "       1.4020, -0.7141, 0.0000, 0.0000,\n"
            "       -0.7010, 0.5291, -0.8860, 1.0000\n"
            "    );\n"
            "    vec4 texel = to_rgb * vec4(y, cb, cr, 1.0);\n"
/* same as:
            "    vec3 yCbCr = vec3(y,cb-0.5,cr-0.5);\n"
            "    vec4 texel = vec4(  dot( vec3( 1.0,  0.0,      1.402 ),    yCbCr ),\n"
            "                        dot( vec3( 1.0 , -0.34414 , -0.71414 ), yCbCr ),\n"
            "                        dot( vec3( 1.0,  1.772,    0.0 ),      yCbCr ), 1.0);\n"
*/
    "    // gamma correction\n"
    "    texel.rgb = pow(texel.rgb, vec3(1.0 / u_gamma));\n"

        " // saturation (algorithm from Chapter 16 of OpenGL Shading Language)\n"
        " { float saturation = 2.0; const vec3 W = vec3(0.2125, 0.7154, 0.0721);\n"
        " vec3 intensity = vec3(dot(texel.rgb, W));\n"
        " texel.rgb = mix(intensity, texel.rgb, saturation); }\n"

            "    fragcolor = vec4(texel.rgb, 1.0);\n"
            "}\n";

        program = shader(vs, fs, "", "fragcolor" );
        u_gamma = glGetUniformLocation(program, "u_gamma");

        uy = glGetUniformLocation(program, "u_texture_y");
        ucb = glGetUniformLocation(program, "u_texture_cb");
        ucr = glGetUniformLocation(program, "u_texture_cr");

        glGenVertexArrays( 1, &vao );
    }

//    glEnable( GL_BLEND );
    glUseProgram( program );
    glUniform1f( u_gamma, gamma );

    glBindVertexArray( vao );

    glUniform1i(uy, 0);
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[0].id );

    glUniform1i(ucb, 1);
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[1].id );

    glUniform1i(ucr, 2);
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, textureYCbCr[2].id );

    glDrawArrays( GL_TRIANGLES, 0, 6 );
    profile_incstat("drawcalls", +1);
    profile_incstat("triangles", +2);

    glBindTexture( GL_TEXTURE_2D, 0 );
    glBindVertexArray( 0 );
    glUseProgram( 0 );
//    glDisable( GL_BLEND );
}

// ----------------------------------------------------------------------------
// sprites

typedef struct sprite_t {
    int cellw, cellh;         // dimensions of any cell in spritesheet
    int frame, ncx, ncy;      // frame in a (num cellx, num celly) spritesheet
    float px, py, pz;         // origin x, y, depth
    float ox, oy, cos, sin;   // offset x, offset y, cos/sin of rotation degree
    float sx, sy;             // scale x,y
    uint32_t rgba;            // vertex color
} sprite_t;

// sprite batching
typedef struct batch_t { array(sprite_t) sprites; mesh_t mesh; int dirty; } batch_t;
typedef map(int, batch_t) batch_group_t; // mapkey is anything that forces a flush. texture_id for now, might be texture_id+program_id soon

// sprite stream
typedef struct sprite_vertex { vec3 pos; vec2 uv; uint32_t rgba; } sprite_vertex;
typedef struct sprite_index  { GLuint triangle[3]; } sprite_index;

#define sprite_vertex(...) M_CAST(sprite_vertex, __VA_ARGS__)
#define sprite_index(...)  M_CAST(sprite_index, __VA_ARGS__)

// sprite impl
static int sprite_count = 0;
static int sprite_program = -1;
static array(sprite_index)  sprite_indices = 0;
static array(sprite_vertex) sprite_vertices = 0;
static batch_group_t sprite_additive_group = {0};
static batch_group_t sprite_translucent_group = {0};

void sprite( texture_t texture, float px, float py, float pz, float rot ) {
    sprite_ex( texture,
        px,py,pz, rot,                 // position (x,y,depth), rotation angle
        0,0, 1,1,                      // offset (x,y), scale (x,y),
        0,~0u,                         // is_additive, tint color
        0, 0,0                         // frame num(x) in a (y,z) spritesheet
    );
}

void sprite_ex( texture_t texture,
    float px, float py, float pz, float rotation,
    float ox, float oy, float sx, float sy,
    int additive, uint32_t rgba,
    float frame, float xcells, float ycells
) {
    if (frame < 0) return;
    if (frame > 0 && frame >= (xcells * ycells)) return;

    // no need to queue if alpha or scale are zero
    if( sx && sy && alpha(rgba) ) {
        sprite_t s;
        s.px = px;
        s.py = py;
        s.pz = pz;
        s.frame = frame;
        s.ncx = xcells ? xcells : 1;
        s.ncy = ycells ? ycells : 1;
        s.sx = sx;
        s.sy = sy;
        s.ox = ox * sx;
        s.oy = oy * sy;
        s.cellw = texture.x * sx / s.ncx;
        s.cellh = texture.y * sy / s.ncy;
        s.rgba = rgba;
        s.cos = 1;
        s.sin = 0;
        if(rotation) {
            rotation = (rotation + 0) * ((float)C_PI / 180);
            s.cos = cosf(rotation);
            s.sin = sinf(rotation);
        }

        batch_group_t *batches = additive == 1 ? &sprite_additive_group : &sprite_translucent_group;
#if 0
        batch_t *found = map_find(*batches, texture.id);
        if( !found ) found = map_insert(*batches, texture.id, (batch_t){0});
#else
        batch_t *found = map_find_or_add(*batches, texture.id, (batch_t){0});
#endif

        array_push(found->sprites, s);
    }
}

static void sprite_rebuild_meshes() {
    sprite_count = 0;

    batch_group_t* list[] = { &sprite_additive_group, &sprite_translucent_group };
    for( int l = 0; l < countof(list); ++l) {
        for each_map_ptr(*list[l], int,_, batch_t,bt) {

            bt->dirty = array_count(bt->sprites) ? 1 : 0;
            if( !bt->dirty ) continue;

            int index = 0;
            array_clear(sprite_indices);
            array_clear(sprite_vertices);

            array_foreach_ptr(bt->sprites, sprite_t,it ) {
                float x0 = it->ox - it->cellw/2, x3 = x0 + it->cellw;
                float y0 = it->oy - it->cellh/2, y3 = y0;
                float x1 = x0,                   x2 = x3;
                float y1 = y0 + it->cellh,       y2 = y1;

                // @todo: move this affine transform into glsl shader
                vec3 v0 = { it->px + ( x0 * it->cos - y0 * it->sin ), it->py + ( x0 * it->sin + y0 * it->cos ), it->pz };
                vec3 v1 = { it->px + ( x1 * it->cos - y1 * it->sin ), it->py + ( x1 * it->sin + y1 * it->cos ), it->pz };
                vec3 v2 = { it->px + ( x2 * it->cos - y2 * it->sin ), it->py + ( x2 * it->sin + y2 * it->cos ), it->pz };
                vec3 v3 = { it->px + ( x3 * it->cos - y3 * it->sin ), it->py + ( x3 * it->sin + y3 * it->cos ), it->pz };

                float cx = (1.0f / it->ncx) - 1e-9f;
                float cy = (1.0f / it->ncy) - 1e-9f;
                int idx = (int)it->frame;
                int px = idx % it->ncx;
                int py = idx / it->ncx;

                float ux = px * cx, uy = py * cy;
                float vx = ux + cx, vy = uy + cy;

                vec2 uv0 = vec2(ux, uy);
                vec2 uv1 = vec2(ux, vy);
                vec2 uv2 = vec2(vx, vy);
                vec2 uv3 = vec2(vx, uy);

                array_push( sprite_vertices, sprite_vertex(v0, uv0, it->rgba) ); // Vertex 0 (A)
                array_push( sprite_vertices, sprite_vertex(v1, uv1, it->rgba) ); // Vertex 1 (B)
                array_push( sprite_vertices, sprite_vertex(v2, uv2, it->rgba) ); // Vertex 2 (C)
                array_push( sprite_vertices, sprite_vertex(v3, uv3, it->rgba) ); // Vertex 3 (D)

                //      A--B                  A               A-B
                // quad |  | becomes triangle |\  and triangle \|
                //      D--C                  D-C               C
                GLuint A = (index+0), B = (index+1), C = (index+2), D = (index+3); index += 4;

                array_push( sprite_indices, sprite_index(C, D, A) ); // Triangle 1
                array_push( sprite_indices, sprite_index(C, A, B) ); // Triangle 2
            }

            mesh_upgrade(&bt->mesh, "p3 t2 c4b", 0,array_count(sprite_vertices),sprite_vertices, 3*array_count(sprite_indices),sprite_indices, MESH_STATIC);

            // clear elements from queue
            sprite_count += array_count(bt->sprites);
            array_clear(bt->sprites);
        }
    }
}

static void sprite_render_meshes() {
    if( sprite_program < 0 ) {
        sprite_program = shader(
            ""
                "in vec3 att_Position;\n"
                "in vec2 att_TexCoord;\n"
                "in vec4 att_Color;\n"
                "out vec2 vTexCoord;\n"
                "out vec4 vColor;\n"
                "\n"
                "uniform mat4 u_mvp;\n"
                "\n"
                "void main() {\n"
                "    vColor = att_Color;\n"
                "    vTexCoord = att_TexCoord;\n"
                "    gl_Position = u_mvp * vec4(att_Position, 1.0);\n"
                "}\n",
            ""
                "in vec2 vTexCoord;\n"
                "in vec4 vColor;\n"
                "out vec4 fragColor;\n"
                "\n"
                "uniform sampler2D u_texture;\n"
                "\n"
                "void main() {\n"
                "    vec4 texColor = texture(u_texture, vTexCoord);\n"
                "texColor = vColor * texColor;\n"
                "if(texColor.a < 0.5) discard;"
                "    fragColor = texColor;\n"
                "}\n",
            "att_Position,att_TexCoord,att_Color",
            "fragColor"
        );
    }

    // use the shader and  bind the texture @ unit 0
    shader_bind(sprite_program);
    glActiveTexture(GL_TEXTURE0);

    // setup rendering state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glDepthFunc(GL_LEQUAL); // try to help with zfighting

    // update camera and set mvp in the uniform
    mat44 mvp2d;
    float zdepth_max = window_height(); // 1;
    ortho44(mvp2d, 0, window_width(), window_height(), 0, -zdepth_max, +zdepth_max);
    shader_mat44("u_mvp", mvp2d);

    // set (unit 0) in the uniform texture sampler, and render batch
    // for all additive then translucent groups

    if( map_count(sprite_additive_group) > 0 ) {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE );
        for each_map_ptr(sprite_additive_group, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//        map_clear(sprite_additive_group);
    }

    if( map_count(sprite_translucent_group) > 0 ) {
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        for each_map_ptr(sprite_translucent_group, int,texture_id, batch_t,bt) {
            if( bt->dirty ) {
                shader_texture("u_texture", *texture_id, 0);
                mesh_render(&bt->mesh);
            }
        }
//        map_clear(sprite_translucent_group);
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthFunc(GL_LESS);
    glUseProgram(0);
}

static void sprite_init() {
    map_init(sprite_translucent_group, less_int, hash_int);
    map_init(sprite_additive_group, less_int, hash_int);
}

static void sprite_update() {
    profile(Sprite rebuild) {
    sprite_rebuild_meshes();
    }
    profile(Sprite render) {
    sprite_render_meshes();
    }
}

// -----------------------------------------------------------------------------
// cubemaps

// project cubemap coords into sphere normals
static
vec3 cubemap2polar(int face, int x, int y, int texture_width) {
    float u = (x / (texture_width - 1.f)) * 2 - 1;
    float v = (y / (texture_width - 1.f)) * 2 - 1;
    /**/ if( face == 0 ) return vec3( u, -1, -v);
    else if( face == 1 ) return vec3(-v, -u,  1);
    else if( face == 2 ) return vec3(-1, -u, -v);
    else if( face == 3 ) return vec3(-u,  1, -v);
    else if( face == 4 ) return vec3( v, -u, -1);
    else                 return vec3( 1,  u, -v);
}
// project normal in a sphere as 2d texcoord
static
vec2 polar2uv(vec3 n) {
    n = norm3(n);
    float theta = atan2(n.y, n.x);
    float phi = atan2(n.z, hypot(n.x, n.y));
    float u = (theta + C_PI) / C_PI;
    float v = (C_PI/2 - phi) / C_PI;
    return vec2(u, v);
}

// equirectangular panorama (2:1) to cubemap - in RGB, out RGB
static
void panorama2cubemap_(image_t out[6], const image_t in, int width){
    int face;
    #pragma omp parallel for
    for( face = 0; face < 6; ++face ) {
        out[face] = image_create(width, width, IMAGE_RGB);
        for (int j=0; j < width; ++j) {
            uint32_t *line = &out[ face ].pixels32[ 0 + j * width ];
            for (int i=0; i < width; ++i) {
                vec3 polar = cubemap2polar(face, i, j, width);
                vec2 uv = polar2uv(polar);
                uv = scale2(uv, in.h-1); // source coords (assumes 2:1, 2*h == w)
                vec3 rgb = bilinear(in, uv);
                union color {
                    struct { uint8_t r,g,b,a; };
                    uint32_t rgba;
                } c = { rgb.x, rgb.y, rgb.z, 255 };
                line[i] = c.rgba;
            }
        }
    }
}
// equirectangular panorama (2:1) to cubemap - in RGB, out RGBA
void panorama2cubemap(image_t out[6], const image_t in, int width) {
    int face;
    #pragma omp parallel for
    for( face = 0; face < 6; ++face ) {
        out[face] = image_create(width, width, IMAGE_RGBA);
        for (int j=0; j < width; ++j) {
            uint32_t *line = &out[ face ].pixels32[ 0 + j * width ];
            for (int i=0; i < width; ++i) {
                vec3 polar = cubemap2polar(face, i, j, width);
                vec2 uv = polar2uv(polar);
                uv = scale2(uv, in.h-1); // source coords (assumes 2:1, 2*h == w)
                vec3 rgb = bilinear(in, uv);
                union color {
                    struct { uint8_t r,g,b,a; };
                    uint32_t rgba;
                } c = { rgb.x, rgb.y, rgb.z, 255 };
                line[i] = c.rgba;
            }
        }
    }
}


cubemap_t cubemap6( const image_t images[6], int flags ) {
    cubemap_t c = {0}, z = {0};

    glGenTextures(1, &c.id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, c.id);

    int samples = 0;
    for (int i = 0; i < 6; i++) {
        image_t img = images[i]; //image(textures[i], IMAGE_RGB);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, img.w, img.h, 0, img.n == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, img.pixels);

        // calculate SH coefficients (@ands)
        const vec3 skyDir[] = {{ 1, 0, 0},{-1, 0, 0},{ 0, 1, 0},{ 0,-1, 0},{ 0, 0, 1},{ 0, 0,-1}};
        const vec3 skyX[]   = {{ 0, 0,-1},{ 0, 0, 1},{ 1, 0, 0},{ 1, 0, 0},{ 1, 0, 0},{-1, 0, 0}};
        const vec3 skyY[]   = {{ 0, 1, 0},{ 0, 1, 0},{ 0, 0,-1},{ 0, 0, 1},{ 0, 1, 0},{ 0, 1, 0}};
        int step = 16;
        for (int y = 0; y < img.h; y += step) {
            unsigned char *p = (unsigned char*)img.pixels + y * img.w * img.n;
            for (int x = 0; x < img.w; x += step) {
                vec3 n = add3(
                    add3(
                        scale3(skyX[i],  2.0f * (x / (img.w - 1.0f)) - 1.0f),
                        scale3(skyY[i], -2.0f * (y / (img.h - 1.0f)) + 1.0f)),
                    skyDir[i]); // texelDirection;
                float l = len3(n);
                vec3 light = div3(vec3(p[0], p[1], p[2]), 255.0f * l * l * l); // texelSolidAngle * texel_radiance;
                n = norm3(n);
                c.sh[0] = add3(c.sh[0], scale3(light,  0.282095f));
                c.sh[1] = add3(c.sh[1], scale3(light, -0.488603f * n.y * 2.0 / 3.0));
                c.sh[2] = add3(c.sh[2], scale3(light,  0.488603f * n.z * 2.0 / 3.0));
                c.sh[3] = add3(c.sh[3], scale3(light, -0.488603f * n.x * 2.0 / 3.0));
                c.sh[4] = add3(c.sh[4], scale3(light,  1.092548f * n.x * n.y / 4.0));
                c.sh[5] = add3(c.sh[5], scale3(light, -1.092548f * n.y * n.z / 4.0));
                c.sh[6] = add3(c.sh[6], scale3(light,  0.315392f * (3.0f * n.z * n.z - 1.0f) / 4.0));
                c.sh[7] = add3(c.sh[7], scale3(light, -1.092548f * n.x * n.z / 4.0));
                c.sh[8] = add3(c.sh[8], scale3(light,  0.546274f * (n.x * n.x - n.y * n.y) / 4.0));
                p += img.n * step;
                samples++;
            }
        }
    }

    for (int s = 0; s < 9; s++) {
        c.sh[s] = scale3(c.sh[s], 32.f / samples);
    }

    if( glGenerateMipmap )
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, glGenerateMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return c;
}

cubemap_t cubemap( const image_t in, int flags ) {
    ASSERT( in.n == 4 );
    image_t out[6];
    panorama2cubemap(out, in, in.h);
    image_t swap[6] = { out[0],out[3],out[1],out[4],out[2],out[5] };
    cubemap_t c = cubemap6(swap, flags);
    int i;
    #pragma omp parallel for
    for( i = 0; i < 6; ++i) image_destroy(&out[i]);
    return c;
}

void cubemap_destroy(cubemap_t *c) {
    glDeleteTextures(1, &c->id);
    c->id = 0; // do not destroy SH coefficients still. they might be useful in the future.
}

static cubemap_t *last_cubemap;

cubemap_t* cubemap_get_active() {
    return last_cubemap;
}

// -----------------------------------------------------------------------------

skybox_t skybox(const char *asset, int flags) {
    skybox_t sky = {0};

    // sky mesh
    vec3 vertices[] = {{+1,-1,+1},{+1,+1,+1},{+1,+1,-1},{-1,+1,-1},{+1,-1,-1},{-1,-1,-1},{-1,-1,+1},{-1,+1,+1}};
    unsigned indices[] = { 0, 1, 2, 3, 4, 5, 6, 3, 7, 1, 6, 0, 4, 2 };
    sky.geometry = mesh_create("p3", 0,countof(vertices),vertices, countof(indices),indices, MESH_TRIANGLE_STRIP);

    // sky program
    sky.program = shader(skybox_vs_3_3, skybox_fs_3_4, "att_position", "fragcolor");

    // sky cubemap & SH
    if( asset ) {
        int is_panorama = vfs_size( asset );
        if( is_panorama ) {
            stbi_hdr_to_ldr_gamma(1.2f);
            image_t panorama = image( asset, IMAGE_RGBA );
            sky.cubemap = cubemap( panorama, 0 ); // RGBA required
            image_destroy(&panorama);
        } else {
            image_t images[6] = {0};
            images[0] = image( stringf("%s/posx", asset), IMAGE_RGB ); // cubepx
            images[1] = image( stringf("%s/negx", asset), IMAGE_RGB ); // cubenx
            images[2] = image( stringf("%s/posy", asset), IMAGE_RGB ); // cubepy
            images[3] = image( stringf("%s/negy", asset), IMAGE_RGB ); // cubeny
            images[4] = image( stringf("%s/posz", asset), IMAGE_RGB ); // cubepz
            images[5] = image( stringf("%s/negz", asset), IMAGE_RGB ); // cubenz
            sky.cubemap = cubemap6( images, 0 );
            for( int i = 0; i < countof(images); ++i ) image_destroy(&images[i]);
        }
    }

    return sky;
}
int skybox_push_state(skybox_t *sky, float mvp[16]) {
    last_cubemap = &sky->cubemap;

//glClear(GL_DEPTH_BUFFER_BIT);
//glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LEQUAL);
//glDisable(GL_CULL_FACE);
glDisable(GL_DEPTH_TEST);

    //glDepthMask(GL_FALSE);
    shader_bind(sky->program);
    shader_mat44("u_mvp", mvp);
    shader_cubemap("u_cubemap", sky->cubemap.id);
    return 0; // @fixme: return sortable hash here?
}
int skybox_pop_state(skybox_t *sky) {
    //glDepthMask(GL_TRUE);
    //glClear(GL_DEPTH_BUFFER_BIT);
    return 0;
}
void skybox_destroy(skybox_t *sky) {
    glDeleteProgram(sky->program);
    cubemap_destroy(&sky->cubemap);
    mesh_destroy(&sky->geometry);
}

// -----------------------------------------------------------------------------

mesh_t mesh_create(const char *format, int vertex_stride,int vertex_count,const void *vertex_data, int index_count,const void *index_data, int flags) {
    mesh_t z = {0};
    mesh_upgrade(&z, format,  vertex_stride,vertex_count,vertex_data,  index_count,index_data,  flags);
    return z;
}

void mesh_upgrade(mesh_t *m, const char *format, int vertex_stride,int vertex_count,const void *vertex_data, int index_count,const void *index_data, int flags) {
    m->flags = flags;

    // setup
    unsigned sizeof_index = sizeof(GLuint);
    unsigned sizeof_vertex = 0;
    m->index_count = index_count;
    m->vertex_count = vertex_count;

    // iterate vertex attributes { position, normal + uv + tangent + bitangent + ... }
    struct vertex_descriptor {
        int vertex_type, num_attribute, num_components, alt_normalized;
        int stride, offset;
    } descriptor[16] = {0}, *dc = &descriptor[0];

    do switch( *format ) {
        break; case '*': dc->alt_normalized = 1;
        break; case '0': dc->num_components = 0;
        break; case '1': dc->num_components = 1;
        break; case '2': dc->num_components = 2;
        break; case '3': dc->num_components = 3;
        break; case '4': dc->num_components = 4;
        break; case 'f': dc->vertex_type = GL_FLOAT;
        break; case 'u': case 'i': dc->vertex_type = GL_UNSIGNED_INT;
        break; case 'b': if(format[-1] >= '0' && format[-1] <= '9') dc->vertex_type = GL_UNSIGNED_BYTE; //else bitangent.
        break; case ' ': while (format[1] == ' ') format++; case '\0':
            if (!dc->vertex_type) dc->vertex_type = GL_FLOAT;
            dc->offset = sizeof_vertex;
            sizeof_vertex += (dc->stride = dc->num_components * (dc->vertex_type == GL_UNSIGNED_BYTE ? 1 : 4));
            ++dc;
        break; default: if( !strchr("pntcwai", *format) ) PANIC("unsupported vertex type '%c'", *format);
    } while (*format++);

    if(vertex_stride > 0) sizeof_vertex = vertex_stride;

    // layout
    if(!m->vao) glGenVertexArrays(1, &m->vao);
    glBindVertexArray(m->vao);

    // index data
    if( index_data && index_count ) {
        m->index_count = index_count;

        if(!m->ibo) glGenBuffers(1, &m->ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m->index_count * sizeof_index, index_data, flags & MESH_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
    }

    // vertex data
    if( vertex_data && vertex_count ) {
        m->vertex_count = vertex_count;

        if(!m->vbo) glGenBuffers(1, &m->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
        glBufferData(GL_ARRAY_BUFFER, m->vertex_count * sizeof_vertex, vertex_data, flags & MESH_STREAM ? GL_STREAM_DRAW : GL_STATIC_DRAW);
    }

    for( int i = 0; i < 8; ++i ) {
//        glDisableVertexAttribArray(i);
    }

    // vertex setup: iterate descriptors
    for( int i = 0; i < countof(descriptor); ++i ) {
        if( descriptor[i].num_components ) {
            glDisableVertexAttribArray(i);
            glVertexAttribPointer(i,
                descriptor[i].num_components, descriptor[i].vertex_type, (descriptor[i].vertex_type == GL_UNSIGNED_BYTE ? GL_TRUE : GL_FALSE) ^ (descriptor[i].alt_normalized ? GL_TRUE : GL_FALSE),
                sizeof_vertex, (GLchar*)NULL + descriptor[i].offset);
            glEnableVertexAttribArray(i);
        } else {
            glDisableVertexAttribArray(i);
        }
    }

    glBindVertexArray(0);
}

void mesh_pop_state(mesh_t *sm) {

}

void mesh_push_state(mesh_t *sm, unsigned program, unsigned texture_id, float model[16], float view[16], float proj[16], unsigned billboard) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glActiveTexture(GL_TEXTURE0);

    shader_bind(program);

    mat44 mv; multiply44x2(mv, view, model);
    if( billboard ) {
        float d = sqrt(mv[4*0+0] * mv[4*0+0] + mv[4*1+1] * mv[4*1+1] + mv[4*2+2] * mv[4*2+2]);
        if(billboard & 4) mv[4*0+0] = d, mv[4*0+1] = 0, mv[4*0+2] = 0;
        if(billboard & 2) mv[4*1+0] = 0, mv[4*1+1] = d, mv[4*1+2] = 0;
        if(billboard & 1) mv[4*2+0] = 0, mv[4*2+1] = 0, mv[4*2+2] = d;
    }

    mat44 mvp; multiply44x2(mvp, proj, mv); // multiply44x3(mvp, proj, view, model);
    shader_mat44("u_mvp", mvp);

    if (cubemap_get_active()) {
    GLuint uniform_loc = glGetUniformLocation(program, "u_coefficients_sh");
    glUniform3fv(uniform_loc, 9, &cubemap_get_active()->sh[0].x);
    }

    shader_texture("u_texture2d", texture_id, 0);
}

void mesh_render(mesh_t *sm) {
    glBindVertexArray(sm->vao);
    if( sm->ibo ) { // with indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm->ibo); // <-- why intel?
        glDrawElements(sm->flags & MESH_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, sm->index_count, GL_UNSIGNED_INT, (char*)0);
        profile_incstat("drawcalls", +1);
        profile_incstat("triangles", sm->index_count/3);
    } else { // with vertices only
        glDrawArrays(sm->flags & MESH_TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, sm->vertex_count /* / 3 */);
        profile_incstat("drawcalls", +1);
        profile_incstat("triangles", sm->vertex_count/3);
    }
}

void mesh_destroy(mesh_t *m) {
    // @todo
}

// -----------------------------------------------------------------------------
// screenshot

void* screenshot( unsigned n ) {
    int w = window_width(), h = window_height();
    int mode = n == 3 ? GL_RGB : n == -3 ? GL_BGR : n == 4 ? GL_RGBA : GL_BGRA;
    static uint8_t *pixels = 0;
    pixels = (uint8_t*)REALLOC(pixels, w * h * 4 );
#if 0
    // sync, 10 ms
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0); // disable any pbo, in case somebody did for us
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, pixels);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    return pixels;
#else
    // async
    enum { NUM_PBOS = 16 };
    static GLuint pbo[NUM_PBOS] = {0}, lastw, lasth;
    static int frame = 0, bound = 0;

    if( lastw != w || lasth != h ) {
        lastw = w, lasth = h;
        frame = 0;
        bound = 0;

        // @fixme: delete previous pbos
        for( int i = 0; i < NUM_PBOS; ++i ) {
        glGenBuffers(1, &pbo[i]);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
        glBufferData(GL_PIXEL_PACK_BUFFER, w * h * 4, NULL, GL_STREAM_READ); // GL_STATIC_READ);
        }
    }

    if (frame < NUM_PBOS) {
        // do setup during initial frames
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[bound]);
        glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, (GLvoid*)((GLchar*)NULL+0));
    } else {
        // read from oldest bound pbo
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[bound]);
        void *ptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        memcpy(pixels, ptr, w * h * abs(n));
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
        // trigger next read
        glReadPixels(0, 0, w, h, mode, GL_UNSIGNED_BYTE, (GLvoid*)((GLchar*)NULL+0));
    }

    bound = (bound + 1) % NUM_PBOS;
    frame += frame >= 0 && frame < NUM_PBOS;
    frame *= frame == NUM_PBOS ? -1 : +1;

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    return pixels;
#endif
}

// -----------------------------------------------------------------------------
// viewport

void viewport_color(vec3 color3) {
    glClearColor(color3.x, color3.y, color3.z, 1);
}

void viewport_clear(bool color, bool depth) {
    glClearDepthf(1);
    glClearStencil(0);
    glClear((color ? GL_COLOR_BUFFER_BIT : 0) | (depth ? GL_DEPTH_BUFFER_BIT : 0));
}

void viewport_clip(vec2 from, vec2 to) {
    float x = from.x, y = from.y, w = to.x-from.x, h = to.y-from.y;

    y = window_height()-y-h;
    glViewport(x, y, w, h);
    glScissor(x, y, w, h);
}

// -----------------------------------------------------------------------------

unsigned fbo(unsigned color_texture_id, unsigned depth_texture_id, int flags) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if( color_texture_id ) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_id, 0);
    if( depth_texture_id ) glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_id, 0);
#if 0 // this is working; it's just not enabled for now
    else {
        // create a non-sampleable renderbuffer object for depth and stencil attachments
        unsigned int rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, color.width, color.height); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    }
#endif

    if(flags) glDrawBuffer(GL_NONE);
    if(flags) glReadBuffer(GL_NONE);

#if 1
    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( GL_FRAMEBUFFER_COMPLETE != result ) {
        PANIC("ERROR: Framebuffer not complete.");
    }
#else
    switch (glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        case GL_FRAMEBUFFER_COMPLETE: break;
        case GL_FRAMEBUFFER_UNDEFINED: PANIC("GL_FRAMEBUFFER_UNDEFINED");
        case GL_FRAMEBUFFER_UNSUPPORTED: PANIC("GL_FRAMEBUFFER_UNSUPPORTED");
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: PANIC("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: PANIC("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: PANIC("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
//      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: PANIC("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
//      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: PANIC("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
        default: PANIC("ERROR: Framebuffer not complete. glCheckFramebufferStatus returned %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }
#endif

    glBindFramebuffer (GL_FRAMEBUFFER, 0);
    return fbo;
}
void fbo_bind(unsigned id) {
    glBindFramebuffer(GL_FRAMEBUFFER, id);
}
void fbo_unbind() {
    fbo_bind(0);
}
void fbo_destroy(unsigned id) {
    // glDeleteRenderbuffers(1, &renderbuffer);
    glDeleteFramebuffers(1, &id);
}

// -----------------------------------------------------------------------------

// post-effects swap chain.
// - rlyeh, public domain.

typedef struct passfx passfx;
typedef struct postfx postfx;

void postfx_create(postfx *fx, int flags);
void postfx_destroy(postfx *fx);

bool postfx_load(postfx *fx, const char *name, const char *fragment);
bool postfx_begin(postfx *fx, int width, int height);
bool postfx_end(postfx *fx);

bool postfx_enabled(postfx *fx, int pass_number);
bool postfx_enable(postfx *fx, int pass_number, bool enabled);
// bool postfx_toggle(postfx *fx, int pass_number);
void postfx_clear(postfx *fx);

char* postfx_name(postfx *fx, int slot);

struct passfx {
    mesh_t m;
    char *name;
    unsigned program;
    int uniforms[16];
};

struct postfx {
    // renderbuffers: color & depth textures
    unsigned fb[2];
    texture_t diffuse[2], depth[2];
    // shader passes
    passfx pass[64];
    uint64_t mask;
    // global enable flag
    bool enabled;
    //
    int num_loaded;
};

enum {
    u_color,
    u_depth,
    u_time,
    u_frame,
    u_width, u_height,
    u_mousex, u_mousey,
    u_channelres0x, u_channelres0y,
    u_channelres1x, u_channelres1y,
};

void postfx_create(postfx *fx, int flags) {
    postfx z = {0};
    *fx = z;
    fx->enabled = 1;
}

void postfx_destroy( postfx *fx ) {
    for( int i = 0; i < 64; ++i ) {
        FREE(fx->pass[i].name);
    }
    texture_destroy(&fx->diffuse[0]);
    texture_destroy(&fx->diffuse[1]);
    texture_destroy(&fx->depth[0]);
    texture_destroy(&fx->depth[1]);
    fbo_destroy(fx->fb[0]);
    fbo_destroy(fx->fb[1]);
    postfx z = {0};
    *fx = z;
}

char* postfx_name(postfx *fx, int slot) {
    return fx->pass[ slot & 63 ].name;
}

bool postfx_load_from_mem( postfx *fx, const char *name, const char *fs ) {
    if(!fs || !fs[0]) PANIC("!invalid fragment shader");

    int slot = fx->num_loaded++;

    passfx *p = &fx->pass[ slot & 63 ];
    p->name = STRDUP(name);

    const char *vs = fullscreen_quad_vertex_shader(0);

    // patch fragment
    char *fs2 = (char*)CALLOC(1, 64*1024);
    strcat(fs2,
        ""
        "#define texture2D texture\n"
        "#define texture2DLod textureLod\n"
        "#define FRAGCOLOR fragColor\n"
        "#define texcoord uv\n"
        "#define TEXCOORD uv\n"
        "uniform sampler2D iChannel0;\n"
        "uniform sampler2D iChannel1;\n"
        "uniform float iWidth, iHeight, iTime, iFrame, iMousex, iMousey;\n"
        "uniform float iChannelRes0x, iChannelRes0y;\n"
        "uniform float iChannelRes1x, iChannelRes1y;\n"
        "vec2 iResolution = vec2(iWidth, iHeight);\n"
        "vec2 iMouse = vec2(iMousex, iMousey);\n"
        "vec2 iChannelResolution[2] = vec2[2]( vec2(iChannelRes0x, iChannelRes0y),vec2(iChannelRes1x, iChannelRes1y) );\n"
        "float iGlobalTime = iTime;\n"
        "in vec2 texcoord;\n"
        "out vec4 fragColor;\n"
    );

    if( strstr(fs, "mainImage") ) {
        strcat(fs2,
            "void mainImage( out vec4 fragColor, in vec2 fragCoord );\n"
            "void main() {\n"
            "   mainImage(fragColor, texcoord.xy * iResolution);\n"
            "}\n");
    }

    strcat(fs2, fs);

    p->program = shader(vs, fs2, "vtexcoord", "fragColor" );

    FREE(fs2);

    glUseProgram(p->program); // needed?

    for( int i = 0; i < countof(p->uniforms); ++i ) p->uniforms[i] = -1;

    if( p->uniforms[u_time] == -1 )   p->uniforms[u_time] = glGetUniformLocation(p->program, "iTime");

    if( p->uniforms[u_frame] == -1 )   p->uniforms[u_frame] = glGetUniformLocation(p->program, "iFrame");

    if( p->uniforms[u_width] == -1 )  p->uniforms[u_width] = glGetUniformLocation(p->program, "iWidth");
    if( p->uniforms[u_height] == -1 ) p->uniforms[u_height] = glGetUniformLocation(p->program, "iHeight");

    if( p->uniforms[u_mousex] == -1 ) p->uniforms[u_mousex] = glGetUniformLocation(p->program, "iMousex");
    if( p->uniforms[u_mousey] == -1 ) p->uniforms[u_mousey] = glGetUniformLocation(p->program, "iMousey");

    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tex");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tex0");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tColor");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "tDiffuse");
    if( p->uniforms[u_color] == -1 ) p->uniforms[u_color] = glGetUniformLocation(p->program, "iChannel0");

    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "tex1");
    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "tDepth");
    if( p->uniforms[u_depth] == -1 ) p->uniforms[u_depth] = glGetUniformLocation(p->program, "iChannel1");

    if( p->uniforms[u_channelres0x] == -1 ) p->uniforms[u_channelres0x] = glGetUniformLocation(p->program, "iChannelRes0x");
    if( p->uniforms[u_channelres0y] == -1 ) p->uniforms[u_channelres0y] = glGetUniformLocation(p->program, "iChannelRes0y");

    if( p->uniforms[u_channelres1x] == -1 ) p->uniforms[u_channelres1x] = glGetUniformLocation(p->program, "iChannelRes1x");
    if( p->uniforms[u_channelres1y] == -1 ) p->uniforms[u_channelres1y] = glGetUniformLocation(p->program, "iChannelRes1y");

    // set quad
    glGenVertexArrays(1, &p->m.vao);
    return true;
}

uint64_t postfx_count_ones(uint64_t x) {
    // [src] https://en.wikipedia.org/wiki/Hamming_weight
    x -= (x >> 1) & 0x5555555555555555ULL;                                //put count of each 2 bits into those 2 bits
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL); //put count of each 4 bits into those 4 bits
    x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;                           //put count of each 8 bits into those 8 bits
    return (x * 0x0101010101010101ULL) >> 56;                             //returns left 8 bits of x + (x<<8) + (x<<16) + (x<<24) + ...
}

bool postfx_enable(postfx *fx, int pass, bool enabled) {
    fx->mask = enabled ? fx->mask | (1ull << pass) : fx->mask & ~(1ull << pass);
    fx->enabled = !!postfx_count_ones(fx->mask);
    return fx->enabled;
}

bool postfx_enabled(postfx *fx, int pass) {
    return (!!(fx->mask & (1ull << pass)));
}

bool postfx_toggle(postfx *fx, int pass) {
    return postfx_enable(fx, pass, 1 ^ postfx_enabled(fx, pass));
}

void postfx_clear(postfx *fx) {
    fx->mask = fx->enabled = 0;
}

bool postfx_begin(postfx *fx, int width, int height) {
    width += !width;
    height += !height;

    // resize if needed
    if( fx->diffuse[0].w != width || fx->diffuse[0].h != height ) {
        texture_destroy(&fx->diffuse[0]);
        texture_destroy(&fx->diffuse[1]);
        texture_destroy(&fx->depth[0]);
        texture_destroy(&fx->depth[1]);
        fbo_destroy(fx->fb[0]);
        fbo_destroy(fx->fb[1]);

        // create texture, set texture parameters and content
        fx->diffuse[0] = texture_create(width, height, 4, NULL, TEXTURE_RGBA);
        fx->depth[0] = texture_create(width, height, 1,  NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
        fx->fb[0] = fbo(fx->diffuse[0].id, fx->depth[0].id, 0);

        // create texture, set texture parameters and content
        fx->diffuse[1] = texture_create(width, height, 4, NULL, TEXTURE_RGBA);
        fx->depth[1] = texture_create(width, height, 1, NULL, TEXTURE_DEPTH|TEXTURE_FLOAT);
        fx->fb[1] = fbo(fx->diffuse[1].id, fx->depth[1].id, 0);
    }

    uint64_t num_active_passes = postfx_count_ones(fx->mask);
    bool active = fx->enabled && num_active_passes;
    if( !active ) {
        fbo_unbind();
        return false;
    }

    fbo_bind(fx->fb[1]);

    viewport_clear(true, true);
    viewport_clip(vec2(0,0), vec2(width, height));

    fbo_bind(fx->fb[0]);

    viewport_clear(true, true);
    viewport_clip(vec2(0,0), vec2(width, height));

    return true;
}

bool postfx_end(postfx *fx) {
    uint64_t num_active_passes = postfx_count_ones(fx->mask);
    bool active = fx->enabled && num_active_passes;
    if( !active ) {
        return false;
    }

    fbo_unbind();

    // disable depth test in 2d rendering
    glDisable(GL_DEPTH_TEST);

    int frame = 0;
    float t = time_ms() / 1000.f;
    float w = fx->diffuse[0].w;
    float h = fx->diffuse[0].h;
    float mx = input(MOUSE_X);
    float my = input(MOUSE_Y);

    for(int i = 0, e = countof(fx->pass); i < e; ++i) {
        if( fx->mask & (1ull << i) ) {
            passfx *pass = &fx->pass[i];

            if( !pass->program ) { --num_active_passes; continue; }
            glUseProgram(pass->program);

            // bind texture to texture unit 0
            // shader_texture(fx->diffuse[frame], 0);
 glActiveTexture(GL_TEXTURE0 + 0);            glBindTexture(GL_TEXTURE_2D, fx->diffuse[frame].id);
            glUniform1i(pass->uniforms[u_color], 0);

            glUniform1f(pass->uniforms[u_channelres0x], fx->diffuse[frame].w);
            glUniform1f(pass->uniforms[u_channelres0y], fx->diffuse[frame].h);

            // bind depth to texture unit 1
            // shader_texture(fx->depth[frame], 1);
 glActiveTexture(GL_TEXTURE0 + 1);            glBindTexture(GL_TEXTURE_2D, fx->depth[frame].id);
            glUniform1i(pass->uniforms[u_depth], 1);

            // bind uniforms
            static unsigned f = 0; ++f;
            glUniform1f(pass->uniforms[u_time], t);
            glUniform1f(pass->uniforms[u_frame], f-1);
            glUniform1f(pass->uniforms[u_width], w);
            glUniform1f(pass->uniforms[u_height], h);

            glUniform1f(pass->uniforms[u_mousex], mx);
            glUniform1f(pass->uniforms[u_mousey], my);

            // bind the vao
            int bound = --num_active_passes;
            if( bound ) fbo_bind(fx->fb[frame ^= 1]);

                // fullscreen quad
                glBindVertexArray(pass->m.vao);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                profile_incstat("drawcalls", +1);
                profile_incstat("triangles", +2);
                glBindVertexArray(0);

            if( bound ) fbo_unbind();
            else glUseProgram(0);
        }
    }

    return true;
}


static postfx fx;
void fx_load_from_mem(const char *nameid, const char *content) {
    ONCE postfx_create(&fx, 0);
    postfx_load_from_mem(&fx, nameid, content);
}
void fx_load(const char *file) {
    postfx_load_from_mem(&fx, file_name(file), vfs_read(file));
}
void fx_begin() {
    postfx_begin(&fx, window_width(), window_height());
}
void fx_end() {
    postfx_end(&fx);
}
int fx_enabled(int pass) {
    return postfx_enabled(&fx, pass);
}
void fx_enable(int pass, int enabled) {
    postfx_enable(&fx, pass, enabled);
}
void fx_enable_all(int enabled) {
    for( int i = 0; i < fx.num_loaded; ++i ) fx_enable(i, enabled);
}
char *fx_name(int pass) {
    return postfx_name(&fx, pass);
}

// -----------------------------------------------------------------------------

#define IQM_MAGIC "INTERQUAKEMODEL"
#define IQM_VERSION 2

struct iqmheader {
    char magic[16];
    unsigned version;
    unsigned filesize;
    unsigned flags;
    unsigned num_text, ofs_text;
    unsigned num_meshes, ofs_meshes;
    unsigned num_vertexarrays, num_vertexes, ofs_vertexarrays;
    unsigned num_triangles, ofs_triangles, ofs_adjacency;
    unsigned num_joints, ofs_joints;
    unsigned num_poses, ofs_poses;
    unsigned num_anims, ofs_anims;
    unsigned num_frames, num_framechannels, ofs_frames, ofs_bounds;
    unsigned num_comment, ofs_comment;
    unsigned num_extensions, ofs_extensions;
};

struct iqmmesh {
    unsigned name;
    unsigned material;
    unsigned first_vertex, num_vertexes;
    unsigned first_triangle, num_triangles;
};

enum {
    IQM_POSITION,
    IQM_TEXCOORD,
    IQM_NORMAL,
    IQM_TANGENT,
    IQM_BLENDINDEXES,
    IQM_BLENDWEIGHTS,
    IQM_COLOR,
    IQM_CUSTOM = 0x10
};

enum {
    IQM_BYTE,
    IQM_UBYTE,
    IQM_SHORT,
    IQM_USHORT,
    IQM_INT,
    IQM_UINT,
    IQM_HALF,
    IQM_FLOAT,
    IQM_DOUBLE,
};

struct iqmtriangle {
    unsigned vertex[3];
};

struct iqmadjacency {
    unsigned triangle[3];
};

struct iqmjoint {
    unsigned name;
    int parent;
    float translate[3], rotate[4], scale[3];
};

struct iqmpose {
    int parent;
    unsigned mask;
    float channeloffset[10];
    float channelscale[10];
};

struct iqmanim {
    unsigned name;
    unsigned first_frame, num_frames;
    float framerate;
    unsigned flags;
};

enum {
    IQM_LOOP = 1<<0
};

struct iqmvertexarray {
    unsigned type;
    unsigned flags;
    unsigned format;
    unsigned size;
    unsigned offset;
};

struct iqmbounds {
    union {
        struct { float bbmin[3], bbmax[3]; };
        struct { vec3 min3, max3; };
        aabb box;
    };
    float xyradius, radius;
};

// -----------------------------------------------------------------------------

typedef struct iqm_vertex {
    GLfloat position[3];
    GLfloat texcoord[2];
    GLfloat normal[3];
    GLfloat tangent[4];
    GLubyte blendindexes[4];
    GLubyte blendweights[4];
    GLubyte color[4];
} iqm_vertex;

typedef struct iqm_t {
    int nummeshes, numtris, numverts, numjoints, numframes, numanims;
    GLuint program;
    GLuint vao, ibo, vbo;
    GLuint *textures;
    uint8_t *buf, *meshdata, *animdata;
    struct iqmmesh *meshes;
    struct iqmjoint *joints;
    struct iqmpose *poses;
    struct iqmanim *anims;
    struct iqmbounds *bounds;
    mat34 *baseframe, *inversebaseframe, *outframe, *frames;
    GLint bonematsoffset;
} iqm_t;

#define program (q->program)
#define meshdata (q->meshdata)
#define animdata (q->animdata)
#define nummeshes (q->nummeshes)
#define numtris (q->numtris)
#define numverts (q->numverts)
#define numjoints (q->numjoints)
#define numframes (q->numframes)
#define numanims (q->numanims)
#define meshes (q->meshes)
#define textures (q->textures)
#define joints (q->joints)
#define poses (q->poses)
#define anims (q->anims)
#define baseframe (q->baseframe)
#define inversebaseframe (q->inversebaseframe)
#define outframe (q->outframe)
#define frames (q->frames)
#define vao (q->vao)
#define ibo (q->ibo)
#define vbo (q->vbo)
#define bonematsoffset (q->bonematsoffset)
#define buf (q->buf)
#define bounds (q->bounds)

static
bool model_load_meshes(iqm_t *q, const struct iqmheader *hdr) {
    if(meshdata) return false;

    lil32p(&buf[hdr->ofs_vertexarrays], hdr->num_vertexarrays*sizeof(struct iqmvertexarray)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_triangles], hdr->num_triangles*sizeof(struct iqmtriangle)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_meshes], hdr->num_meshes*sizeof(struct iqmmesh)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_joints], hdr->num_joints*sizeof(struct iqmjoint)/sizeof(uint32_t));

    meshdata = buf;
    nummeshes = hdr->num_meshes;
    numtris = hdr->num_triangles;
    numverts = hdr->num_vertexes;
    numjoints = hdr->num_joints;
    outframe = CALLOC(hdr->num_joints, sizeof(mat34));

    float *inposition = NULL, *innormal = NULL, *intangent = NULL, *intexcoord = NULL;
    uint8_t *inblendindex8 = NULL, *inblendweight8 = NULL;
    int *inblendindexi = NULL; float *inblendweightf = NULL;
    struct iqmvertexarray *vas = (struct iqmvertexarray *)&buf[hdr->ofs_vertexarrays];
    for(int i = 0; i < (int)hdr->num_vertexarrays; i++) {
        struct iqmvertexarray *va = &vas[i];
        switch(va->type) {
        default: continue; // return PANIC("unknown iqm vertex type (%d)", va->type), false;
        break; case IQM_POSITION: if(va->format != IQM_FLOAT || va->size != 3) return PANIC("!"); false; inposition = (float *)&buf[va->offset]; lil32pf(inposition, 3*hdr->num_vertexes);
        break; case IQM_NORMAL: if(va->format != IQM_FLOAT || va->size != 3) return PANIC("!"); false; innormal = (float *)&buf[va->offset]; lil32pf(innormal, 3*hdr->num_vertexes);
        break; case IQM_TANGENT: if(va->format != IQM_FLOAT || va->size != 4) return PANIC("!"); false; intangent = (float *)&buf[va->offset]; lil32pf(intangent, 4*hdr->num_vertexes);
        break; case IQM_TEXCOORD: if(va->format != IQM_FLOAT || va->size != 2) return PANIC("!"); false; intexcoord = (float *)&buf[va->offset]; lil32pf(intexcoord, 2*hdr->num_vertexes);
        break; case IQM_BLENDINDEXES: if(va->size != 4) return PANIC("!"); false; if(va->format != IQM_UBYTE && va->format != IQM_INT) return PANIC("!"); false;
        if(va->format == IQM_UBYTE) inblendindex8 = (uint8_t *)&buf[va->offset];
        else inblendindexi = (int *)&buf[va->offset];
        break; case IQM_BLENDWEIGHTS: if(va->size != 4) return PANIC("!"); false; if(va->format != IQM_UBYTE && va->format != IQM_FLOAT) return PANIC("!"); false;
        if(va->format == IQM_UBYTE) inblendweight8 = (uint8_t *)&buf[va->offset];
        else inblendweightf = (float *)&buf[va->offset];
        }
    }

    if (hdr->ofs_bounds) lil32p(buf + hdr->ofs_bounds, hdr->num_frames * sizeof(struct iqmbounds));
    if (hdr->ofs_bounds) bounds = (struct iqmbounds *) &buf[hdr->ofs_bounds];

    meshes = (struct iqmmesh *)&buf[hdr->ofs_meshes];
    joints = (struct iqmjoint *)&buf[hdr->ofs_joints];

    baseframe = CALLOC(hdr->num_joints, sizeof(mat34));
    inversebaseframe = CALLOC(hdr->num_joints, sizeof(mat34));
    for(int i = 0; i < (int)hdr->num_joints; i++) {
        struct iqmjoint *j = &joints[i];
        compose34(baseframe[i], ptr3(j->translate), normq(ptrq(j->rotate)), ptr3(j->scale));
        invert34(inversebaseframe[i], baseframe[i]);
        if(j->parent >= 0) {
            multiply34x2(baseframe[i], baseframe[j->parent], baseframe[i]);
            multiply34(inversebaseframe[i], inversebaseframe[j->parent]);
        }
    }

    struct iqmtriangle *tris = (struct iqmtriangle *)&buf[hdr->ofs_triangles];

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if(!ibo) glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, hdr->num_triangles*sizeof(struct iqmtriangle), tris, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    iqm_vertex *verts = CALLOC(hdr->num_vertexes, sizeof(iqm_vertex));
    for(int i = 0; i < (int)hdr->num_vertexes; i++) {
        iqm_vertex *v = &verts[i];
        if(inposition) memcpy(v->position, &inposition[i*3], sizeof(v->position));
        if(innormal) memcpy(v->normal, &innormal[i*3], sizeof(v->normal));
        if(intangent) memcpy(v->tangent, &intangent[i*4], sizeof(v->tangent));
        if(intexcoord) memcpy(v->texcoord, &intexcoord[i*2], sizeof(v->texcoord));
        if(inblendindex8) memcpy(v->blendindexes, &inblendindex8[i*4], sizeof(v->blendindexes));
        if(inblendweight8) memcpy(v->blendweights, &inblendweight8[i*4], sizeof(v->blendweights));
        if(inblendindexi) {
            uint8_t conv[4] = { inblendindexi[i*4], inblendindexi[i*4+1], inblendindexi[i*4+2], inblendindexi[i*4+3] };
            memcpy(v->blendindexes, conv, sizeof(v->blendindexes));
        }
        if(inblendweightf) {
            uint8_t conv[4] = { inblendweightf[i*4] * 255, inblendweightf[i*4+1] * 255, inblendweightf[i*4+2] * 255, inblendweightf[i*4+3] * 255 };
            memcpy(v->blendweights, conv, sizeof(v->blendweights));
        }
    }

    if(!vbo) glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, hdr->num_vertexes*sizeof(iqm_vertex), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    FREE(verts);

    return true;
}

static
bool model_load_anims(iqm_t *q, const struct iqmheader *hdr) {
    if((int)hdr->num_poses != numjoints) return false;

    if(animdata) {
        if(animdata != meshdata) FREE(animdata);
        FREE(frames);
        animdata = NULL;
        anims = NULL;
        frames = 0;
        numframes = 0;
        numanims = 0;
    }

    lil32p(&buf[hdr->ofs_poses], hdr->num_poses*sizeof(struct iqmpose)/sizeof(uint32_t));
    lil32p(&buf[hdr->ofs_anims], hdr->num_anims*sizeof(struct iqmanim)/sizeof(uint32_t));
    lil16p((uint16_t *)&buf[hdr->ofs_frames], hdr->num_frames*hdr->num_framechannels);

    animdata = buf;
    numanims = hdr->num_anims;
    numframes = hdr->num_frames;

    const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
    anims = (struct iqmanim *)&buf[hdr->ofs_anims];
    poses = (struct iqmpose *)&buf[hdr->ofs_poses];
    frames = CALLOC(hdr->num_frames * hdr->num_poses, sizeof(mat34));
    uint16_t *framedata = (uint16_t *)&buf[hdr->ofs_frames];

    for(int i = 0; i < (int)hdr->num_frames; i++) {
        for(int j = 0; j < (int)hdr->num_poses; j++) {
            struct iqmpose *p = &poses[j];
            quat rotate;
            vec3 translate, scale;
            translate.x = p->channeloffset[0]; if(p->mask&0x01) translate.x += *framedata++ * p->channelscale[0];
            translate.y = p->channeloffset[1]; if(p->mask&0x02) translate.y += *framedata++ * p->channelscale[1];
            translate.z = p->channeloffset[2]; if(p->mask&0x04) translate.z += *framedata++ * p->channelscale[2];

            rotate.x = p->channeloffset[3]; if(p->mask&0x08) rotate.x += *framedata++ * p->channelscale[3];
            rotate.y = p->channeloffset[4]; if(p->mask&0x10) rotate.y += *framedata++ * p->channelscale[4];
            rotate.z = p->channeloffset[5]; if(p->mask&0x20) rotate.z += *framedata++ * p->channelscale[5];
            rotate.w = p->channeloffset[6]; if(p->mask&0x40) rotate.w += *framedata++ * p->channelscale[6];

            scale.x = p->channeloffset[7]; if(p->mask&0x80)  scale.x += *framedata++ * p->channelscale[7];
            scale.y = p->channeloffset[8]; if(p->mask&0x100) scale.y += *framedata++ * p->channelscale[8];
            scale.z = p->channeloffset[9]; if(p->mask&0x200) scale.z += *framedata++ * p->channelscale[9];

            // Concatenate each pose with the inverse base pose to avoid doing this at animation time.
            // If the joint has a parent, then it needs to be pre-concatenated with its parent's base pose.
            // Thus it all negates at animation time like so:
            //   (parentPose * parentInverseBasePose) * (parentBasePose * childPose * childInverseBasePose) =>
            //   parentPose * (parentInverseBasePose * parentBasePose) * childPose * childInverseBasePose =>
            //   parentPose * childPose * childInverseBasePose

            mat34 m; compose34(m, translate, normq(rotate), scale);
            if(p->parent >= 0) multiply34x3(frames[i*hdr->num_poses + j], baseframe[p->parent], m, inversebaseframe[j]);
            else multiply34x2(frames[i*hdr->num_poses + j], m, inversebaseframe[j]);
        }
    }

    for(int i = 0; i < (int)hdr->num_anims; i++) {
        struct iqmanim *a = &anims[i];
        PRINTF("loaded anim[%d]: %s\n", i, &str[a->name]);
    }

    return true;
}

static
bool model_load_textures(iqm_t *q, const struct iqmheader *hdr) {
    textures = CALLOC(hdr->num_meshes, sizeof(GLuint));

    for(int i = 0; i < (int)hdr->num_meshes; i++) {
        struct iqmmesh *m = &meshes[i];
        textures[i] = 0;

        const char *str = hdr->ofs_text ? (char *)&buf[hdr->ofs_text] : "";
        PRINTF("loaded mesh: %s\n", &str[m->name]);

        #if 0
        // diffuse+translucency,normal,specular+emissive,metallic+smoothness,ao/cavity or subdermis
        const char *pf = file_pathdir(pathfile);
        const char *suffixes[] = { "", "_d", "_n", "_s", "_m", "_a", 0};
        const char *extensions[] = { "", ".jpg", ".png", ".tga", ".ktx", ".dds", 0 };
        const char *directories[] = { "", pf, 0 };
        for( int dir = 0; directories[dir] && !textures[i]; ++dir ) {
            for( int ext = 0; extensions[ext] && !textures[i]; ++ext ) {
                for( int suf = 0; suffixes[suf] && !textures[i]; ++suf ) {
                    textures[i] = texture(stringf("%s%s%s%s", directories[dir], &str[m->material], suffixes[suf], extensions[ext]), TEXTURE_FLIP ).id;
                }
            }
        }
        #else
        textures[i] = texture( file_find(&str[m->material]), 0 ).id;
        #endif

        if( textures[i] != texture_checker().id) {
            PRINTF("loaded material[%d]: %s\n", i, &str[m->material]);
        } else {
            PRINTF("fail: material[%d] not found: %s\n", i, &str[m->material]);
            PRINTF("warn: using placeholder material[%d]=texture_checker\n", i);
            textures[i] = texture_checker().id; // placeholder
        }
    }

    return true;
}

model_t model(const char *filename, int flags) {
    int len;  // vfs_pushd(filedir(filename))
    char *ptr = vfs_load(filename, &len); // + vfs_popd
    return model_from_mem( ptr, len, flags );
}
model_t model_from_mem(const void *mem, int len, int flags) {
    const char *ptr = (const char *)mem;
    static int shaderprog = -1;
    if( shaderprog < 0 ) {
        const char* vs =
            ""
            "#ifndef MAX_BONES\n"
            "#define MAX_BONES 110\n"
            "#endif\n"
            "uniform mat3x4 vsBoneMatrix[MAX_BONES];\n"
            "uniform bool SKINNED = false;\n"
            "in vec3 att_position;\n"
            "in vec2 att_texcoord;\n"
            "in vec3 att_normal;\n"
            "in vec4 att_tangent;\n"
            "in vec4 att_indexes;\n"
            "in vec4 att_weights;\n"
            "in vec4 att_color;\n"
            "in vec3 att_bitangent;\n"
            "out vec3 v_position;\n"
            "out vec3 v_normal;\n"
            "out vec2 v_texcoord;\n"
            // "uniform mat4 M;\n" // RIM
            "uniform mat4 MVP;\n"
            "void main() {\n"
            "   vec3 objPos;\n"
            "   if(!SKINNED) {\n"
            "       objPos = att_position;\n"
            "       v_normal = att_normal;\n"
            "   } else {\n"
            "       mat3x4 m = vsBoneMatrix[int(att_indexes.x)] * att_weights.x;\n"
            "       m += vsBoneMatrix[int(att_indexes.y)] * att_weights.y;\n"
            "       m += vsBoneMatrix[int(att_indexes.z)] * att_weights.z;\n"
            "       m += vsBoneMatrix[int(att_indexes.w)] * att_weights.w;\n"
            "       objPos = vec4(att_position, 1.0) * m;\n"
            "       v_normal = vec4(att_normal, 0.0) * m;\n"
            "       //@todo: tangents\n"
            "   }\n"
            "   v_position = att_position;\n"
            "   v_texcoord = att_texcoord;\n"
            "   gl_Position = MVP * vec4( objPos, 1.0 );\n"
            "}\n";
        const char* fs =
            ""
            "in vec3 v_normal;\n"
            "in vec2 v_texcoord;\n"
            "out vec4 fragColor;\n"
            "uniform sampler2D fsDiffTex;\n"
            "uniform sampler2D fsNormalTex;\n"
            "uniform sampler2D fsPositionTex;\n"
            "uniform mat4 MVP;\n"
            "void main() {\n"
            "    vec4 diff = texture(fsDiffTex, v_texcoord).rgba;\n"
            "    vec3 n = normalize(mat3(MVP) * v_normal); // transform normal to eye space\n"
            "    fragColor = diff;// * vec4(v_normal.xyz, 1);\n"
            "}\n";
        shaderprog = shader(vs,stringf(/*"#define RIM\n"*/ "%s", fragment_shader_32_4), //fs,
            "att_position,att_texcoord,att_normal,att_tangent,att_indexes,att_weights,att_color,att_bitangent","fragColor");
    }

    iqm_t *q = CALLOC(1, sizeof(iqm_t));
    program = shaderprog;

    int error = 1;
    if( ptr && len ) {
        struct iqmheader hdr; memcpy(&hdr, ptr, sizeof(hdr)); ptr += sizeof(hdr);
        if( !memcmp(hdr.magic, IQM_MAGIC, sizeof(hdr.magic))) {
            lil32p(&hdr.version, (sizeof(hdr) - sizeof(hdr.magic))/sizeof(uint32_t));
            if(hdr.version == IQM_VERSION) {
                buf = CALLOC(hdr.filesize, sizeof(uint8_t));
                memcpy(buf + sizeof(hdr), ptr, hdr.filesize - sizeof(hdr));
                error = 0;
                if( hdr.num_meshes > 0 && !(flags & MODEL_NO_MESHES) )     error |= !model_load_meshes(q, &hdr);
                if( hdr.num_meshes > 0 && !(flags & MODEL_NO_TEXTURES) )   error |= !model_load_textures(q, &hdr);
                if( hdr.num_anims  > 0 && !(flags & MODEL_NO_ANIMATIONS) ) error |= !model_load_anims(q, &hdr);
                if( buf != meshdata && buf != animdata ) FREE(buf);
            }
        }
    }

    model_t m = {0};
    if( error ) {
        PRINTF("Error: cannot load %s", "model");
        FREE(q), q = 0;
    } else {
        // m.boxes = bounds; // <@todo
        m.num_meshes = nummeshes;
        m.num_triangles = numtris;
        m.num_joints = numjoints;
        //m.num_poses = numposes;
        m.num_anims = numanims;
        m.num_frames = numframes;
        m.iqm = q;
        m.curframe = model_animate(m, 0);
        id44(m.pivot);
    }
    return m;
}

void model_get_bone_pose(model_t m, float curframe, int joint, vec3 *pos, vec3 *from) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    // mat34 *mat = &frames[(int)curframe * numjoints];
    float *a = outframe[joint];

#if 0
           mat34 m34 = {0};
           muladd34(m34, outframe[int(att_indexes.x)], att_weights.x);
           muladd34(m34, outframe[int(att_indexes.y)], att_weights.y);
           muladd34(m34, outframe[int(att_indexes.z)], att_weights.z);
           muladd34(m34, outframe[int(att_indexes.w)], att_weights.w);
           objPos = vec4(att_position, 1.0) * m34;
#endif

    *pos = vec3(a[12], a[13], a[14]);

    if (joints[joint].parent >= 0) {
        float *b = outframe[joints[joint].parent];

        /*
        @fixme: do as above
        */

        *from = vec3(b[12], b[13], b[14]);
    } else {
        *from = vec3(0, 0, 0);
    }
}

float model_animate_clip(model_t m, float curframe, int minframe, int maxframe, bool loop) {
    if(!m.iqm) return -1;
    iqm_t *q = m.iqm;

    float retframe = -1;
    if( numframes > 0 ) {
        int frame1 = (int)/*floor*/(curframe);
        int frame2 = frame1 + (curframe >= m.curframe ? 1 : -1);
        float frameoffset = curframe - frame1;

        if( loop ) {
            int distance = (maxframe - minframe);
            frame1 = frame1 >= maxframe ? minframe : frame1 < minframe ? maxframe - clampf(minframe - frame1, 0, distance) : frame1;
            frame2 = frame2 >= maxframe ? minframe : frame2 < minframe ? maxframe - clampf(minframe - frame2, 0, distance) : frame2;
            retframe = frame1 + frameoffset;
        } else {
            frame1 = clampf(frame1, minframe, maxframe);
            frame2 = clampf(frame2, minframe, maxframe);
            retframe = minf(frame1 + frameoffset, maxframe); // clamp to maxframe
        }

        mat34 *mat1 = &frames[frame1 * numjoints];
        mat34 *mat2 = &frames[frame2 * numjoints];
        // @todo: add animation blending and inter-frame blending here
        // Interpolate matrixes between the two closest frames and concatenate with
        // parent matrix if necessary. Concatenate the result with the inverse of the base pose.
        for(int i = 0; i < numjoints; i++) {
            mat34 mat; lerp34(mat, mat1[i], mat2[i], frameoffset);
            if(joints[i].parent >= 0) multiply34x2(outframe[i], outframe[joints[i].parent], mat);
            else copy34(outframe[i], mat);
        }

        // model_render_skeleton
        if(0)
        for( int i = 0; i < numjoints; i++ ) {
            vec3 pos, from;
            model_get_bone_pose(m, curframe, i, &pos, &from);
            ddraw_line(pos, from);
        }
    }

    return retframe;
}

float model_animate(model_t m, float curframe) {
    if(!m.iqm) return -1;
    iqm_t *q = m.iqm;
    return model_animate_clip(m, curframe, 0, numframes-1, true);
}

void model_render(model_t m, mat44 mvp) {
    if(!m.iqm) return;
    iqm_t *q = m.iqm;

    glBindVertexArray( vao );

    glUseProgram(program);
    // glUniformMatrix4fv( glGetUniformLocation(program, "M"), 1, GL_FALSE/*GL_TRUE*/, m); // RIM
    glUniformMatrix4fv( glGetUniformLocation(program, "MVP"), 1, GL_FALSE/*GL_TRUE*/, mvp);
    glUniformMatrix3x4fv( glGetUniformLocation(program, "vsBoneMatrix"), numjoints, GL_FALSE, outframe[0]);
    glUniform1i( glGetUniformLocation(program, "SKINNED"), numanims ? GL_TRUE : GL_FALSE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, position) );
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, texcoord) );
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, normal) );
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex, tangent) );

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    if(numframes > 0) {
        glVertexAttribPointer(4, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,blendindexes) );
        glVertexAttribPointer(5, 4, GL_UNSIGNED_BYTE, GL_TRUE,  sizeof(iqm_vertex), (GLvoid*)offsetof(iqm_vertex,blendweights) );
        glEnableVertexAttribArray(4);
        glEnableVertexAttribArray(5);
    }

    // 6 color
    // 7 bitangent? into texcoord.z?

    struct iqmtriangle *tris = NULL;
    for(int i = 0; i < nummeshes; i++) {
        struct iqmmesh *m = &meshes[i];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[i] );
        glUseProgram(program);
        glUniform1i(glGetUniformLocation(program, "fsDiffTex"), 0 /*<-- unit!*/ );

        glDrawElements(GL_TRIANGLES, 3*m->num_triangles, GL_UNSIGNED_INT, &tris[m->first_triangle]);
        profile_incstat("drawcalls", +1);
        profile_incstat("triangles", +m->num_triangles);
    }

    glDisableVertexAttribArray(1);
    if(numframes > 0) {
        glDisableVertexAttribArray(4);
        glDisableVertexAttribArray(5);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static
aabb aabb_transform( aabb A, mat44 M) {
    // Based on "Transforming Axis-Aligned Bounding Boxes" by Jim Arvo, 1990
    aabb B = { {M[12],M[13],M[14]}, {M[12],M[13],M[14]} }; // extract translation from mat44
    for( int i = 0; i < 3; i++ )
    for( int j = 0; j < 3; j++ ) {
        float a = M[i*4+j] * j[&A.min.x]; // use mat33 from mat44
        float b = M[i*4+j] * j[&A.max.x]; // use mat33 from mat44
        if( a < b ) {
            i[&B.min.x] += a;
            i[&B.max.x] += b;
        } else {
            i[&B.min.x] += b;
            i[&B.max.x] += a;
        }
    }
    return B;
}

aabb model_aabb(model_t m, mat44 transform) {
    iqm_t *q = m.iqm;
    if( q && bounds ) {
    int f = ( (int)m.curframe ) % (numframes + !numframes);
    vec3 bbmin = ptr3(bounds[f].bbmin);
    vec3 bbmax = ptr3(bounds[f].bbmax);
    return aabb_transform(aabb(bbmin,bbmax), transform);
    }
    return aabb(vec3(0,0,0),vec3(0,0,0));
}

void model_destroy(model_t m) {
    iqm_t *q = m.iqm;
//    if(m.mesh) mesh_destroy(m.mesh);
    FREE(outframe);
    FREE(textures);
    FREE(baseframe);
    FREE(inversebaseframe);
    if(animdata != meshdata) FREE(animdata);
    //FREE(meshdata);
    FREE(frames);
    FREE(buf);
    FREE(q);
}

#undef program
#undef meshdata
#undef animdata
#undef nummeshes
#undef numtris
#undef numverts
#undef numjoints
#undef numframes
#undef numanims
#undef meshes
#undef textures
#undef joints
#undef poses
#undef anims
#undef baseframe
#undef inversebaseframe
#undef outframe
#undef frames
#undef vao
#undef ibo
#undef vbo
#undef bonematsoffset
#undef buf
#undef bounds

#endif // RENDER_C
