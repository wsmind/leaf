import bpy
import ctypes

def export_data(updated_only=False):
    data = {}

    data["scenes"] = {}
    for scene in list(bpy.data.scenes):
        #if scene.is_updated or not updated_only:
        if True:
            print("exporting scene: " + scene.name)
            data["scenes"][scene.name] = export_scene(scene)
            #import json
            #print(json.dumps(data["scenes"][scene.name]))

    data["materials"] = {}
    for mtl in list(bpy.data.materials):
        if mtl.is_updated or not updated_only:
            print("exporting material: " + mtl.name)
            data["materials"][mtl.name] = export_material(mtl)

    data["textures"] = {}
    for tex in list(bpy.data.textures):
        #if tex.is_updated or not updated_only:
        if True:
            print("exporting texture: " + tex.name)
            data["textures"][tex.name] = export_texture(tex)
            import json
            print(json.dumps(data["textures"][tex.name]))

    data["images"] = {}
    for img in list(bpy.data.images):
        if img.is_updated or not updated_only:
        #if True:
            print("exporting image: " + img.name)
            data["images"][img.name] = export_image(img)
            #import json
            #print(json.dumps(data["images"][img.name]))

    data["meshes"] = {}
    for mesh in list(bpy.data.meshes):
        if mesh.is_updated or not updated_only:
            print("exporting mesh: " + mesh.name)
            data["meshes"][mesh.name] = export_mesh(mesh)

    return data

def export_scene(scene):
    return {
        "meshes": [export_mesh_instance(obj) for obj in scene.objects if obj.type == "MESH"],
        "lights": [export_light(obj) for obj in scene.objects if obj.type == "LAMP"]
    }

def export_mesh_instance(obj):
    wm = obj.matrix_world.copy()
    wm.transpose()
    world_matrix = [
        wm[0][0], wm[0][1], wm[0][2], wm[0][3],
        wm[1][0], wm[1][1], wm[1][2], wm[1][3],
        wm[2][0], wm[2][1], wm[2][2], wm[2][3],
        wm[3][0], wm[3][1], wm[3][2], wm[3][3]
    ]

    return {
        "transform": world_matrix,
        "mesh": obj.data.name
    }

def export_light(obj):
    return {}

def export_material(mtl):
    lmtl = mtl.leaf
    return {
        "albedo": [mtl.diffuse_color.r, mtl.diffuse_color.g, mtl.diffuse_color.b],
        "metalness": lmtl.metalness,
        "roughness": lmtl.roughness
    }

def export_texture(tex):
    output = {
        "type": tex.type
    }

    if tex.type == "IMAGE":
        output["image"] = tex.image.name if tex.image else "__default"

    return output

def export_image(img):
    return {
        "width": img.size[0],
        "height": img.size[1],
        "channels": img.channels,
        "float": img.is_float,
        "pixels": [pixel for pixel in img.pixels]
    }

def export_mesh(sourceMesh):
    # always apply an edge split modifier, to get proper normals on sharp edges
    obj = bpy.data.objects.new("__temp_obj_for_mesh_export", sourceMesh)
    obj.modifiers.new(name="edge_split", type="EDGE_SPLIT")
    obj.modifiers["edge_split"].use_edge_angle = False
    mesh = obj.to_mesh(scene=bpy.context.scene, apply_modifiers=True, settings="PREVIEW", calc_tessface=True)

    vertices = []
    vertexCount = 0

    for i in range(0, len(mesh.tessfaces)):
        face = mesh.tessfaces[i]
        texFace = None
        if mesh.tessface_uv_textures.active:
            texFace = mesh.tessface_uv_textures.active.data[i]

        elements = (0, 1, 2)
        if len(face.vertices) == 4:
            elements = (0, 1, 2, 2, 3, 0)

        for e in elements:
            vertex = mesh.vertices[face.vertices[e]]
            uv = texFace.uv[e] if texFace else (0.0, 0.0)

            vertices.append(vertex.co.x)
            vertices.append(vertex.co.y)
            vertices.append(vertex.co.z)
            vertices.append(vertex.normal.x)
            vertices.append(vertex.normal.y)
            vertices.append(vertex.normal.z)
            vertices.append(uv[0])
            vertices.append(uv[1])
            vertexCount += 1

    bpy.data.objects.remove(obj)
    bpy.data.meshes.remove(mesh)

    return {
        "vertices": vertices,
        "vertexCount": vertexCount,
        "material": sourceMesh.materials[0].name if (len(sourceMesh.materials) > 0 and sourceMesh.materials[0]) else "__default"
    }
