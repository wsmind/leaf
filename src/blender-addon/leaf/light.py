import bpy

from bpy.types import Panel, Menu, Operator, UIList
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty)

class LeafLightButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "lamp"
    COMPAT_ENGINES = {"LEAF"}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine in cls.COMPAT_ENGINES

class LeafLight_PT_spot(LeafLightButtonsPanel, Panel):
    bl_label = "Spot Shape"
    bl_context = "data"

    @classmethod
    def poll(cls, context):
        lamp = context.lamp
        return (lamp and lamp.type == 'SPOT') and LeafLightButtonsPanel.poll(context)

    def draw(self, context):
        layout = self.layout

        lamp = context.lamp

        split = layout.split()

        col = split.column()
        sub = col.column()
        sub.prop(lamp, "spot_size", text="Size")
        sub.prop(lamp, "spot_blend", text="Blend", slider=True)

        col = split.column()
        col.prop(lamp, "show_cone")
