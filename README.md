# F·W·K
Minimalist framework in plain C for quick 3D game development.

<!--## Gallery-->
![img](https://i.imgur.com/oywPKvL.png)

## Goals
- [x] ~C++~. C.
- [x] ~Fast~. Simple.
- [x] ~Modern~. Readable.
- [x] ~Full featured~. Small.
- [x] ~Royaltie fee~. Free and unlicensed.

## Roadmap ᕕ(ᐛ)ᕗ
- [x] Portable: Windows, ~~Linux and OSX.~~ MSVC, ~~GCC and clang.~~
- [x] Windowing, fullscreen, msaa, cursor handling.
- [x] UI: color3, color4, button, list, slider, checkbox, editbox...
- [x] Math: rand, ease, vec2, vec3, vec4, mat33, mat34, mat44, quat ~~and quat2. bezier, catmull~~.
- [x] Collisions: ray, line, plane, aabb, sphere, capsule, triangle, poly and frustum.
- [x] Input: keyboard, mouse and gamepads.
- [x] Render: Post-effects (SSAO,FXAA1/3,CRT,Contrast,Grain,Outline,Vignette,...) ~~and shadertoys~~.
- [x] Render: Cubemaps, panoramas and spherical harmonics.
- [x] Render: 2D Sprites, spritesheets, batching ~~and 2D Spines~~.
- [x] Render: 3D Debugdraw, batching and vectorial font.
- [x] Render: 3D Anims, skeletal anims and hardware skinning.
- [x] Models: IQM, GLTF/2, FBX, OBJ, DAE, BLEND, MD3, MD5, MS3D, SMD, X, 3DS, BVH, DXF, LWO.
- [x] Audios: WAV, MOD, XM, FLAC, OGG, MP1, MP3, MID ~~and SFXR~~.
- [x] Images: JPG, PNG, TGA, BMP, PSD, HDR, PIC, PNM ~~and GIF~~.
- [x] Textures: BCn 1/2/3/~~4/5/6/7, DDS, KTX, ETC~~.
- [x] Videos: MPG, ~~OGV and MP4~~.
- [x] Compressors: DEFLATE, LZMA, LZ4, ULZ, BALZ, BCM, CRUSH, LZW3, LZSS, PPP ~~and LZP1~~.
- [x] Local and virtual filesystems: ZIP, PAK, TAR and DIR.
- [x] Network: downloads (http/s and ftp) ~~and tcp/udp sockets~~.
- [x] Script: Lua, bindings ~~and FFI~~.
- [x] Integrated asset pipeline.
- [x] Profiler and stats.
- [x] Scene handling.
- [x] Disk cache.
- [x] Level data: JSON5 ~~and XML~~.
- [ ] Level objects: volumes, triggers, platforms, streaming.
- [ ] Level editor.

<!--
//
// done:
// [x] model loading, [x] transforms, [x] camera, [x] basic scene handling
// [x] skybox, [x] cubemaps, [x] harmonics,
// [x] compressed textures (bc1,bc2,bc3)
// [x] sprite sheet batching
// [x] billboards (@fixme: distortion)
// [x] postprocesses
// [x] skeletal anims
// [x] bounding boxes
// [x] ssao
//
// almost:
// [x] video textures (mpeg), [x] shadertoy textures,
// [ ] billboards
//
// roadmap:
// 1) lighting (per @procedural's request)
//    shadowing (vsm,pcf), lightmapping (demos), reflection probes
// 2) extending asset pipeline, file/vfs optimizations, fixing leaks, crashes and stability issues.
//    fwk_app: app state manager, options, modules, app icon, battery, orientation
//    fwk_input: cursor, mouse clip, mouse wrap,
// 3) lock API and release first stable version.
// 4) editor: load/save jsons, property editor for anything (remote osc server/client)
//    scene: scenegraph, obj naming, obj picking, obj bounds, obj collisions, obj/scene streaming
//    ecs: state load/save/diff/patch, ecs: messaging, ecs: filesystem,
// X) level
//    scripts: states, signals, h/fsm, coroutines, load/save context
//    ai: h/fsm for level objects and small enemies, planning GOAL, BTrees,
//    nav: navigation, pathfinding (jps.hh), a*, swarm/flocks,
//    physics: emitters: particles, lights, lightmaps, sound sources, triggers, etc
//    level: box triggers, start/end, spawn, streaming, checkpoints,
//    level: jump, shoots, platforms, collisions
//    level: 60s, 70s, 80s, 90s
// 5) PBR/IBL/materials (from Foxotron+sgorsten) + shading models
//    instancing, frustum culling, impostors, mesh lods,
//    renderbuckets
//    camera friction & projections (dimetric, isometric, ...)
//    compressed textures (soil2), animated textures (vids, shadertoys)
//    2d: billboards, spines, particles (tlfx2), pixel perfect zooming
//    3d-reverse-z {
//    fbo attach format D16_UNORM -> D32_SFLOAT
//    pipeline depth compare LEQUAL -> GEQUAL
//    pipeline depth clear 1.0 -> 0.0
//    proj matrix: float a = zfar / (zfar - znear); -> float a = -znear / (zfar - znear);
//    proj matrix: float b = (-znear * zfar) / (zfar - znear); -> float b = (znear * zfar) / (zfar - znear);
//    }
// 6) anims, I (playlist: forward/backwards/loop/rewind), II (blend/shapes), III (ik/bone), IV (graph/controller)
//    blend anims, animtracks+animevents, draw skeleton, additive anims,
//    fwk_data: quantization: ~~half, quant, microfloat~~.
//    anim; keyframes[] { frame+delay,frame+delay,... }, anim duration, anim flip
// 7) network replication & messaging
//    network: replication, dead reckoning, interpolation, extrapolation, bandwidth
//    network: messaging: un/reliable, fragmentation, priority, etc
//    network: topologies: bus, star, p2p, pubsub, etc
//    network: filesystem
// 8) vm/ecs core + engines: custom frontends & backends
//    vm: ram, workqueues, threading, priorities
//    service protocols: websocket bqqbarbhg/bq_websocket, https, handshake
//    databases, services, quotas, black/whitelists, etc

## To Merge (14)
- [ ] fwk.h
- [ ] fwk_audio.h
- [ ] fwk_editor.h + fwk_cooker.h
- [ ] fwk_file.h
- [ ] fwk_input.h
- [ ] fwk_math.h + fwk_collide.h
- [ ] fwk_network.h + fwk_socket.h
- [ ] fwk_render.h + fwk_renderdd.h + fwk_animate(->fwk_model).h
- [ ] fwk_scene.h + fwk_level.h + fwk_asset.h + fwk_data.h
- [ ] fwk_script.h
- [ ] fwk_system.h + fwk_ds.h
- [ ] fwk_ui.h
- [ ] fwk_video.h
- [ ] fwk_window.h

-->

## Build (msvc, x64)
```
c:\fwk> cl test.c && test
```
Note: Assimp.dll may need [this package installed](https://www.microsoft.com/en-us/download/confirmation.aspx?id=30679).

## License
The project is released into the public domain (as per [this notice](UNLICENSE)).<br/>
Also dual-licensed as MIT (No Attribution) for those countries where public domain is a concern (sigh).<br/>
Any contribution to this repository is implicitly subjected to the same release conditions aforementioned.

## Artwork
- [Goblin165cm](https://sketchfab.com/3d-models/halloween-little-witch-ccc023590bfb4789af9322864e42d1ab), for witch 3D model (CC BY 4.0).
- [Nanofactory](https://sketchfab.com/3d-models/kgirls01-d2f946f58a8040ae993cda70c97b302c), for kgirls01 3D model (CC BY-NC-ND 4.0).
- [Rxi](https://github.com/rxi/autobatch), for sprites & cats demo (MIT).

## Credits
- [Assimp authors](https://github.com/assimp/assimp), for assimp (BSD3).
- [Barerose](https://github.com/barerose), for swrap (CC0).
- [Camilla Löwy](https://github.com/elmindreda), for glfw3 (Zlib)
- [David Herberth](https://github.com/dav1dde/), for glad generated code (PD).
- [David Reid](https://github.com/mackron), for miniaudio (PD).
- [Dominik Madarász](https://github.com/zaklaus), for json5 parser (PD).
- [Dominic Szablewski](https://github.com/phoboslab/pl_mpeg), for pl_mpeg (MIT).
- [Joonas Pihlajamaa](https://github.com/jokkebk/JUnzip), for JUnzip library (PD).
- [Jon Olick](https://www.jonolick.com/), for jo_mp1 and jo_mpeg (PD).
- [Joshua Reisenauer](https://github.com/kd7tck), for jar_mod and jar_xm (PD).
- [Lee Salzman](https://github.com/lsalzman/iqm/tree/5882b8c32fa622eba3861a621bb715d693573420/demo), for IQM spec & player (PD).
- [Lua authors](https://www.lua.org/), for Lua language (MIT).
- [Mattias Gustavsson](https://github.com/mattiasgustavsson/libs), for thread.h (PD).
- [Micha Mettke](https://github.com/vurtun/nuklear), for nuklear (PD).
- [Rich Geldreich](https://github.com/richgel999/miniz), for miniz (PD).
- [Sean Barrett](https://github.com/nothings), for stb_image, stb_image_write and stb_vorbis (PD).
- [Sebastian Steinhauer](https://github.com/kieselsteini), for sts_mixer (PD).
- [Tomas Pettersson](http://www.drpetter.se/), for sfxr (PD).
- Special thanks to @ands (PD), @barerose (CC0), @datenwolf (WTFPL2), @evanw (CC0), @glampert (PD), @krig (CC0), @sgorsten (Unlicense), @vurtun (PD) for their math libraries.

## Links
<a href="https://github.com/r-lyeh/FWK/issues"><img alt="Issues" src="https://img.shields.io/github/issues-raw/r-lyeh/FWK.svg"/></a>
<a href="https://discord.gg/vu6Vt9d"><img alt="Discord" src="https://img.shields.io/badge/chat-FWK%20lounge-738bd7.svg?logo=discord"/></a>

Looking for alternatives?
[amulet](https://github.com/ianmaclarty/amulet),
[aroma](https://github.com/leafo/aroma/),
[astera](https://github.com/tek256/astera),
[blendelf](https://github.com/jesterKing/BlendELF),
[bullordengine](https://github.com/MarilynDafa/Bulllord-Engine),
[candle](https://github.com/EvilPudding/candle),
[cave](https://github.com/kieselsteini/cave),
[chickpea](https://github.com/ivansafrin/chickpea),
[corange](https://github.com/orangeduck/Corange),
[ejoy2d](https://github.com/ejoy/ejoy2d),
[exengine](https://github.com/exezin/exengine),
[gunslinger](https://github.com/MrFrenik/gunslinger),
[hate](https://github.com/excessive/hate),
[island](https://github.com/island-org/island),
[juno](https://github.com/rxi/juno),
[l](https://github.com/Lyatus/L),
[lgf](https://github.com/Planimeter/lgf),
[limbus](https://github.com/redien/limbus),
[love](https://github.com/love2d/love/),
[lovr](https://github.com/bjornbytes/lovr),
[mini3d](https://github.com/mini3d/mini3d),
[mintaro](https://github.com/mackron/mintaro),
[mio](https://github.com/ccxvii/mio),
[opensource](https://github.com/w23/OpenSource),
[pez](https://github.com/prideout/pez),
[pixie](https://github.com/mattiasgustavsson/pixie),
[punity](https://github.com/martincohen/Punity),
[ricotech](https://github.com/dbechrd/RicoTech),
[rizz](https://github.com/septag/rizz),
[tigr](https://github.com/erkkah/tigr),