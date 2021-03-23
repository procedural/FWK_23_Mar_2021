// naive debugdraw framework
// - rlyeh, public domain.
//
// Credits: Based on work by @glampert https://github.com/glampert/debug-draw (PD)
// [x] grid, axis, frustum, cube, sphere, triangle, square, pentagon, hexagon, circle, normal.
// [x] arrow, point, text, capsule, aabb
// [ ] plane, bone (pyramid?), gizmo (proper), ring, floatzilla locator,
// [ ] camera, light bulb, light probe,
// [x] line batching
// [*] line width and stipple

#ifndef RENDERDD_H
#define RENDERDD_H

void ddraw_color(unsigned rgb);
//
void ddraw_aabb(vec3 minbb, vec3 maxbb);
void ddraw_aabb_corners(vec3 minbb, vec3 maxbb);
void ddraw_arrow(vec3 begin, vec3 end);
void ddraw_axis(float units);
void ddraw_bone(vec3 center, vec3 end);
void ddraw_bounds(const vec3 points[8]);
void ddraw_box(vec3 c, vec3 extents);
void ddraw_capsule(vec3 from, vec3 to, float radius);
void ddraw_circle(vec3 pos, float radius);
void ddraw_cone(vec3 center, vec3 top, float radius);
void ddraw_cube(vec3 center, float radius);
void ddraw_diamond(vec3 from, vec3 to, float size);
void ddraw_frustum(float projview[16]);
void ddraw_gizmo(vec3 center);
void ddraw_grid(float scale);
void ddraw_hexagon(vec3 pos, float radius);
void ddraw_line(vec3 from, vec3 to);
void ddraw_line_dash(vec3 from, vec3 to);
void ddraw_line_thin(vec3 from, vec3 to);
void ddraw_normal(vec3 pos, vec3 n);
void ddraw_pentagon(vec3 pos, float radius);
void ddraw_plane(vec3 p, vec3 n, float scale);
void ddraw_point(vec3 from);
void ddraw_pyramid(vec3 center, float height, int segments);
void ddraw_sphere(vec3 pos, float radius);
void ddraw_square(vec3 pos, float radius);
void ddraw_text(vec3 pos, float scale, const char *text);
void ddraw_text2d(vec2 pos, float scale, const char *text);
void ddraw_triangle(vec3 p1, vec3 p2, vec3 p3);
//
void ddraw_demo();

#define ddraw_text(pos, scale, ...) ddraw_text(pos, scale, stringf(__VA_ARGS__))
#define ddraw_text2d(pos, scale, ...) ddraw_text2d(pos, scale, stringf(__VA_ARGS__))

#endif


#ifdef RENDERDD_C
#pragma once

static const char *dd_vs =
    "#version 130\n"
    "in vec3 att_position;\n"
    "uniform mat4 u_MVP;\n"
    "uniform vec3 u_color;\n"
    "out vec3 out_color;\n"
    "void main() {\n"
    "    gl_Position = u_MVP * vec4(att_position, 1.0);\n"
    "    gl_PointSize = 4.0; /* for GL_POINTS draw commands */\n"
    "    out_color = u_color;\n"
    "}";

static const char *dd_fs =
    "#version 130\n"
    "precision mediump float;\n"
    "in vec3 out_color;\n"
    "out vec4 fragcolor;\n"
    "void main() {\n"
    "   fragcolor = vec4(out_color, 1.0);\n"
    "}";

#define X RGB_HEX
const uint32_t pal[32] = { // pico8 secret palette (CC0, public domain)
X(000000),X(1D2B53),X(7E2553),X(008751),X(AB5236),X(5F574F),X(C2C3C7),X(FFF1E8), /*00.07*/
X(FF004D),X(FFA300),X(FFEC27),X(00E436),X(29ADFF),X(83769C),X(FF77A8),X(FFCCAA), /*08.15*/
X(291814),X(111D35),X(422136),X(125359),X(742F29),X(49333B),X(A28879),X(F3EF7D), /*16.23*/
X(BE1250),X(FF6C24),X(A8E72E),X(00B543),X(065AB5),X(754665),X(FF6E59),X(FF9D81), /*24.31*/
};
#undef X

static uint32_t    dd_color = ~0u;
static GLuint      dd_program = -1;
static int         dd_u_color = -1;
static map(unsigned,array(vec3)) dd_lists[3] = {0}; // [0] thin lines, [1] thick lines, [2] points
static bool        dd_use_line = 0;

