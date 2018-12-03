import bpy

from bpy.types import Panel, Menu, Operator, UIList
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty)

class LeafObjectSettings(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        bpy.types.Object.leaf = PointerProperty(
            name="Leaf Object Settings",
            description="Leaf object settings",
            type=cls,
        )
        cls.is_distance_field = BoolProperty(
            name="Distance Field",
            description="This object is rendered as a distance field",
            default=False,
        )
        cls.code = PointerProperty(
            name="Code",
            description="Implementation for the SDF function",
            type=bpy.types.Text,
        )

    @classmethod
    def unregister(cls):
        del bpy.types.Object.leaf

class LeafObjectButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

class LeafObject_PT_settings(LeafObjectButtonsPanel, Panel):
    bl_label = "Leaf Settings"

    def draw(self, context):
        layout = self.layout

        obj = context.object
        leaf_obj = obj.leaf

        layout.prop(leaf_obj, "is_distance_field")
        if leaf_obj.is_distance_field:
            layout.prop(leaf_obj, "code")
