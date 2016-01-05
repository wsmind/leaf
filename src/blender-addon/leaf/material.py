import bpy

from bpy.types import Panel, Menu, Operator
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty)

class LeafMaterialSettings(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        bpy.types.Material.leaf = PointerProperty(
            name="Leaf Material Settings",
            description="Leaf material settings",
            type=cls,
        )
        cls.metalness = FloatProperty(
            name="Metalness",
            description="0: dielectric, 1: metal",
            min=0.0, max=1.0,
            default=0.0,
        )
        cls.roughness = FloatProperty(
            name="Roughness",
            description="0: smooth, 1: rough",
            min=0.0, max=1.0,
            default=0.5,
        )

    @classmethod
    def unregister(cls):
        del bpy.types.Material.leaf

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
        lmat = context.material.leaf

        layout.prop(mat, "diffuse_color", text="Albedo")
        layout.prop(lmat, "metalness")
        layout.prop(lmat, "roughness")
