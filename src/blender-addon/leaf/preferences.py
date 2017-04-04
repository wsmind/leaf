import os
import shutil

import bpy
from bpy.types import AddonPreferences
from bpy.props import BoolProperty

class LeafPreferences(bpy.types.AddonPreferences):
    bl_idname = __package__

    cuda_enabled = BoolProperty(
        name="CUDA accelerated texture compression",
        description="Faster texture compression, but sometimes less predictable results on specific hardware",
        default=True,
    )

    def draw(self, context):
        layout = self.layout
        layout.prop(self, "cuda_enabled")
