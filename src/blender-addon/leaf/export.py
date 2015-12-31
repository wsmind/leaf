import bpy

def export_data(updated_only=False):
    data = {}

    data["objects"] = {}
    for obj in bpy.data.objects:
        if obj.is_updated or not updated_only:
            print("exporting object: " + obj.name)

    data["materials"] = {}
    for mtl in bpy.data.materials:
        if mtl.is_updated or not updated_only:
            print("exporting material: " + mtl.name)
            data["materials"][mtl.name] = export_material(mtl)

    return data

def export_material(mtl):
    return {
        "diffuse": [mtl.diffuse_color.r, mtl.diffuse_color.g, mtl.diffuse_color.b]
    }
