import bpy

from bpy.types import Panel, Menu, Operator
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty,
                       FloatVectorProperty)

class LeafCameraSettings(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        bpy.types.Camera.leaf = PointerProperty(
            name="Leaf Camera Settings",
            description="Leaf camera settings",
            type=cls,
        )
        cls.shutter_speed = FloatProperty(
            name="Shutter speed",
            min=-0.001, max=10.0,
            default=0.01
        )

    @classmethod
    def unregister(cls):
        del bpy.types.Camera.leaf

class LeafCameraButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "data"
    COMPAT_ENGINES = {"LEAF"}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return context.camera and (rd.engine in cls.COMPAT_ENGINES)

class LeafCamera_PT_shutter(LeafCameraButtonsPanel, Panel):
    bl_label = "Shutter"

    @classmethod
    def poll(cls, context):
        return context.camera and LeafCameraButtonsPanel.poll(context)

    def draw(self, context):
        layout = self.layout
        cam = context.camera
        lcam = context.camera.leaf

        layout.prop(lcam, "shutter_speed")
