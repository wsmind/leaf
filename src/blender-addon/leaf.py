import bpy

bl_info = {
    "name": "Leaf",
    "category": "Render",
    "author": "wsmind"
}

class LeafRenderEngine(bpy.types.RenderEngine):
    bl_idname = "leaf_renderer"
    bl_label = "Leaf"
    
    def __init__(self):
        print("constructed")

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

def register():
    bpy.utils.register_module(__name__)

def unregister():
    bpy.utils.unregister_module(__name__)
