import bpy

from bpy.types import Panel, Menu, Operator
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty)

# class LeafTextureSettings(bpy.types.PropertyGroup):
    # @classmethod
    # def register(cls):
        # bpy.types.Texture.leaf = PointerProperty(
            # name="Leaf Texture Settings",
            # description="Leaf texture settings",
            # type=cls,
        # )

    # @classmethod
    # def unregister(cls):
        # del bpy.types.Texture.leaf

class LeafTextureButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "texture"
    COMPAT_ENGINES = {"LEAF"}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine in cls.COMPAT_ENGINES
