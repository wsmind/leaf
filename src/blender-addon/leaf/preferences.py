import os
import shutil

import bpy
from bpy.types import AddonPreferences
from bpy.props import StringProperty

class LeafPreferences(bpy.types.AddonPreferences):
    bl_idname = __package__

    nvttPath = StringProperty(
        name="NVIDIA Texture Tools Path",
        subtype="FILE_PATH",
        default="C:/Program Files/NVIDIA Corporation/NVIDIA Texture Tools 2"
    )

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "nvttPath")