static
void ddraw_flush() {
    glEnable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);

    mat44 mvp;
    multiply44x2(mvp, camera_get_active()->proj, camera_get_active()->view); // MVP where M=id

    glUseProgram(dd_program);
    glUniformMatrix4fv(glGetUniformLocation(dd_program, "u_MVP"), 1, GL_FALSE, mvp);

    static GLuint vao, vbo;
    if(!vao) glGenVertexArrays(1, &vao);    glBindVertexArray(vao);
    if(!vbo) glGenBuffers(1, &vbo);         glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);

    glDepthFunc(GL_LEQUAL);
    glEnable(GL_PROGRAM_POINT_SIZE); // for GL_POINTS
    glEnable(GL_LINE_SMOOTH); // for GL_LINES (thin)

    for( int i = 0; i < 3; ++i ) { // [0] thin, [1] thick, [2] points
        GLenum mode = i < 2 ? GL_LINES : GL_POINTS;
        glLineWidth(i == 1 ? 1 : 0.3); // 0.625);
        for each_map(dd_lists[i], unsigned, rgb, array(vec3), list) {
            int count = array_count(list);
            if(!count) continue;
                // color
                vec3 rgbf = {((rgb>>16)&255)/255.f,((rgb>>8)&255)/255.f,((rgb>>0)&255)/255.f};
                glUniform3fv(dd_u_color, GL_TRUE, &rgbf.x);
                // config vertex data
                glBufferData(GL_ARRAY_BUFFER, count * 3 * 4, list, GL_STATIC_DRAW);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
                // feed vertex data
                glDrawArrays(mode, 0, count);
                profile_incstat("drawcalls", +1);
                profile_incstat(i < 2 ? "lines" : "points", count);
            array_clear(list);
        }
    }

    if(0)
    {
        // ddraw_text2d(vec3(0,0,1), "hello world\n123");
        float mvp[16]; float zdepth_max = 1;
        ortho44(mvp, -window_width()/2, window_width()/2, -window_height()/2, window_height()/2, -1, 1);
        translate44(mvp, -window_width()/2, window_height()/2, 0);
        glUniformMatrix4fv(glGetUniformLocation(dd_program, "u_MVP"), 1, GL_FALSE, mvp);
        ddraw_color(BLACK);
        for(int i = 0; i < 10; ++i)
        ddraw_text(vec3(window_width()/2,-(i * 12),0), 0.5, "\nhello world"); // scale 0.5 is like 12units each
        ddraw_color(WHITE);
        for(int i = 0; i < 10; ++i)
        ddraw_text(vec3(window_width()/2+1,-(i * 12)-1,0), 0.5, "\nhello world"); // scale 0.5 is like 12units each
        for( int i = 0; i < 3; ++i ) { // [0] thin, [1] thick, [2] points
            GLenum mode = i < 2 ? GL_LINES : GL_POINTS;
            glLineWidth(i == 1 ? 1 : 0.3); // 0.625);
            for each_map(dd_lists[i], unsigned, rgb, array(vec3), list) {
                int count = array_count(list);
                if(!count) continue;
                    // color
                    vec3 rgbf = {((rgb>>16)&255)/255.f,((rgb>>8)&255)/255.f,((rgb>>0)&255)/255.f};
                    glUniform3fv(dd_u_color, GL_TRUE, &rgbf.x);
                    // config vertex data
                    glBufferData(GL_ARRAY_BUFFER, count * 3 * 4, list, GL_STATIC_DRAW);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
                    // feed vertex data
                    glDrawArrays(mode, 0, count);
                    profile_incstat("drawcalls", +1);
                    profile_incstat(i < 2 ? "lines" : "points", count);
                array_clear(list);
            }
        }
    }

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_PROGRAM_POINT_SIZE);

    glBindVertexArray(0);

    ddraw_color(WHITE); // reset color for next drawcall
}

