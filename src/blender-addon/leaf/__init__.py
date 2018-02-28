bl_info = {
    "name": "Leaf",
    "category": "Render",
    "author": "wsmind"
}

if "bpy" in locals():
    import imp
    imp.reload(camera)
    imp.reload(cooking)
    imp.reload(export)
    imp.reload(image)
    imp.reload(light)
    imp.reload(material)
    imp.reload(preferences)
    imp.reload(render)
    imp.reload(texture)
else:
    from . import camera
    from . import cooking
    from . import export
    from . import image
    from . import light
    from . import material
    from . import preferences
    from . import render
    from . import texture

import bpy
import ctypes
import _ctypes
import os
import shutil
import io
import threading

from bpy.app.handlers import persistent

engine = None

class LeafRenderEngine(bpy.types.RenderEngine):
    bl_idname = "LEAF"
    bl_label = "Leaf"

    bl_use_preview = True

    def export(self, data, prefix=""):
        global engine

        with io.BytesIO() as f:
            if self.is_preview:
                export.export_data(f, data, prefix, False)
            else:
                export.export_data(f, data, prefix, not engine.full_data_send)
                engine.full_data_send = False

            data_bytes = f.getvalue()
            engine.dll.leaf_load_data(data_bytes, len(data_bytes))

    # viewport render
    def view_update(self, context):
        global engine
        engine.acquire()
        self.export(bpy.data, "")
        engine.release()

    def view_draw(self, context):
        global engine
        engine.acquire()
        vm = context.region_data.view_matrix.copy()
        vm.transpose()
        view_matrix = (ctypes.c_float * 16)(
            vm[0][0], vm[0][1], vm[0][2], vm[0][3],
            vm[1][0], vm[1][1], vm[1][2], vm[1][3],
            vm[2][0], vm[2][1], vm[2][2], vm[2][3],
            vm[3][0], vm[3][1], vm[3][2], vm[3][3]
        )

        pm = context.region_data.window_matrix.copy()
        pm.transpose()
        projection_matrix = (ctypes.c_float * 16)(
            pm[0][0], pm[0][1], pm[0][2], pm[0][3],
            pm[1][0], pm[1][1], pm[1][2], pm[1][3],
            pm[2][0], pm[2][1], pm[2][2], pm[2][3],
            pm[3][0], pm[3][1], pm[3][2], pm[3][3]
        )

        engine.dll.leaf_render_blender_viewport(context.region.width, context.region.height, view_matrix, projection_matrix)
        engine.release()

    def update(self, data, scene):
        global engine
        engine.acquire()
        self.export(data, "preview." if self.is_preview else "")
        engine.release()

    def render(self, scene):
        global engine
        engine.acquire()
        result = self.begin_result(0, 0, scene.render.resolution_x, scene.render.resolution_y)
        layer = result.layers[0].passes["Combined"]
        engine.dll.leaf_render_blender_frame((("preview." if self.is_preview else "") + scene.name).encode("utf-8"), ctypes.cast(layer.as_pointer(), ctypes.POINTER(ctypes.c_float)), ctypes.c_float(scene.frame_current))
        self.end_result(result)
        engine.release()

class EngineWrapper:
    def __init__(self):
        self.handle = None
        self.dll = None

        self.script_dir = os.path.dirname(__file__)
        self.dll_name = os.path.join(self.script_dir, "LeafEngine.dll")
        self.loaded_dll_name = os.path.join(bpy.app.tempdir, "LeafEngine-Loaded.dll")

        self.lock = threading.Lock()

    def load(self):
        # copy the dll, to allow hot reload after rebuild
        shutil.copy(self.dll_name, self.loaded_dll_name)

        # loading pattern from http://stackoverflow.com/questions/21770419/free-the-opened-ctypes-library-in-python
        self.dll = ctypes.CDLL(self.loaded_dll_name)

    def unload(self):
        _ctypes.FreeLibrary(self.dll._handle)
        del self.dll

        # remove the temp dll
        os.remove(self.loaded_dll_name)
    
    def acquire(self):
        self.lock.acquire()

    def release(self):
        self.lock.release()

compatible_panels = [
    bpy.types.TEXTURE_PT_context_texture,
    bpy.types.TEXTURE_PT_envmap,
    bpy.types.TEXTURE_PT_image,
    bpy.types.DATA_PT_context_mesh,
    bpy.types.DATA_PT_lens,
    bpy.types.DATA_PT_camera,
    bpy.types.DATA_PT_camera_display,
    bpy.types.DATA_PT_camera_dof,
    bpy.types.DATA_PT_camera_safe_areas,
    bpy.types.DATA_PT_lamp,
    bpy.types.DATA_PT_normals,
    bpy.types.MATERIAL_PT_preview,
    bpy.types.RENDER_PT_render,
    bpy.types.RENDER_PT_dimensions,
    bpy.types.RENDER_PT_output,
    bpy.types.WORLD_PT_world,
    bpy.types.WORLD_PT_mist
]

def register():
    global engine
    engine = EngineWrapper()
    render.engine = engine
    engine.load()

    # these dimensions define the maximum viewport size, actual viewports can be smaller
    # and will use only a subregion of it
    engine.dll.leaf_initialize(1920, 1080, True, None)
    
    # tag everything for reupload in engine
    engine.full_data_send = True
    
    cooking.cooker = cooking.Cooker()
    cooking.cooker.register_processor("image", cooking.ImageProcessor())

    # register callbacks
    bpy.app.handlers.frame_change_pre.append(frame_change_pre)
    bpy.app.handlers.load_post.append(load_post)

    bpy.utils.register_module(__name__)

    for panel in compatible_panels:
        panel.COMPAT_ENGINES.add("LEAF");

def unregister():
    # switch to another render engine if necessary
    if bpy.context.scene.render.engine == "LEAF":
        bpy.context.scene.render.engine = "BLENDER_RENDER"

    for panel in compatible_panels:
        panel.COMPAT_ENGINES.remove("LEAF");

    bpy.utils.unregister_module(__name__)

    cooking.cooker = None

    # unregister callbacks
    bpy.app.handlers.frame_change_pre.remove(frame_change_pre)
    bpy.app.handlers.load_post.remove(load_post)

    global engine
    engine.dll.leaf_shutdown()
    engine.unload()
    engine = None
    render.engine = None

@persistent
def frame_change_pre(scene):
    global engine
    engine.dll.leaf_update_animation(ctypes.c_float(scene.frame_current))

@persistent
def load_post(dummy):
    global engine
    engine.full_data_send = True
