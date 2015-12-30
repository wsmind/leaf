import bpy

from bpy.types import Panel, Menu, Operator

class LeafButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    COMPAT_ENGINES = {"LEAF"}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine in cls.COMPAT_ENGINES

class LeafMaterial_PT_surface(LeafButtonsPanel, Panel):
    bl_label = "Surface"
    bl_context = "material"

    @classmethod
    def poll(cls, context):
        return context.material and LeafButtonsPanel.poll(context)

    def draw(self, context):
        layout = self.layout

        mat = context.material
        layout.prop(mat, "diffuse_color")

def register():
    pass

def unregister():
    pass