void ddraw_color(unsigned rgb) {
    dd_color = rgb;
}
void ddraw_point(vec3 from) {
#if 0
    array(vec3) *found = map_find(dd_lists[2], dd_color);
    if(!found) found = map_insert(dd_lists[2], dd_color, 0);
#else
    array(vec3) *found = map_find_or_add(dd_lists[2], dd_color, 0);
#endif
    array_push(*found, from);
}
void ddraw_line_thin(vec3 from, vec3 to) { // thin lines
#if 0
    array(vec3) *found = map_find(dd_lists[0], dd_color);
    if(!found) found = map_insert(dd_lists[0], dd_color, 0);
#else
    array(vec3) *found = map_find_or_add(dd_lists[0], dd_color, 0);
#endif
    array_push(*found, from);
    array_push(*found, to);
}
void ddraw_line(vec3 from, vec3 to) { // thick lines
#if 0
    array(vec3) *found = map_find(dd_lists[1], dd_color);
    if(!found) found = map_insert(dd_lists[1], dd_color, 0);
#else
    array(vec3) *found = map_find_or_add(dd_lists[1], dd_color, 0);
#endif
    array_push(*found, from);
    array_push(*found, to);
}
void ddraw_line_dash(vec3 from, vec3 to) { // thick lines
    vec3 dist = sub3(to, from); vec3 unit = norm3(dist);
    for( float len = 0, mag = len3(dist) / 2; len < mag; ++len ) {
        to = add3(from, unit);
        ddraw_line(from, to);
        from = add3(to, unit);
    }
}
void ddraw_triangle(vec3 pa, vec3 pb, vec3 pc) {
    ddraw_line(pa, pb);
    ddraw_line(pa, pc);
    ddraw_line(pb, pc);
}
void ddraw_axis(float units) {
    ddraw_color(RED);   ddraw_line(vec3(0,0,0), vec3(units,0,0)); ddraw_line_dash(vec3(0,0,0), vec3(-units,0,0));
    ddraw_color(GREEN); ddraw_line(vec3(0,0,0), vec3(0,units,0)); ddraw_line_dash(vec3(0,0,0), vec3(0,-units,0));
    ddraw_color(BLUE);  ddraw_line(vec3(0,0,0), vec3(0,0,units)); ddraw_line_dash(vec3(0,0,0), vec3(0,0,-units));
}
void ddraw_grid_(float scale) { // 10x10
    ddraw_color( WHITE ); // outer
    for( float i = -scale, c = 0; c <= 20; c += 20, i += c * (scale/10) ) {
        ddraw_line(vec3(-scale,0,i), vec3(+scale,0,i)); // horiz
        ddraw_line(vec3(i,0,-scale), vec3(i,0,+scale)); // vert
    }
    ddraw_color( GRAY ); // inner
    for( float i = -scale + scale/10, c = 1; c < 20; ++c, i += (scale/10) ) {
        ddraw_line_thin(vec3(-scale,0,i), vec3(+scale,0,i)); // horiz
        ddraw_line_thin(vec3(i,0,-scale), vec3(i,0,+scale)); // vert
    }
}
void ddraw_grid(float scale) {
    if( scale ) {
        ddraw_grid_(scale);
    } else {
        ddraw_grid_(100);
        ddraw_grid_(10);
        ddraw_grid_(1);
        ddraw_grid_(0.1);
        ddraw_grid_(0.01);
    }
    ddraw_axis(scale ? scale : 100);
}


