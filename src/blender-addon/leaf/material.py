import bpy

from bpy.types import Panel, Menu, Operator
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty,
                       FloatVectorProperty)

class LeafMaterialSettings(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        bpy.types.Material.leaf = PointerProperty(
            name="Leaf Material Settings",
            description="Leaf material settings",
            type=cls,
        )
        cls.bsdf = EnumProperty(
            name="Shading",
            items=(
                ("STANDARD", "Standard", "Standard PBR Metallic/Roughness"),
                ("UNLIT", "Unlit", "Pure emissive")
            ),
            default="STANDARD"
        )
        cls.metallic_offset = FloatProperty(
            name="Metallic Offset",
            description="0: dielectric, 1: metal",
            min=-1.0, max=1.0,
            default=0.0,
            subtype="FACTOR"
        )
        cls.roughness_offset = FloatProperty(
            name="Roughness Offset",
            description="0: smooth, 1: rough",
            min=-1.0, max=1.0,
            default=0.0,
            subtype="FACTOR"
        )
        cls.emissive = FloatVectorProperty(
            name="Emissive",
            subtype='COLOR',
            description="Unshaded color",
            default=[0.0, 0.0, 0.0]
        )
        cls.uv_scale = FloatVectorProperty(
            name="UV Scale",
            description="Multiplies the mesh UV data",
            size=2,
            default=[1.0, 1.0]
        )
        cls.uv_offset = FloatVectorProperty(
            name="UV Offset",
            description="Offsets the mesh UV data",
            size=2,
            default=[0.0, 0.0]
        )

    @classmethod
    def unregister(cls):
        del bpy.types.Material.leaf

class LeafMaterialButtonsPanel():
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "material"
    COMPAT_ENGINES = {"LEAF"}

    @classmethod
    def poll(cls, context):
        rd = context.scene.render
        return rd.engine in cls.COMPAT_ENGINES

class LeafMaterial_PT_context(LeafMaterialButtonsPanel, Panel):
    bl_label = ""
    bl_options = {'HIDE_HEADER'}

    @classmethod
    def poll(cls, context):
        return (context.material or context.object) and LeafMaterialButtonsPanel.poll(context)

    def draw(self, context):
        layout = self.layout

        mat = context.material
        ob = context.object
        slot = context.material_slot
        space = context.space_data
        is_sortable = len(ob.material_slots) > 1

        if ob:
            rows = 1
            if (is_sortable):
                rows = 4

            row = layout.row()

            row.template_list("MATERIAL_UL_matslots", "", ob, "material_slots", ob, "active_material_index", rows=rows)

            col = row.column(align=True)
            col.operator("object.material_slot_add", icon='ZOOMIN', text="")
            col.operator("object.material_slot_remove", icon='ZOOMOUT', text="")

            col.menu("MATERIAL_MT_specials", icon='DOWNARROW_HLT', text="")

            if is_sortable:
                col.separator()

                col.operator("object.material_slot_move", icon='TRIA_UP', text="").direction = 'UP'
                col.operator("object.material_slot_move", icon='TRIA_DOWN', text="").direction = 'DOWN'

            if ob.mode == 'EDIT':
                row = layout.row(align=True)
                row.operator("object.material_slot_assign", text="Assign")
                row.operator("object.material_slot_select", text="Select")
                row.operator("object.material_slot_deselect", text="Deselect")

        split = layout.split(percentage=0.65)

        if ob:
            split.template_ID(ob, "active_material", new="material.new")
            row = split.row()

            if slot:
                row.prop(slot, "link", text="")
            else:
                row.label()
        elif mat:
            split.template_ID(space, "pin_id")
            split.separator()

class LeafMaterial_PT_common(LeafMaterialButtonsPanel, Panel):
    bl_label = ""
    bl_options = {'HIDE_HEADER'}

    @classmethod
    def poll(cls, context):
        return context.material and LeafMaterialButtonsPanel.poll(context)

    def draw(self, context):
        layout = self.layout
        mat = context.material
        lmat = context.material.leaf

        layout.prop(lmat, "bsdf")

class LeafMaterialBSDFPanel(LeafMaterialButtonsPanel):
    @classmethod
    def poll(cls, context, bsdf):
        return context.material and LeafMaterialButtonsPanel.poll(context) and context.material.leaf.bsdf == bsdf

    def texture_picker(self, material, slot_index):
        slot = material.texture_slots[slot_index] or material.texture_slots.create(slot_index)

        row = self.layout.row()
        row.prop(slot, "use", text="")
        row.template_ID(slot, "texture", new="texture.new")

###############################################################################
# Standard BSDF
###############################################################################

class LeafMaterial_PT_Standard_BaseColor(LeafMaterialBSDFPanel, Panel):
    bl_label = "Base Color"

    @classmethod
    def poll(cls, context):
        return LeafMaterialBSDFPanel.poll(context, "STANDARD")

    def draw(self, context):
        layout = self.layout
        mat = context.material

        layout.prop(mat, "diffuse_color", text="")
        self.texture_picker(mat, 0)

class LeafMaterial_PT_Standard_Emissive(LeafMaterialBSDFPanel, Panel):
    bl_label = "Emissive"

    @classmethod
    def poll(cls, context):
        return LeafMaterialBSDFPanel.poll(context, "STANDARD")

    def draw(self, context):
        layout = self.layout
        lmat = context.material.leaf

        layout.prop(lmat, "emissive", text="")

class LeafMaterial_PT_Standard_Normals(LeafMaterialBSDFPanel, Panel):
    bl_label = "Normals"

    @classmethod
    def poll(cls, context):
        return LeafMaterialBSDFPanel.poll(context, "STANDARD")

    def draw(self, context):
        self.texture_picker(context.material, 1)

class LeafMaterial_PT_Standard_Metallic(LeafMaterialBSDFPanel, Panel):
    bl_label = "Metallic"

    @classmethod
    def poll(cls, context):
        return LeafMaterialBSDFPanel.poll(context, "STANDARD")

    def draw(self, context):
        layout = self.layout
        mat = context.material
        lmat = context.material.leaf

        self.texture_picker(mat, 2)
        layout.prop(lmat, "metallic_offset")

class LeafMaterial_PT_Standard_Roughness(LeafMaterialBSDFPanel, Panel):
    bl_label = "Roughness"

    @classmethod
    def poll(cls, context):
        return LeafMaterialBSDFPanel.poll(context, "STANDARD")

    def draw(self, context):
        layout = self.layout
        mat = context.material
        lmat = context.material.leaf

        self.texture_picker(mat, 3)
        layout.prop(lmat, "roughness_offset")

class LeafMaterial_PT_Standard_UVTransform(LeafMaterialBSDFPanel, Panel):
    bl_label = "UV Transform"

    @classmethod
    def poll(cls, context):
        return LeafMaterialBSDFPanel.poll(context, "STANDARD")

    def draw(self, context):
        layout = self.layout
        mat = context.material
        lmat = context.material.leaf

        layout.prop(lmat, "uv_scale")
        layout.prop(lmat, "uv_offset")

###############################################################################
# Unlit BSDF
###############################################################################

class LeafMaterial_PT_Unlit_Emissive(LeafMaterialBSDFPanel, Panel):
    bl_label = "Emissive"

    @classmethod
    def poll(cls, context):
        return LeafMaterialBSDFPanel.poll(context, "UNLIT")

    def draw(self, context):
        layout = self.layout
        mat = context.material
        lmat = context.material.leaf

        layout.prop(lmat, "emissive", text="")
        self.texture_picker(mat, 0)

class LeafMaterial_PT_Unlit_UVTransform(LeafMaterialBSDFPanel, Panel):
    bl_label = "UV Transform"

    @classmethod
    def poll(cls, context):
        return LeafMaterialBSDFPanel.poll(context, "UNLIT")

    def draw(self, context):
        layout = self.layout
        mat = context.material
        lmat = context.material.leaf

        layout.prop(lmat, "uv_scale")
        layout.prop(lmat, "uv_offset")
