import bpy

from bpy.types import Panel, Menu, Operator, UIList
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty)

# class LeafTextureSettings(bpy.types.PropertyGroup):
    # @classmethod
    # def register(cls):
        # bpy.types.Texture.leaf = PointerProperty(
            # name="Leaf Texture Settings",
            # description="Leaf texture settings",
            # type=cls,
        # )

    # @classmethod
    # def unregister(cls):
        # del bpy.types.Texture.leaf

#class LeafTextureButtonsPanel():
#    bl_space_type = "PROPERTIES"
#    bl_region_type = "WINDOW"
#    bl_context = "texture"
#    COMPAT_ENGINES = {"LEAF"}

#    @classmethod
#    def poll(cls, context):
#        rd = context.scene.render
#        return rd.engine in cls.COMPAT_ENGINES

#class LeafTexture_UL_slots(UIList):
#    def draw_item(self, context, layout, data, item, icon, active_data, active_propname, index):
#        ma = data
#        tex = item.texture if item else None
#        slot_names = ["Base Color", "Normal Map", "Metallic", "Roughness"]
#        title = slot_names[index] if index < len(slot_names) else "<unused>"

#        if tex:
#            layout.prop(tex, "name", text=title, emboss=False, icon_value=icon)
#        else:
#            layout.label(text=title, icon_value=icon)
#        if tex and isinstance(item, bpy.types.MaterialTextureSlot):
#            layout.prop(ma, "use_textures", text="", index=index)

#class LeafTexture_PT_context(LeafTextureButtonsPanel, Panel):
#    bl_label = ""
#    bl_options = {"HIDE_HEADER"}

#    @classmethod
#    def poll(cls, context):
#        return context.material and LeafTextureButtonsPanel.poll(context)

#    def draw(self, context):
#        layout = self.layout

#        mat = context.material
#        tex = context.texture

#        row = layout.row()

#        row.template_list("LeafTexture_UL_slots", "", mat, "texture_slots", mat, "active_texture_index", rows=4, maxrows=4)

#        col = row.column(align=True)
#        col.operator("texture.slot_move", text="", icon='TRIA_UP').type = 'UP'
#        col.operator("texture.slot_move", text="", icon='TRIA_DOWN').type = 'DOWN'
#        col.menu("TEXTURE_MT_specials", icon='DOWNARROW_HLT', text="")

#        layout.template_ID(mat, "active_texture", new="texture.new")

#        if tex:
#            split = layout.split(percentage=0.2)
#            split.label(text="Type:")
#            split.prop(tex, "type", text="")