void (ddraw_text)(vec3 pos, float scale, const char *text) {
    // [ref] http://paulbourke.net/dataformats/hershey/ (PD)
    // [ref] https://sol.gfxile.net/hershey/fontprev.html (WTFPL2)
    static const signed char *hershey[] = { /* simplex font */
    "AQ","IKFVFH@@FCEBFAGBFC","FQEVEO@@MVMO","LVLZE:@@RZK:@@EMSM@@DGRG","[UIZI=@@MZ"
    "M=@@RSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","`YVVDA@@IVKTKRJPHOFODQDSEUGVIVK"
    "UNTQTTUVV@@RHPGOEOCQASAUBVDVFTHRH","c[XMXNWOVOUNTLRGPDNBLAHAFBECDEDGEIFJMNNOOQ"
    "OSNULVJUISIQJNLKQDSBUAWAXBXC","HKFTEUFVGUGSFQEP","KOLZJXHUFQELEHFCH?J<L:","KOD"
    "ZFXHUJQKLKHJCH?F<D:","IQIVIJ@@DSNM@@NSDM","F[NSNA@@EJWJ","IKGBFAEBFCGBG@F>E=",\
    "C[EJWJ","FKFCEBFAGBFC","CWUZC:","RUJVGUERDMDJEEGBJALAOBQERJRMQROULVJV","EUGRIS"
    "LVLA","OUEQERFTGUIVMVOUPTQRQPPNNKDARA","PUFVQVKNNNPMQLRIRGQDOBLAIAFBECDE","GUN"
    "VDHSH@@NVNA","RUPVFVEMFNIOLOONQLRIRGQDOBLAIAFBECDE","XUQSPUMVKVHUFREMEHFDHBKAL"
    "AOBQDRGRHQKOMLNKNHMFKEH","FURVHA@@DVRV","^UIVFUESEQFOHNLMOLQJRHREQCPBMAIAFBECD"
    "EDHEJGLJMNNPOQQQSPUMVIV","XUQOPLNJKIJIGJELDODPESGUJVKVNUPSQOQJPENBKAIAFBED","L"
    "KFOENFMGNFO@@FCEBFAGBFC","OKFOENFMGNFO@@GBFAEBFCGBG@F>E=","DYUSEJUA","F[EMWM@@"
    "EGWG","DYESUJEA","USDQDRETFUHVLVNUOTPRPPONNMJKJH@@JCIBJAKBJC","x\\SNRPPQMQKPJO"
    "ILIIJGLFOFQGRI@@MQKOJLJIKGLF@@SQRIRGTFVFXHYKYMXPWRUTSUPVMVJUHTFREPDMDJEGFEHCJB"
    "MAPASBUCVD@@TQSISGTF","ISJVBA@@JVRA@@EHOH","XVEVEA@@EVNVQURTSRSPRNQMNL@@ELNLQK"
    "RJSHSERCQBNAEA","SVSQRSPUNVJVHUFSEQDNDIEFFDHBJANAPBRDSF","PVEVEA@@EVLVOUQSRQSN"
    "SIRFQDOBLAEA","LTEVEA@@EVRV@@ELML@@EARA","ISEVEA@@EVRV@@ELML","WVSQRSPUNVJVHUF"
    "SEQDNDIEFFDHBJANAPBRDSFSI@@NISI","IWEVEA@@SVSA@@ELSL","CIEVEA","KQMVMFLCKBIAGA"
    "EBDCCFCH","IVEVEA@@SVEH@@JMSA","FREVEA@@EAQA","LYEVEA@@EVMA@@UVMA@@UVUA","IWEV"
    "EA@@EVSA@@SVSA","VWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV","NVEVEA@@EVNVQU"
    "RTSRSORMQLNKEK","YWJVHUFSEQDNDIEFFDHBJANAPBRDSFTITNSQRSPUNVJV@@MES?","QVEVEA@@"
    "EVNVQURTSRSPRNQMNLEL@@LLSA","UURSPUMVIVFUDSDQEOFNHMNKPJQIRGRDPBMAIAFBDD","FQIV"
    "IA@@BVPV","KWEVEGFDHBKAMAPBRDSGSV","FSBVJA@@RVJA","LYCVHA@@MVHA@@MVRA@@WVRA",""
    "FUDVRA@@RVDA","GSBVJLJA@@RVJL","IURVDA@@DVRV@@DARA","LOEZE:@@FZF:@@EZLZ@@E:L:",
    "COAVO>","LOJZJ:@@KZK:@@DZKZ@@D:K:","KQGPISKP@@DMIRNM@@IRIA","CQA?Q?","HKGVFUES"
    "EQFPGQFR","RTPOPA@@PLNNLOIOGNELDIDGEDGBIALANBPD","RTEVEA@@ELGNIOLONNPLQIQGPDNB"
    "LAIAGBED","OSPLNNLOIOGNELDIDGEDGBIALANBPD","RTPVPA@@PLNNLOIOGNELDIDGEDGBIALANB"
    "PD","RSDIPIPKOMNNLOIOGNELDIDGEDGBIALANBPD","IMKVIVGUFRFA@@COJO","WTPOP?O<N;L:I"
    ":G;@@PLNNLOIOGNELDIDGEDGBIALANBPD","KTEVEA@@EKHNJOMOONPKPA","IIDVEUFVEWDV@@EOE"
    "A","LKFVGUHVGWFV@@GOG>F;D:B:","IREVEA@@OOEE@@IIPA","CIEVEA","S_EOEA@@EKHNJOMOO"
    "NPKPA@@PKSNUOXOZN[K[A","KTEOEA@@EKHNJOMOONPKPA","RTIOGNELDIDGEDGBIALANBPDQGQIP"
    "LNNLOIO","RTEOE:@@ELGNIOLONNPLQIQGPDNBLAIAGBED","RTPOP:@@PLNNLOIOGNELDIDGEDGBI"
    "ALANBPD","INEOEA@@EIFLHNJOMO","RROLNNKOHOENDLEJGILHNGOEODNBKAHAEBDD","IMFVFEGB"
    "IAKA@@COJO","KTEOEEFBHAKAMBPE@@POPA","FQCOIA@@OOIA","LWDOHA@@LOHA@@LOPA@@TOPA",
    "FRDOOA@@OODA","JQCOIA@@OOIAG=E;C:B:","IROODA@@DOOO@@DAOA","hOJZHYGXFVFTGRHQIOI"
    "MGK@@HYGWGUHSIRJPJNILEJIHJFJDIBHAG?G=H;@@GIIGIEHCGBF@F>G<H;J:","CIEZE:","hOFZH"
    "YIXJVJTIRHQGOGMIK@@HYIWIUHSGRFPFNGLKJGHFFFDGBHAI?I=H;@@IIGGGEHCIBJ@J>I<H;F:",""
    "XYDGDIELGMIMKLOIQHSHUIVK@@DIEKGLILKKOHQGSGUHVKVM" };
    vec3 src = pos, old = {0};
    for( signed char c; (c = *text++, c > 0 && c < 127); ) {
        if( c == '\n' || c == '\r' ) {
            pos.x = src.x, pos.y -= scale * (hershey['W'-32][1] - 65) * 1.25f; // spacing @1
        } else {
            char *glyph = (char*)hershey[c - 32];
            if( c > 32 ) for( int pen = 0, i = 0; i < (glyph[0] - 65); i++ ) { // verts @0
                int x = glyph[2 + i*2 + 0] - 65, y = glyph[2 + i*2 + 1] - 65;
                if( x == -1 && y == -1 ) pen = 0; else {
                    vec3 next = add3(pos, vec3(scale*x, scale*y, 0));
                    if( !pen ) pen = 1; else ddraw_line(old, next);
                    old = next;
                }
            }
            pos.x += scale * (glyph[1] - 65); // spacing @1
        }
    }
}

