import os
import shutil
import subprocess

import bpy
from bpy.types import Panel, Menu, Operator
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       StringProperty,
                       PointerProperty)

engine = None

class LEAF_OT_export(Operator):
    bl_idname = "leaf.export"
    bl_label = "Export Demo"

    def execute(self, context):
        rd = context.scene.render
        lrd = context.scene.leaf

        os.makedirs(rd.filepath, exist_ok=True)

        script_dir = os.path.dirname(__file__)
        runtime_files = ["LeafEngine.dll", "LeafRunner.exe"]

        # export data
        from . import export
        with open(os.path.join(rd.filepath, "data.bin"), "wb") as f:
            export.export_data(f, bpy.data, "")

        # export shaders
        args = [
            os.path.join(script_dir, "LeafRunner.exe"),
            "--export=" + rd.filepath
        ]
        subprocess.Popen(args, cwd=script_dir).wait()

        # copy engine files in the output folder
        try:
            for f in runtime_files:
                source = os.path.join(script_dir, f)
                destination = os.path.join(rd.filepath, f)
                shutil.copy(source, destination)
        except PermissionError:
            self.report({"ERROR"}, "Failed to copy engine files. Make sure the demo is not running while exporting.")

        # run if selected
        if lrd.run_after_export:
            args = [
                os.path.join(rd.filepath, "LeafRunner.exe"),
                "--start-frame=" + str(lrd.run_start_frame)
            ]
            if lrd.run_profile:
                args.append("--profile=profile.json")
            subprocess.Popen(args, cwd=rd.filepath)

        return {"FINISHED"}

class LEAF_OT_refresh(Operator):
    bl_idname = "leaf.refresh"
    bl_label = "Refresh"

    def execute(self, context):
        global engine

        # tag everything for reupload in engine
        engine.full_data_send = True

        return {"FINISHED"}

class LEAF_OT_setup_audio(Operator):
    bl_idname = "leaf.setup_audio"
    bl_label = "Setup Audio"

    def execute(self, context):
        scene = context.scene
        
        scene.frame_start = 0
        scene.frame_current = 0 if scene.frame_current == 1 else scene.frame_current
        scene.render.fps = 60
        scene.sync_mode = "AUDIO_SYNC"
        scene.use_frame_drop = True

        scene.timeline_markers.clear()
        for beat in range(500):
            frame = beat * 60.0 * scene.render.fps / scene.leaf.audio_bpm
            scene.timeline_markers.new("%d.%d" % (int(beat / 4) + 1, (beat % 4) + 1), frame)
        
        if scene.leaf.audio_file != "":
            if not scene.sequence_editor:
                scene.sequence_editor_create()

            soundstrip = scene.sequence_editor.sequences.new_sound("music", scene.leaf.audio_file, 0, 0)
            scene.frame_end = soundstrip.frame_final_end

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
        cls.run_start_frame = IntProperty(
            name="Start frame",
            description="Frame on which the demo starts when previewing export",
            default=0,
            min=0
        )
        cls.run_profile = BoolProperty(
            name="Profile performance",
            description="Save a detailed performance analysis for further inspection",
            default=False,
        )
        cls.bloom_threshold = FloatProperty(
            name="Bloom threshold",
            min=0.0, max=100.0,
            default=1.0
        )
        cls.bloom_intensity = FloatProperty(
            name="Bloom intensity",
            min=0.0, max=2.0,
            default=1.0
        )
        cls.bloom_size = IntProperty(
            name="Bloom size",
            min=2, max=8,
            default=4
        )
        cls.bloom_debug = BoolProperty(
            name="Bloom debug",
            default=False
        )
        cls.pixellate_divider = FloatProperty(
            name="Pixellate Divider",
            min=0.0, max=1.0,
            default=0.0
        )
        cls.vignette_size = FloatProperty(
            name="Vignette Size",
            default=1.0
        )
        cls.vignette_power = FloatProperty(
            name="Vignette Power",
            default=1.6
        )
        cls.abberation_strength = FloatProperty(
            name="Chromatic Abberation Strength",
            default=0.1
        )
        cls.scanline_strength = FloatProperty(
            name="Scanline Strength",
            default=0.0
        )
        cls.scanline_frequency = FloatProperty(
            name="Scanline Frequency",
            default=20.0
        )
        cls.scanline_offset = FloatProperty(
            name="Scanline Offset",
            default=0.0
        )
        cls.audio_file = StringProperty(
            name="Audio File",
            subtype='FILE_PATH',
            default=""
        )
        cls.audio_bpm = FloatProperty(
            name="Audio BPM",
            default=120.0
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

        row = layout.row(align=True)
        row.prop(lrd, "run_start_frame", text="Start Frame")

        row = layout.row(align=True)
        row.prop(lrd, "run_profile", text="Profile Performance")

        row = layout.row(align=True)
        row.operator("leaf.refresh", text="Refresh")

class LeafRender_PT_bloom(LeafRenderButtonsPanel, Panel):
    bl_label = "Bloom"

    def draw(self, context):
        layout = self.layout
        rd = context.scene.render
        lrd = context.scene.leaf

        row = layout.row()
        row.prop(lrd, "bloom_threshold", text="Threshold")
        row.prop(lrd, "bloom_debug", text="Debug")

        layout.prop(lrd, "bloom_intensity", text="Intensity")
        layout.prop(lrd, "bloom_size", text="Size")

class LeafRender_PT_posteffects(LeafRenderButtonsPanel, Panel):
    bl_label = "Post Effects"

    def draw(self, context):
        layout = self.layout
        rd = context.scene.render
        lrd = context.scene.leaf

        layout.prop(lrd, "pixellate_divider", text="Pixellate")
        layout.prop(lrd, "vignette_size")
        layout.prop(lrd, "vignette_power")
        layout.prop(lrd, "abberation_strength", text="Chromatic Abberation")
        layout.prop(lrd, "scanline_strength")
        layout.prop(lrd, "scanline_frequency")
        layout.prop(lrd, "scanline_offset")

class LeafRender_PT_audio_setup(LeafRenderButtonsPanel, Panel):
    bl_label = "Audio Setup"

    def draw(self, context):
        layout = self.layout
        rd = context.scene.render
        lrd = context.scene.leaf

        layout.prop(lrd, "audio_file", text="Wav File")
        layout.prop(lrd, "audio_bpm", text="BPM")

        layout.operator("leaf.setup_audio", text="Setup Audio", icon='FILE_TICK')
