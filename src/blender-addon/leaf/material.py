import bpy

from bpy.types import Panel, Menu, Operator
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty)

class LeafMaterialSettings(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):
        bpy.types.Material.leaf = PointerProperty(
            name="Leaf Material Settings",
            description="Leaf material settings",
            type=cls,
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

class LeafMaterial_PT_surface(LeafMaterialButtonsPanel, Panel):
    bl_label = "Surface"

    @classmethod
    def poll(cls, context):
        return context.material and LeafMaterialButtonsPanel.poll(context)

    def draw(self, context):
        layout = self.layout
        mat = context.material
        lmat = context.material.leaf

        layout.prop(mat, "diffuse_color", text="Base Color")
        layout.prop(lmat, "metallic_offset")
        layout.prop(lmat, "roughness_offset")