/*
void ddraw_prism(vec3 pos, int segments) {
    ddraw_color(vec3(1,1,1));
    float cycle = 2 * 3.14159 + 2 * 3.14159 / segments, acc = 0;
    for( int i = 0; i < segments; ++i, acc += cycle ) {
        ddraw_line(add3(pos,vec3(cos(acc),0,sin(acc))), add3(pos,vec3(cos(acc+cycle),0,sin(acc+cycle))));
    }
}
*/
void ddraw_prism(vec3 center, float radius, float height, vec3 normal, int segments) {
    vec3 left = {0}, up = {0};
    ortho3(&left, &up, normal);

    vec3 point, lastPoint;
    up = scale3(up, radius);
    left = scale3(left, radius);
    lastPoint = add3(center, up);
    vec3 pivot = add3(center, scale3(normal, height));

    for (int i = 1; i <= segments; ++i) {
        const float radians = (C_PI * 2) * i / segments;

        vec3 vs = scale3(left, sinf(radians));
        vec3 vc = scale3(up,   cosf(radians));

        point = add3(center, vs);
        point = add3(point,  vc);

        ddraw_line(lastPoint, point);
        if( height > 0 ) ddraw_line(point, pivot);
        else if(height < 0) {
            ddraw_line(point, add3(point,scale3(normal, -height)));
        }
        lastPoint = point;
    }

    if(height < 0) ddraw_prism(add3(center, scale3(normal, -height)), radius, 0, normal, segments);
}
void ddraw_cube(vec3 center, float radius) { // draw_prism(center, 1, -1, vec3(0,1,0), 4);
    float half = radius * 0.5f;
    vec3 l = vec3(center.x-half,center.y+half,center.z-half); // left-top-far
    vec3 r = vec3(center.x+half,center.y-half,center.z+half); // right-bottom-near

    ddraw_line(l, vec3(r.x,l.y,l.z));
    ddraw_line(vec3(r.x,l.y,l.z), vec3(r.x,l.y,r.z));
    ddraw_line(vec3(r.x,l.y,r.z), vec3(l.x,l.y,r.z));
    ddraw_line(vec3(l.x,l.y,r.z), l);
    ddraw_line(l, vec3(l.x,r.y,l.z));

    ddraw_line(r, vec3(l.x,r.y,r.z));
    ddraw_line(vec3(l.x,r.y,r.z), vec3(l.x,r.y,l.z));
    ddraw_line(vec3(l.x,r.y,l.z), vec3(r.x,r.y,l.z));
    ddraw_line(vec3(r.x,r.y,l.z), r);
    ddraw_line(r, vec3(r.x,l.y,r.z));

    ddraw_line(vec3(l.x,l.y,r.z), vec3(l.x,r.y,r.z));
    ddraw_line(vec3(r.x,l.y,l.z), vec3(r.x,r.y,l.z));
}
void ddraw_normal(vec3 pos, vec3 n) {
    ddraw_color(YELLOW);
    ddraw_line(pos, add3(pos, norm3(n)));
}

