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
else:
    from . import export
    from . import material
    from . import render

import bpy
import ctypes
import _ctypes
import os
import shutil
import json

engine = None

class LeafRenderEngine(bpy.types.RenderEngine):
    bl_idname = "LEAF"
    bl_label = "Leaf"

    # viewport render
    def view_update(self, context):
        print("view_update")
        data = export.export_data()
        print(json.dumps(data))

    def view_draw(self, context):
        print("view_render!")
        print(context.region.x, context.region.y, context.region.width, context.region.height)
        print(context.space_data, context.region_data)

        global engine
        engine.dll.leaf_render_blender_viewport(context.region.width, context.region.height)

class EngineWrapper:
    def __init__(self):
        self.handle = None
        self.dll = None

        self.script_dir = os.path.dirname(__file__)
        self.dll_name = os.path.join(self.script_dir, "LeafEngine.dll")
        self.loaded_dll_name = os.path.join(self.script_dir, "LeafEngine-Loaded.dll")

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

def register():
    global engine
    engine = EngineWrapper()
    engine.load()

    # these dimensions define the maximum viewport size, actual viewports can be smaller
    # and will use only a subregion of it
    engine.dll.leaf_initialize(1920, 1080, True)

    bpy.utils.register_module(__name__)

def unregister():
    bpy.utils.unregister_module(__name__)

    global engine
    engine.dll.leaf_shutdown()
    engine.unload()
    engine = None
