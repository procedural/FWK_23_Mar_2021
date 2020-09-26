// in-game editor
// - rlyeh, public domain.
//
// when editor is on:
// - [ ] ESC, toggles editor on/off
// - [ ] ESC (long press), sends escape key to game
// - [ ] F5, toggles game pause on/off
// - [ ] F5 (long press), restarts game
// - [ ] TAB, see console/log
// - [ ] LMB, select item, display hierarchy @ left sidebar, status @ right sidebar. right click to unselect
// - [ ] RMB, on selected item, display contextual menu: edit, edit script...
// - [ ] RMB, camera move if no gameobj is selected + WASDEC + wheel
// - [ ] SPC, on selected item, cycle transform: locate, rotate, scale
// - [ ] CTRL+S save, CTRL+L load restore point
// - [ ] IMPR save capture bug/info
//
// roadmap
// - [x] ray/object picking
// - [ ] introspect properties on selected object
// - [ ] add/rem entities, add/rem components, add/rem/pause/resume systems
// - [ ] widgets for (bool,str,flt,int,vec2,vec3,vec4,range,mesh,texture,audio,any other asset,combo of anything)
// - [ ] osc server for properties and editor behavior
// - [ ] multiple selections/select all
// - [ ] undo/redo
// - [ ] cut/copy/paste (ctrl-c to serialize)
// - [ ] menu: open, save, save as, save all, reload

#ifndef EDITOR_H
#define EDITOR_H

void editor();

#endif // EDITOR_H

#ifdef EDITOR_C
#pragma once

// editing:
// nope > functions: add/rem property

static int editor_mode = 1;
static int editor_selected = -1; // object in scene selected
static vec2 editor_mouse; // 2d coord for ray/picking
static bool editor_slow_sim = 0; // 30fps?

void editor_update() {
    scene_t *scene = scene_get_active();
    camera_t *camera = camera_get_active();

    // input: mouse
    if( input_down(MOUSE_L) && !ui_hover() ) {
        editor_mouse = vec2(input(MOUSE_X), input(MOUSE_Y));
    }
    if( input_click(MOUSE_L, 500) && !ui_hover() ) { // pick entity
        // unproject 2d coord as 3d coord
        vec3 out, xyd = vec3(editor_mouse.x,window_height()-editor_mouse.y,1); // usually x:mouse_x,y:window_height()-mouse_y,d:0=znear/1=zfar
        mat44 mvp, model; identity44(model); multiply44x3(mvp, camera->proj, camera->view, model);
        bool ok = unproject44(&out, xyd, vec4(0,0,window_width(),window_height()), mvp);

        vec3 from = camera_get_active()->position, to = out;
        ray r = ray(from, to);
        //ddraw_line(from, to); // visualize ray

        int found = -1, count = scene_count();
        for( int i = 0; i < count; ++i ) {
            object_t *obj = scene_index(i);
            // bring aabb box to object position
            aabb box = model_aabb(obj->model, obj->transform); //add3(obj->pos, obj->bounds.min), add3(obj->pos, obj->bounds.max));
            // test ray hit
            if( ray_hit_aabb(r, box) ) {
                editor_selected = i;
                break;
            }
        }
    }

    object_t *obj = 0;
    if( editor_selected >= 0 ) {
        obj = scene_index(editor_selected);
        // bring aabb box to object position
        aabb box = model_aabb(obj->model, obj->transform); // aabb box = aabb(add3(obj->pos, obj->bounds.min), add3(obj->pos, obj->bounds.max));
        ddraw_aabb(box.min, box.max);
    }

    if( ui_begin("Editor", 0) ) {
        bool x;
        ui_float2("mouse (2d pick)", &editor_mouse.x);
        if( ui_bool("breakpoint", (x = 0, &x)) ) breakpoint("editor breakpoint");
        if( ui_bool("debugger", (x = has_debugger(), &x))) {}
        if( ui_bool("fullscreen", (x = window_has_fullscreen(), &x)) ) window_fullscreen( x );
        if( ui_bool("pause", (x = window_has_pause(), &x)) ) window_pause( x );
        if( ui_bool("30 fps", (x = editor_slow_sim, &x) ) ) editor_slow_sim ^= 1;
        ui_separator();
        if( editor_selected >= 0 ) {
            ui_label(stringf("[%p]", obj));
            if(ui_float3("position", &obj->pos.x)) object_teleport(obj, obj->pos);
            if(ui_float3("rotation", &obj->euler.x)) object_rotate(obj, obj->euler);
            if(ui_float3("scale", &obj->sca.x)) object_scale(obj, obj->sca);
        }
        ui_end();
    }

    if( editor_slow_sim ) sleep_ms(18);
}

void editor() {
    // input: keyboard.
#if 0
    if( input_up(KEY_TAB) ) editor_mode ^= 1; // cycle editor mode
    if( input_up(KEY_F1) )  window_pause( window_has_pause() ^ 1 );
    if( input_up(KEY_F5) )  app_reload();
    if( input_up(KEY_F11) ) window_fullscreen( window_has_fullscreen() ^ 1);
    if( input_up(KEY_F12) ) screenshot("_screenshot.png", 3, true);
#endif
    // @fixme: send keys to game
    // if( input_repeat(KEY_TAB, 300)) {}
    // if( input_repeat(KEY_F1, 300)) {}
    // etc...

    if( editor_mode ) editor_update();
}

#endif // EDITOR_C