void ddraw_circle(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 24); } // ddraw_pyramid(pos, 0, 5+5+2);
void ddraw_hexagon(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 6); }
void ddraw_pentagon(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 5); }
void ddraw_square(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 4); }
//void ddraw_triangle(vec3 pos, float r) { ddraw_prism(pos, r, 0, vec3(0,1,0), 3); }
void ddraw_sphere(vec3 center, float radius) {
    float lod = 15, yp = -radius, rp = 0, y, r, x, z;
    for( int j = 1; j <= lod / 2; ++j, yp = y, rp = r ) {
        y = j * 2.f / (lod / 2) - 1;
        r = cosf(y * 3.14159f / 2) * radius;
        y = sinf(y * 3.14159f / 2) * radius;

        float xp = 1, zp = 0;
        for( int i = 1; i <= lod; ++i, xp = x, zp = z ) {
            x = 3.14159f * 2 * i / lod;
            z = sinf(x);
            x = cosf(x);

            vec3 a1 = add3(center, vec3(xp * rp, yp, zp * rp));
            vec3 b1 = add3(center, vec3(xp * r,  y,  zp * r));
            vec3 c1 = add3(center, vec3(x  * r,  y,  z  * r));

            ddraw_line(a1,b1);
            ddraw_line(b1,c1);
            ddraw_line(c1,a1);

            vec3 a2 = add3(center, vec3(xp * rp, yp, zp * rp));
            vec3 b2 = add3(center, vec3(x  * r,  y,  z  * r));
            vec3 c2 = add3(center, vec3(x  * rp, yp, z  * rp));

            ddraw_line(a2,b2);
            ddraw_line(b2,c2);
            ddraw_line(c2,a2);
        }
    }
}
void ddraw_box(vec3 c, vec3 extents) {
    vec3 points[8], whd = scale3(extents, 0.5f);
    #define DD_BOX_V(v, op1, op2, op3) (v).x = c.x op1 whd.x; (v).y = c.y op2 whd.y; (v).z = c.z op3 whd.z
    DD_BOX_V(points[0], -, +, +);
    DD_BOX_V(points[1], -, +, -);
    DD_BOX_V(points[2], +, +, -);
    DD_BOX_V(points[3], +, +, +);
    DD_BOX_V(points[4], -, -, +);
    DD_BOX_V(points[5], -, -, -);
    DD_BOX_V(points[6], +, -, -);
    DD_BOX_V(points[7], +, -, +);
    #undef DD_BOX_V
    ddraw_bounds(points);
}
void ddraw_capsule(vec3 from, vec3 to, float r) {
    /* calculate axis */
    vec3 up, right, forward;
    forward = sub3(to, from);
    forward = norm3(forward);
    ortho3(&right, &up, forward);

    /* calculate first two cone verts (buttom + top) */
    vec3 lastf, lastt;
    lastf = scale3(up,r);
    lastt = add3(to,lastf);
    lastf = add3(from,lastf);

    /* step along circle outline and draw lines */
    enum { step_size = 20 };
    for (int i = step_size; i <= 360; i += step_size) {
        /* calculate current rotation */
        vec3 ax = scale3(right, sinf(i*TO_RAD));
        vec3 ay = scale3(up, cosf(i*TO_RAD));

        /* calculate current vertices on cone */
        vec3 tmp = add3(ax, ay);
        vec3 pf = scale3(tmp, r);
        vec3 pt = scale3(tmp, r);

        pf = add3(pf, from);
        pt = add3(pt, to);

        /* draw cone vertices */
        ddraw_line(lastf, pf);
        ddraw_line(lastt, pt);
        ddraw_line(pf, pt);

        lastf = pf;
        lastt = pt;

        /* calculate first top sphere vert */
        vec3 prevt = scale3(tmp, r);
        vec3 prevf = add3(prevt, from);
        prevt = add3(prevt, to);

        /* sphere (two half spheres )*/
        for (int j = 1; j < 180/step_size; j++) {
            /* angles */
            float ta = j*step_size;
            float fa = 360-(j*step_size);

            /* top half-sphere */
            ax = scale3(forward, sinf(ta*TO_RAD));
            ay = scale3(tmp, cosf(ta*TO_RAD));

            vec3 t = add3(ax, ay);
            pf = scale3(t, r);
            pf = add3(pf, to);
            ddraw_line(pf, prevt);
            prevt = pf;

            /* bottom half-sphere */
            ax = scale3(forward, sinf(fa*TO_RAD));
            ay = scale3(tmp, cosf(fa*TO_RAD));

            t = add3(ax, ay);
            pf = scale3(t, r);
            pf = add3(pf, from);
            ddraw_line(pf, prevf);
            prevf = pf;
        }
    }
}
void ddraw_gizmo(vec3 center) { // @fixme: improve billboard for outer ring
    ddraw_color(RED);   ddraw_prism(center, 1.0, 0, vec3(1,0,0), 24);
    ddraw_color(GREEN); ddraw_prism(center, 1.0, 0, vec3(0,1,0), 24);
    ddraw_color(BLUE);  ddraw_prism(center, 1.0, 0, vec3(0,0,1), 24);

    ddraw_color(WHITE); ddraw_prism(center, 1.0, 0, norm3(camera_get_active()->look), 24);
}
void ddraw_pyramid(vec3 center, float height, int segments) {
    ddraw_prism(center, 1, height, vec3(0,1,0), segments);
}
void ddraw_diamond(vec3 from, vec3 to, float size) {
    poly p = diamond(from, to, size);
    vec3 *dmd = p.verts;

    vec3 *a = dmd + 0;
    vec3 *b = dmd + 1;
    vec3 *c = dmd + 2;
    vec3 *d = dmd + 3;
    vec3 *t = dmd + 4;
    vec3 *f = dmd + 5;

    /* draw vertices */
    ddraw_line(*a, *b);
    ddraw_line(*b, *c);
    ddraw_line(*c, *d);
    ddraw_line(*d, *a);

    /* draw roof */
    ddraw_line(*a, *t);
    ddraw_line(*b, *t);
    ddraw_line(*c, *t);
    ddraw_line(*d, *t);

    /* draw floor */
    ddraw_line(*a, *f);
    ddraw_line(*b, *f);
    ddraw_line(*c, *f);
    ddraw_line(*d, *f);

    poly_free(&p);
}
void ddraw_cone(vec3 center, vec3 top, float radius) {
    vec3 diff3 = sub3(top, center);
    ddraw_prism(center, radius ? radius : 1, len3(diff3), norm3(diff3), 24);
}
void ddraw_cone_lowres(vec3 center, vec3 top, float radius) {
    vec3 diff3 = sub3(top, center);
    ddraw_prism(center, radius ? radius : 1, len3(diff3), norm3(diff3), 3);
}
void ddraw_bone(vec3 center, vec3 end) {
    vec3 diff3 = sub3(end, center);
    float len = len3(diff3), len10 = len / 10;
    ddraw_prism(center, len10, 0, vec3(1,0,0), 24);
    ddraw_prism(center, len10, 0, vec3(0,1,0), 24);
    ddraw_prism(center, len10, 0, vec3(0,0,1), 24);
    ddraw_line(end, add3(center, vec3(0,+len10,0)));
    ddraw_line(end, add3(center, vec3(0,-len10,0)));
}
void ddraw_bounds(const vec3 points[8]) {
    for( int i = 0; i < 4; ++i ) {
        ddraw_line(points[i], points[(i + 1) & 3]);
        ddraw_line(points[i], points[4 + i]);
        ddraw_line(points[4 + i], points[4 + ((i + 1) & 3)]);
    }
}
void ddraw_bounds_corners(const vec3 points[8]) {
    for( int i = 0; i < 4; ++i ) {
        #define ddraw_unit(a,b) ddraw_line(a,add3(a,norm3(sub3(b,a)))), ddraw_line(b,add3(b,norm3(sub3(a,b))))
        ddraw_unit(points[i], points[(i + 1) & 3]);
        ddraw_unit(points[i], points[4 + i]);
        ddraw_unit(points[4 + i], points[4 + ((i + 1) & 3)]);
        #undef ddraw_unit
    }
}
void ddraw_aabb(vec3 minbb, vec3 maxbb) {
    vec3 points[8], bb[2] = { minbb, maxbb };
    for (int i = 0; i < 8; ++i) {
        points[i].x = bb[(i ^ (i >> 1)) & 1].x;
        points[i].y = bb[     (i >> 1)  & 1].y;
        points[i].z = bb[     (i >> 2)  & 1].z;
    }
    ddraw_bounds/*_corners*/(points);
}
void ddraw_frustum(float projview[16]) {
    mat44 clipmatrix = {0}; // clip matrix
    invert44(clipmatrix, projview);

    // Start with the standard clip volume, then bring it back to world space.
    const vec3 planes[8] = {
        {-1,-1,-1}, {+1,-1,-1}, {+1,+1,-1}, {-1,+1,-1}, // near plane
        {-1,-1,+1}, {+1,-1,+1}, {+1,+1,+1}, {-1,+1,+1}, // far plane
    };

    vec3 points[8];
    float wCoords[8];

    // Transform the planes by the inverse clip matrix:
    for( int i = 0; i < 8; ++i ) {
        // wCoords[i] = matTransformPointXYZW2(&points[i], planes[i], clipmatrix);
        vec3 *out = &points[i], in = planes[i]; const float *m = clipmatrix;
        out->x = (m[0] * in.x) + (m[4] * in.y) + (m[ 8] * in.z) + m[12]; // in.w (vec4) assumed to be 1
        out->y = (m[1] * in.x) + (m[5] * in.y) + (m[ 9] * in.z) + m[13];
        out->z = (m[2] * in.x) + (m[6] * in.y) + (m[10] * in.z) + m[14];
        wCoords[i] =  (m[3] * in.x) + (m[7] * in.y) + (m[11] * in.z) + m[15]; // rw

        // bail if any W ended up as zero.
        const float epsilon = 1e-9f;
        if (absf(wCoords[i]) < epsilon) {
            return;
        }
    }

    // Divide by the W component of each:
    for( int i = 0; i < 8; ++i ) {
        points[i].x /= wCoords[i];
        points[i].y /= wCoords[i];
        points[i].z /= wCoords[i];
    }

    // Connect the dots:
    ddraw_bounds(points);
}
void ddraw_arrow(vec3 begin, vec3 end) {
    vec3 diff = sub3(end, begin);
    float len = len3(diff);
    float stick_len = len * 2 / 3, head_radius = len / 6;

    ddraw_line(begin, end);
    ddraw_cone_lowres(add3(begin, scale3(norm3(diff), stick_len)), end, head_radius);
}

