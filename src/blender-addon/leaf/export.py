import bpy
import mathutils
import ctypes

def export_data(updated_only=False):
    data = {}
    blobs = {}

    data["scenes"] = {}
    for scene in list(bpy.data.scenes):
        if scene.is_updated or not updated_only:
        #if True:
            print("exporting scene: " + scene.name)
            data["scenes"][scene.name] = export_scene(scene)
            #import json
            #print(json.dumps(data["scenes"][scene.name]))

    data["materials"] = {}
    generated_textures = {}
    generated_images = {}
    for mtl in list(bpy.data.materials):
        if mtl.is_updated or not updated_only:
            print("exporting material: " + mtl.name)
            data["materials"][mtl.name] = export_material(mtl, blobs, generated_textures, generated_images)

    data["textures"] = {}
    for tex in list(bpy.data.textures):
        if tex.is_updated or not updated_only:
            print("exporting texture: " + tex.name)
            data["textures"][tex.name] = export_texture(tex)

    data["images"] = {}
    for img in list(bpy.data.images):
        if img.is_updated or not updated_only:
            print("exporting image: " + img.name)
            data["images"][img.name] = export_image(img, blobs)
            #import json
            #print(json.dumps(data["images"][img.name]))

    data["meshes"] = {}
    for mesh in list(bpy.data.meshes):
        if mesh.is_updated or not updated_only:
            print("exporting mesh: " + mesh.name)
            data["meshes"][mesh.name] = export_mesh(mesh)

    data["actions"] = {}
    for action in list(bpy.data.actions):
        if action.is_updated or not updated_only:
        #if True:
            print("exporting action: " + action.name)
            data["actions"][action.name] = export_action(action)

    data["lights"] = {}
    for light in list(bpy.data.lamps):
        if light.is_updated or not updated_only:
            print("exporting light: " + light.name)
            data["lights"][light.name] = export_light(light)

    data["cameras"] = {}
    for camera in list(bpy.data.cameras):
        if camera.is_updated or not updated_only:
            print("exporting camera: " + camera.name)
            data["cameras"][camera.name] = export_camera(camera)

    data["textures"].update(generated_textures)
    data["images"].update(generated_images)

    return data, blobs

def export_scene(scene):
    return {
        "meshes": [export_scene_node(obj) for obj in scene.objects if obj.type == "MESH"],
        "lights": [export_scene_node(obj) for obj in scene.objects if obj.type == "LAMP"],
        "cameras": [export_scene_node(obj) for obj in scene.objects if obj.type == "CAMERA"]
    }

def export_scene_node(obj):
    mesh_instance = {
        "position": [obj.location.x, obj.location.y, obj.location.z],
        "orientation": [obj.rotation_euler.x, obj.rotation_euler.y, obj.rotation_euler.z],
        "scale": [obj.scale.x, obj.scale.y, obj.scale.z],
        "hide": float(obj.hide),
        "data": obj.data.name
    }

    if obj.animation_data:
        mesh_instance["animation"] = export_animation(obj.animation_data)

    return mesh_instance

