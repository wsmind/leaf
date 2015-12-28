import bpy
import ctypes
import _ctypes
import os
import shutil

bl_info = {
    "name": "Leaf",
    "category": "Render",
    "author": "wsmind"
}

engine = None

class LeafRenderEngine(bpy.types.RenderEngine):
    bl_idname = "leaf_renderer"
    bl_label = "Leaf"

    # viewport render
    def view_update(self, context):
        print("view_update")

        if bpy.data.objects.is_updated:
            for obj in bpy.data.objects:
                if obj.is_updated:
                    print("updated: " + obj.name)

    def view_draw(self, context):
        print("view_render!")
        print(context.region.x, context.region.y, context.region.width, context.region.height)
        print(context.space_data, context.region_data)

        global engine
        engine.dll.plop()

        # bgl.glClearColor(1.0, 1.0, 0.0, 1.0)
        # bgl.glClear(bgl.GL_COLOR_BUFFER_BIT)

        # pixelCount = context.region.width * context.region.height

        # if not hasattr(self, "view_buffer") or context.region.width != self.view_buffer["width"] or context.region.height != self.view_buffer["height"]:
        #     print("new buffer")
        #     self.view_buffer = {
        #         "buf": bgl.Buffer(bgl.GL_BYTE, pixelCount * 3),
        #         "width": context.region.width,
        #         "height": context.region.height
        #     }

        # bgl.glRasterPos2i(0, 0)
        # bgl.glDrawPixels(self.view_buffer["width"], self.view_buffer["height"], bgl.GL_RGB, bgl.GL_UNSIGNED_BYTE, self.view_buffer["buf"])

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
        #self.handle = ctypes.windll.kernel32.LoadLibraryA(self.loaded_dll_name)
        #self.dll = ctypes.CDLL(None, handle=self.handle)
        self.dll = ctypes.CDLL(self.loaded_dll_name)

        # api
        self.dll.plop.restype = None

    def unload(self):
        _ctypes.FreeLibrary(self.dll._handle)
        del self.dll

        # remove the temp dll
        os.remove(self.loaded_dll_name)

def register():
    global engine
    engine = EngineWrapper()
    engine.load()

    bpy.utils.register_module(__name__)

def unregister():
    bpy.utils.unregister_module(__name__)

    global engine
    engine.unload()
    engine = None
