import bpy

from bpy.types import Panel, Menu, Operator

class LeafMaterialButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "material"
    COMPAT_ENGINES = {"LEAF"}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine in cls.COMPAT_ENGINES

class LeafMaterial_PT_surface(LeafMaterialButtonsPanel, Panel):
    bl_label = "Surface"

    @classmethod
    def poll(cls, context):
        return context.material and LeafMaterialButtonsPanel.poll(context)

    def draw(self, context):
        layout = self.layout
        mat = context.material

        layout.prop(mat, "diffuse_color")
