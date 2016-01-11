import os
import shutil
import json

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
        lrd = context.scene.leaf

        os.makedirs(rd.filepath, exist_ok=True)

        # export data
        from . import export
        data, blobs = export.export_data()
        data_string = json.dumps(data)
        with open(os.path.join(rd.filepath, "data.json"), "wb") as f:
            f.write(data_string.encode('utf-8'))

        # copy engine files in the output folder
        script_dir = os.path.dirname(__file__)
        try:
            runtime_files = ["LeafEngine.dll", "LeafRunner.exe"]
            for f in runtime_files:
                source = os.path.join(script_dir, f)
                destination = os.path.join(rd.filepath, f)
                shutil.copy(source, destination)

            # run if selected
            if lrd.run_after_export:
                import subprocess
                engineExe = os.path.join(rd.filepath, "LeafRunner.exe")
                subprocess.Popen([engineExe], cwd=rd.filepath)
        except PermissionError:
            self.report({"ERROR"}, "Failed to copy engine files. Make sure the demo is not running while exporting.")

        return {"FINISHED"}

class LeafRenderSettings(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        bpy.types.Scene.leaf = PointerProperty(
            name="Leaf Render Settings",
            description="Leaf render settings",
            type=cls,
        )
        cls.run_after_export = BoolProperty(
            name="Run after export",
            description="Start the exported demo when export finishes",
            default=True,
        )

    @classmethod
    def unregister(cls):
        del bpy.types.Scene.leaf

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
        lrd = context.scene.leaf

        layout.prop(rd, "filepath", text="")

        row = layout.row(align=True)
        row.operator("leaf.export", text="Export", icon='FILE_TICK')
        row.prop(lrd, "run_after_export", text="Start Demo")
