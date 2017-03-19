import bpy

from bpy.types import Panel, Menu, Operator, UIList
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty)

class LeafImageSettings(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        bpy.types.Image.leaf = PointerProperty(
            name="Leaf Image Settings",
            description="Leaf image settings",
            type=cls,
        )
        cls.is_normal_map = BoolProperty(
            name="Normal Map",
            description="This image contains normal data and should be treated differently from color information",
            default=False,
        )

    @classmethod
    def unregister(cls):
        del bpy.types.Image.leaf

class LeafImageButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "texture"
    COMPAT_ENGINES = {"LEAF"}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        tex = context.texture
        return tex and (tex.type == "IMAGE") and (rd.engine in cls.COMPAT_ENGINES)

class LeafImage_PT_settings(LeafImageButtonsPanel, Panel):
    bl_label = "Leaf"

    def draw(self, context):
        layout = self.layout
        texture = context.texture
        leaf_image = texture.image.leaf

        layout.prop(leaf_image, "is_normal_map")
