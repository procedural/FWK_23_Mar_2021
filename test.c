// framework demo
// - rlyeh, public domain

#define FWK_C
#include "fwk.h"

int main() {
    // options
    bool  do_about = 0;
    float do_scale = 0.10f;
    bool  do_twosided = 1;
    bool  do_wireframe = 0;
    bool  do_debugdraw = 0;
    float do_gamepad_deadzone = 0.15f;
    vec2  do_gamepad_polarity = vec2(+1,+1);
    vec2  do_gamepad_sensitivity = vec2(0.1f,0.1f);
    vec2  do_mouse_polarity = vec2(+1,-1);
    vec2  do_mouse_sensitivity = vec2(0.2f,0.2f);
    bool  do_billboard_x = 0, do_billboard_y = 0, do_billboard_z = 0;

    // window (80% sized, MSAA x8 flag)
    window_create(80, WINDOW_MSAA8);
    window_title(__FILE__);

    // load all fx files
    for(const char **list = file_list("fx**.fs"); *list; list++) {
        fx_load(*list);
    }

    // scene loading
    #define SCENE(...) #__VA_ARGS__
    const char *my_scene = SCENE([
        {
            skybox: '3rd/3rd_assets/cubemaps/stardust/',
        },
        {
            position:[-5.0,-2.0,2.0],
            rotation: [180.0,-90.0,0.0],
            scale:0.20,
            //anchor/pivot:[],
            // vertex:'p3 t2',
            mesh:'3rd/3rd_assets/models/witch/witch.obj',
            texture:'3rd/3rd_assets/models/witch/witch_diffuse.tga.png',
            swapzy:true,
            flipuv:false,
        },
        {
            position:[-5.0,-2.0,2.0],
            rotation: [180.0,-90.0,0.0],
            scale:2.20,
            //anchor/pivot:[],
            // vertex:'p3 t2',
            mesh:'3rd/3rd_assets/models/witch/witch_object.obj',
            texture:'3rd/3rd_assets/models/witch/witch_object_diffuse.tga.png',
            swapzy:true,
            flipuv:false,
        },
    ]);
    int num_spawned = scene_merge(my_scene);
    object_t *obj1 = scene_index(0);
    object_t *obj2 = scene_index(1);

    // manual spawn & loading
    model_t m1 = model(file_find("3rd/3rd_assets/models/kgirl/kgirls01.fbx"), 0); //MODEL_NO_ANIMS);
    texture_t t1 = texture(file_find("3rd/3rd_assets/models/kgirl/g01_texture.png"), TEXTURE_RGB);
    object_t* obj3 = scene_spawn();
    object_model(obj3, m1);
    object_diffuse(obj3, t1);
    object_scale(obj3, vec3(3,3,3));
    object_move(obj3, vec3(-10,0,-10));
    object_pivot(obj3, vec3(180,0,0));

    // animated models
    model_t model4 = model(file_find("kgirls01.fbx"), 0);
    rotation44(model4.pivot, -180,0,0,1);

    // camera
    camera_t cam = camera();
    cam.speed = 0.2f;

    // audio (both clips & streams)
    audio_t voice = audio_clip(file_find("coin.wav"));
    audio_t stream = audio_stream(file_find("wrath_of_the_djinn.xm")); // "larry.mid"
    audio_play(voice, 0);
    audio_play(stream, 0);

    // demo loop
    while (window_swap())
    {
        // input
        if( input_down(KEY_ESC) ) break;
        if( input_down(KEY_F5) ) app_reload();
        if( input_down(KEY_W) && input_held(KEY_LCTRL) ) break;
        if( input_down(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1 );
        if( input_down(KEY_Z) ) window_screenshot(__FILE__ ".png");
        if( input_down(KEY_V) ) window_videorec(window_has_videorec() ? 0 : __FILE__ ".mpg");

        // vec2 filtered_lpad = input_filter_deadzone(input2(GAMEPAD_LPAD), do_gamepad_deadzone + 1e-3);
        // vec2 filtered_rpad = input_filter_deadzone(input2(GAMEPAD_RPAD), do_gamepad_deadzone + 1e-3);

        // fps camera
        bool active = ui_hover() ? false : input(MOUSE_L) || input(MOUSE_M) || input(MOUSE_R);
        window_cursor( !active );

        if( active ) cam.speed = clampf(cam.speed + input_diff(MOUSE_W) / 10, 0.05f, 5.0f);
        vec2 mouse = scale2(vec2(input_diff(MOUSE_X), -input_diff(MOUSE_Y)), 0.2f * active);
        vec3 wasdec = scale3(vec3(input(KEY_D)-input(KEY_A),input(KEY_E)-input(KEY_C),input(KEY_W)-input(KEY_S)), cam.speed);
        camera_move(&cam, wasdec.x,wasdec.y,wasdec.z);
        camera_fps(&cam, mouse.x,mouse.y);

        // queue model scale bounces
        float t = fmod(window_time(), 0.3) / 0.3;
        float s = 0.01f * ease_ping_pong(t, ease_in_cubic,ease_out_cubic);
        object_scale(obj1, vec3(0.20f - s,0.20f + s,0.20f - s));
        object_scale(obj2, vec3(0.20f - s,0.20f + s,0.20f - s));

        // queue model rotation
        object_rotate(obj3, vec3(0,1*window_time() * 20,0));
        object_billboard(obj3, (do_billboard_x << 2)|(do_billboard_y << 1)|(do_billboard_z << 0));

        // flush render scene (background objects: skybox)
        profile(Scene background) {
            scene_render(SCENE_BACKGROUND);
        }

        // queue debug drawcalls
        profile(Debugdraw) {
            ddraw_grid(0);
            ddraw_color(YELLOW);
            ddraw_text(vec3(+1,+1,-1), 0.04f, "(%f,%f,%f)", cam.position.x,cam.position.y,cam.position.z);
            if(do_debugdraw) ddraw_demo(); // showcase many debugdraw shapes
            ddraw_color(YELLOW);
        }

        profile(Editor) {
            editor();
        }

        // apply post-fxs from here
        fx_begin();

        // render scene (foreground objects) with post-effects
        profile(Scene foreground) {
            int scene_flags = 0;
            scene_flags |= do_wireframe ? SCENE_WIREFRAME : 0;
            scene_flags |= do_twosided ? 0 : SCENE_CULLFACE;
            scene_render(SCENE_FOREGROUND | scene_flags);
        }

        if(!window_has_pause())
        profile(Skeletal update) {
            float delta = (window_delta()*30); // 30fps anim
            obj3->model.curframe = model_animate(obj3->model, obj3->model.curframe + delta);
            ddraw_text(add3(obj3->pos,vec3(0,5,0)), 0.05, "Frame: %.1f", obj3->model.curframe);
            model4.curframe = model_animate(model4, model4.curframe + delta);
        }

        profile(Skeletal render) {
            {
                float scale = 3.00; // 0.025;
                mat44 M; copy44(M, model4.pivot); translate44(M,-10,-10,0); scale44(M, scale,scale,-scale);
                float mvp[16]; multiply44x3(mvp, cam.proj, cam.view, M);
                model_render(model4, mvp);
            }
        }

        // post-fxs end here
        fx_end();

        // queue ui
        if( ui_begin("App", 0)) {
            if(ui_bool("Show debugdraw demo", &do_debugdraw)) {}
            if(ui_separator()) {}
            if(ui_slider("Gamepad deadzone", &do_gamepad_deadzone)) {}
            if(ui_float2("Gamepad polarity", &do_gamepad_polarity.x)) {}
            if(ui_float2("Gamepad sensitivity", &do_gamepad_sensitivity.x)) {}
            if(ui_separator()) {}
            if(ui_float2("Mouse polarity", &do_mouse_polarity.x)) {}
            if(ui_float2("Mouse sensitivity", &do_mouse_sensitivity.x)) {}
            if(ui_separator()) {}
            if(ui_button("About...")) { do_about = 1; audio_play(voice, 0); }
            if(ui_dialog("About", __FILE__ "\n" __DATE__ "\n" "Public Domain.", 0, &do_about)) {}
            ui_end();
        }
        if( ui_begin("Camera", 0)) {
            if( ui_float("Speed", &cam.speed) ) {}
            if( ui_float3("Position", &cam.position.x) ) {}
            ui_end();
        }
        if( ui_begin("Scene", 0)) {
            if(ui_toggle("Billboard X", &do_billboard_x)) {}
            if(ui_toggle("Billboard Y", &do_billboard_y)) {}
            if(ui_toggle("Billboard Z", &do_billboard_z)) {}
            if(ui_separator()) {}
            if(ui_bool("Wireframe", &do_wireframe)) {}
            if(ui_bool("Two sided", &do_twosided)) {}
            ui_end();
        }
        if( ui_begin("Model", 0)) {
            ui_end();
        }
        if( ui_begin("FX", 0) ) {
            for( int i = 0; i < 64; ++i ) {
                char *name = fx_name(i); if( !name ) break;
                bool b = fx_enabled(i);
                if( ui_bool(name, &b) ) fx_enable(i, fx_enabled(i) ^ 1);
            }
            ui_end();
        }

        input_demo(); // show some keyboard/mouse/gamepad UI tabs
        ui_demo(); // show all UI widgets in a tab
    }

    // data tests (json5)
    char json5[] =
    "  /* json5 */ // comment\n"
    "  abc: 42.67, def: true, integer:0x100 \n"
    "  huge: 2.2239333e5, \n"
    "  hello: 'world /*comment in string*/ //again', \n"
    "  children : { a: 1, b: 2, c: 3 },\n"
    "  array: [+1,2,-3,4,5],    \n"
    "  invalids : [ nan, NaN, -nan, -NaN, inf, Infinity, -inf, -Infinity ],";
    if( data_push(json5) ) {
        assert( data_float("/abc") == 42.67 );
        assert( data_int("/def") == 1 );
        assert( data_int("/integer") == 0x100 );
        assert( data_float("/huge") > 2.22e5 );
        assert( strlen(data_string("/hello")) == 35 );
        assert( data_int("/children/a") == 1 );
        assert( data_int("/children.b") == 2 );
        assert( data_int("/children[c]") == 3 );
        assert( data_int("/array[%d]", 2) == -3 );
        assert( data_count("/invalids") == 8 );
        assert( isnan(data_float("/invalids[0]")) );
        assert( ~puts("json5 tests: ok") );
        data_pop();
    }

    // script test (lua)
    script_run( "-- Bye.lua\nio.write(\"script test: Bye world!, from \", _VERSION, \"\\n\")" );

    // network test (https)
    FILE *webfile = tmpfile();
    download(webfile, "https://google.com");
    printf("Network test: %d bytes downloaded from google.com\n", (int)ftell(webfile));
}
