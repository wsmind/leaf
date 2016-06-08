bl_info = {
    "name": "Leaf",
    "category": "Render",
    "author": "wsmind"
}

if "bpy" in locals():
    import imp
    imp.reload(export)
    imp.reload(material)
    imp.reload(render)
    imp.reload(texture)
else:
    from . import export
    from . import material
    from . import render
    from . import texture

import bpy
import ctypes
import _ctypes
import os
import shutil
import json

from bpy.app.handlers import persistent

engine = None

class LeafRenderEngine(bpy.types.RenderEngine):
    bl_idname = "LEAF"
    bl_label = "Leaf"

    # viewport render
    def view_update(self, context):
        global engine
        data, blobs = export.export_data(not engine.full_data_send)
        engine.full_data_send = False

        for name, buffer in blobs.items():
            print("registering blob " + name)
            engine.dll.leaf_register_blob(name.encode('utf-8'), buffer)

        engine.blobs.update(blobs)

        data_string = json.dumps(data)
        engine.dll.leaf_load_data(data_string.encode('utf-8'))

    def view_draw(self, context):
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

        global engine
        engine.dll.leaf_render_blender_viewport(context.region.width, context.region.height, view_matrix, projection_matrix)

class EngineWrapper:
    def __init__(self):
        self.handle = None
        self.dll = None

        self.script_dir = os.path.dirname(__file__)
        self.dll_name = os.path.join(self.script_dir, "LeafEngine.dll")
        self.loaded_dll_name = os.path.join(self.script_dir, "LeafEngine-Loaded.dll")

        self.blobs = {}

    def load(self):
        # copy the dll, to allow how reload after rebuild
        shutil.copy(self.dll_name, self.loaded_dll_name)

        # loading pattern from http://stackoverflow.com/questions/21770419/free-the-opened-ctypes-library-in-python
        self.dll = ctypes.CDLL(self.loaded_dll_name)

    def unload(self):
        _ctypes.FreeLibrary(self.dll._handle)
        del self.dll

        # remove the temp dll
        os.remove(self.loaded_dll_name)

        # unload all blobs stored on python side
        self.blobs = {}

compatible_panels = [
    bpy.types.TEXTURE_PT_image,
    bpy.types.DATA_PT_context_mesh
]

def register():
    global engine
    engine = EngineWrapper()
    engine.load()

    # these dimensions define the maximum viewport size, actual viewports can be smaller
    # and will use only a subregion of it
    engine.dll.leaf_initialize(1920, 1080, True)
    
    # tag everything for reupload in engine
    engine.full_data_send = True

    # register callbacks
    bpy.app.handlers.frame_change_pre.append(frame_change_pre)

    bpy.utils.register_module(__name__)

    for panel in compatible_panels:
        panel.COMPAT_ENGINES.add("LEAF");

def unregister():
    for panel in compatible_panels:
        panel.COMPAT_ENGINES.remove("LEAF");

    bpy.utils.unregister_module(__name__)

    # unregister callbacks
    bpy.app.handlers.frame_change_pre.remove(frame_change_pre)

    global engine
    engine.dll.leaf_shutdown()
    engine.unload()
    engine = None

@persistent
def frame_change_pre(scene):
    global engine
    engine.dll.leaf_update_animation(ctypes.c_float(scene.frame_current))
