import os
import shutil

import bpy
from bpy.types import Panel, Menu, Operator
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty)

class LEAF_OT_export(Operator):
    bl_idname = "leaf.export"
    bl_label = "Export Demo"

    def execute(self, context):
        rd = context.scene.render
        
        os.makedirs(rd.filepath, exist_ok=True)

        script_dir = os.path.dirname(__file__)

        runtime_files = ["LeafEngine.dll", "LeafRunner.exe"]
        for f in runtime_files:
            source = os.path.join(script_dir, f)
            destination = os.path.join(rd.filepath, f)
            shutil.copy(source, destination)

        return {"FINISHED"}

class LeafRenderButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "render"
    COMPAT_ENGINES = {"LEAF"}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine in cls.COMPAT_ENGINES

class LeafRender_PT_export(LeafRenderButtonsPanel, Panel):
    bl_label = "Export"

    def draw(self, context):
        layout = self.layout
        rd = context.scene.render

        layout.prop(rd, "filepath", text="")
        layout.operator("leaf.export", icon='FILE_TICK')