def export_material(mtl, blobs, generated_textures, generated_images):

    def make_albedo_texture(color):
        name = "__generated_albedo_" + mtl.name

        generated_textures[name] = {
            "type": "IMAGE",
            "image": name
        }

        generated_images[name] = {
            "width": 1,
            "height": 1,
            "channels": 4,
            "float": False,
            "pixels": name
        }

        blobs[name] = (ctypes.c_uint8 * 4)(int(color.r * 255.0), int(color.g * 255.0), int(color.b * 255.0), 0)

        return name

    def make_default_normal_map():
        name = "__generated_normal_map"

        generated_textures[name] = {
            "type": "IMAGE",
            "image": name
        }

        generated_images[name] = {
            "width": 1,
            "height": 1,
            "channels": 4,
            "float": False,
            "pixels": name
        }

        blobs[name] = (ctypes.c_uint8 * 4)(128, 128, 255, 0)

        return name

    def make_metalness_texture(metalness):
        name = "__generated_metalness_" + mtl.name

        generated_textures[name] = {
            "type": "IMAGE",
            "image": name
        }

        generated_images[name] = {
            "width": 1,
            "height": 1,
            "channels": 4,
            "float": False,
            "pixels": name
        }

        blobs[name] = (ctypes.c_uint8 * 4)(int(metalness * 255.0), 0, 0, 0)

        return name

    def make_roughness_texture(roughness):
        name = "__generated_roughness_" + mtl.name

        generated_textures[name] = {
            "type": "IMAGE",
            "image": name
        }

        generated_images[name] = {
            "width": 1,
            "height": 1,
            "channels": 4,
            "float": False,
            "pixels": name
        }

        blobs[name] = (ctypes.c_uint8 * 4)(int(roughness * 255.0), 0, 0, 0)

        return name

    lmtl = mtl.leaf
    return {
        "albedo": [mtl.diffuse_color.r, mtl.diffuse_color.g, mtl.diffuse_color.b],
        "emit": [mtl.emit],
        "albedoTexture": mtl.texture_slots[0].name if mtl.texture_slots[0] and mtl.texture_slots[0].use else make_albedo_texture(mathutils.Color((1.0, 1.0, 1.0))),
        "normalTexture": mtl.texture_slots[1].name if mtl.texture_slots[1] and mtl.texture_slots[1].use else make_default_normal_map(),
        "metalnessTexture": mtl.texture_slots[2].name if mtl.texture_slots[2] and mtl.texture_slots[2].use else make_metalness_texture(lmtl.metalness),
        "roughnessTexture": mtl.texture_slots[3].name if mtl.texture_slots[3] and mtl.texture_slots[3].use else make_roughness_texture(lmtl.roughness)
    }

def export_texture(tex):
    # filter out unsupported types
    if tex.type not in ["IMAGE"]:
        return {
            "type": "IMAGE",
            "image": "__default"
        }

    output = {
        "type": tex.type
    }

    if tex.type == "IMAGE":
        output["image"] = tex.image.name if tex.image else "__default"

    return output

def export_image(img, blobs):
    blob_name = "image_" + img.name

    pixel_data = None
    if img.is_float:
        pixel_data = (ctypes.c_float * (img.size[0] * img.size[1] * img.channels))()
        i = 0
        tmp = img.pixels[:]
        for component in tmp:
            pixel_data[i] = component
            i += 1
    else:
        pixel_data = (ctypes.c_uint8 * (img.size[0] * img.size[1] * img.channels))()
        i = 0
        tmp = img.pixels[:]
        for component in tmp:
            pixel_data[i] = int(component * 255.0)
            i += 1

    blobs[blob_name] = pixel_data

    return {
        "width": img.size[0],
        "height": img.size[1],
        "channels": img.channels,
        "float": img.is_float,
        "pixels": blob_name
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
            uv = texFace.uv[e] if texFace else (vertex.co.x, vertex.co.y + vertex.co.z)

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

def export_action(action):
    return {
        "fcurves": [export_fcurve(fcurve) for fcurve in action.fcurves],
    }

def export_fcurve(fcurve):
    return {
        "path": fcurve.data_path,
        "index": fcurve.array_index,
        "keyframes": [export_keyframe(keyframe) for keyframe in fcurve.keyframe_points]
    }

def export_keyframe(keyframe):
    return [
        export_interpolation(keyframe.interpolation),
        keyframe.co.x,
        keyframe.co.y,
        keyframe.handle_left.x,
        keyframe.handle_left.y,
        keyframe.handle_right.x,
        keyframe.handle_right.y
    ]

def export_interpolation(interpolation):
    # convert interpolation name to function index in FCurve.cpp
    if interpolation == "CONSTANT": return 0
    if interpolation == "LINEAR": return 1
    if interpolation == "BEZIER": return 2
    return 0

def export_animation(anim_data):
    return {
        "action": anim_data.action.name
    }

def export_light(light):
    data = {}

    if light.animation_data:
        data["animation"] = export_animation(light.animation_data)

    return data

def export_camera(camera):
    data = {
        "lens": camera.lens,
        "clip_start": camera.clip_start,
        "clip_end": camera.clip_end,
        "sensor_height": camera.sensor_height
    }

    if camera.animation_data:
        data["animation"] = export_animation(camera.animation_data)

    return data