void ddraw_plane(vec3 p, vec3 n, float scale)  // @todo
{}

void ddraw_init() {
    for( int i = 0; i < 3; ++i ) map_init(dd_lists[i], less_int, hash_int);
    dd_program = shader(dd_vs,dd_fs,"att_position","fragcolor");
    dd_u_color = glGetUniformLocation(dd_program, "u_color");
    ddraw_flush(); // alloc vao & vbo, also resets color
}

void ddraw_demo() {
    // freeze current frame for (frustum) camera forensics
    static mat44 projview_copy;
    ONCE {
        multiply44x2(projview_copy, camera_get_active()->proj, camera_get_active()->view);
    }
    ddraw_frustum(projview_copy);

    //ddraw_grid();

    vec3 origin = {0,0,0};
    ddraw_color(ORANGE);
    ddraw_arrow(origin, vec3(-1,1,1));
    ddraw_color(YELLOW);
    ddraw_text(vec3(-1,1,1), 0.008f, "hello 1%s2!", "world");

    const char abc[] = " !\"#$%&'()*+,-./\n"
        "0123456789:;<=>?@\n"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`\n"
        "abcdefghijklmnopqrstuvwxyz{|}~";
    ddraw_text(vec3(2,2,2), 0.008f, abc);


    for( int i = -5; i <= 5; ++i ) {
        ddraw_pyramid(vec3(i*2,0,3),  0, i+5+2); ddraw_text(vec3(i*2,0,3), 0.008f, "%d/1", i);
        ddraw_pyramid(vec3(i*2,0,6), -2, i+5+2); ddraw_text(vec3(i*2,0,6), 0.008f, "%d/2", i);
        ddraw_pyramid(vec3(i*2,0,9), +2, i+5+2); ddraw_text(vec3(i*2,0,9), 0.008f, "%d/3", i);
    }

#if 1 // @fixme: add positions to these
    // ddraw_triangle(origin, 1);
    ddraw_square(origin, 1);
    ddraw_pentagon(origin, 1);
    ddraw_hexagon(origin, 1);
    ddraw_cube(origin, 1);
    ddraw_pyramid(origin, 2, 3);
    ddraw_pyramid(origin, 2, 16);
    ddraw_cone(origin, add3(origin, vec3(0,1,0)), 0.5f);
    ddraw_arrow(origin, vec3(0,1,0));
    ddraw_bone(vec3(0,0,0), vec3(3,3,3));
    ddraw_aabb(vec3(0,0,0), vec3(1,1,1));
#endif

    ddraw_gizmo(vec3(-5,0,-5));
    ddraw_point(vec3(-2,0,-2));
    ddraw_color(PURPLE);
    ddraw_sphere(vec3(-3,0,-3),1);
}

#endif
