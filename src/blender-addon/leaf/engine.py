import bpy
import ctypes
import _ctypes
import os
import shutil

engine = None

class LeafRenderEngine(bpy.types.RenderEngine):
    bl_idname = "LEAF"
    bl_label = "Leaf"

    # viewport render
    def view_update(self, context):
        print("view_update")

        if bpy.data.objects.is_updated:
            for obj in bpy.data.objects:
                if obj.is_updated:
                    print("updated: " + obj.name)

        for mtl in bpy.data.materials:
            if mtl.is_updated:
                print("updated: " + mtl.name)

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

    bpy.utils.register_class(LeafRenderEngine)

    # declare compatibility of this render engine with the material panel
    from bl_ui import properties_material
    if not "LEAF" in properties_material.MATERIAL_PT_context_material.COMPAT_ENGINES:
        properties_material.MATERIAL_PT_context_material.COMPAT_ENGINES.add('LEAF')
    del properties_material

def unregister():
    bpy.utils.unregister_class(LeafRenderEngine)

    global engine
    engine.dll.leaf_shutdown()
    engine.unload()
    engine = None
