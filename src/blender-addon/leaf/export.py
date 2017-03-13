import bpy
import mathutils
import ctypes
import json
import struct
import os.path
import subprocess
import tempfile

def export_data(output_file, updated_only=False):
    data = {}

    data["Scene"] = {}
    for scene in list(bpy.data.scenes):
        #if scene.is_updated or not updated_only:
        if True:
            print("exporting scene: " + scene.name)
            data["Scene"][scene.name] = export_scene(scene)

    data["Material"] = {}
    for mtl in list(bpy.data.materials):
        if mtl.is_updated or not updated_only:
            print("exporting material: " + mtl.name)
            data["Material"][mtl.name] = export_material(mtl)

    data["Texture"] = {}
    for tex in list(bpy.data.textures):
        if tex.is_updated or not updated_only:
            print("exporting texture: " + tex.name)
            data["Texture"][tex.name] = export_texture(tex)

    data["Image"] = {}
    for img in list(bpy.data.images):
        if img.is_updated or not updated_only:
            print("exporting image: " + img.name)
            data["Image"][img.name] = export_image(img)

    data["Mesh"] = {}
    for mesh in list(bpy.data.meshes):
        if mesh.is_updated or not updated_only:
            print("exporting mesh: " + mesh.name)
            data["Mesh"][mesh.name] = export_mesh(mesh)

    data["Action"] = {}
    for action in list(bpy.data.actions):
        if action.is_updated or not updated_only:
            print("exporting action: " + action.name)
            data["Action"][action.name] = export_action(action)

    data["Light"] = {}
    for light in list(bpy.data.lamps):
        if light.is_updated or not updated_only:
            print("exporting light: " + light.name)
            data["Light"][light.name] = export_light(light)

    data["Camera"] = {}
    for camera in list(bpy.data.cameras):
        if camera.is_updated or not updated_only:
            print("exporting camera: " + camera.name)
            data["Camera"][camera.name] = export_camera(camera)

    for typename, resources in data.items():
        typebytes = typename.encode("utf-8")
        for name, blob in resources.items():
            namebytes = name.encode("utf-8")
            # type name
            output_file.write(struct.pack("=I", len(typebytes)))
            output_file.write(typebytes)

            # resource name
            output_file.write(struct.pack("=I", len(namebytes)))
            output_file.write(namebytes)

            # actual resource contents
            output_file.write(struct.pack("=I", len(blob)))
            output_file.write(blob)

def export_scene(scene):
    markers = [marker for marker in scene.timeline_markers if marker.camera]
    markers = sorted(markers, key = lambda m: m.frame)

    # sort objects by parenting depth to ensure child transform correctness
    objects = scene.objects[:]
    objects = sorted(objects, key = lambda obj: compute_parent_depth(obj))

    data = {
        "nodes": [export_scene_node(obj, objects) for obj in objects],
        "markers": [export_marker(marker, objects) for marker in markers],
        "activeCamera": objects.index(scene.camera)
    }

    return json.dumps(data).encode("utf-8")

def compute_parent_depth(obj):
    depth = 0
    while (obj.parent != None):
        obj = obj.parent
        depth = depth + 1

    return depth

def export_scene_node(obj, all_objects):
    node = {
        "type": export_object_type(obj.type),
        "position": [obj.location.x, obj.location.y, obj.location.z],
        "orientation": [obj.rotation_euler.x, obj.rotation_euler.y, obj.rotation_euler.z],
        "scale": [obj.scale.x, obj.scale.y, obj.scale.z],
        "hide": float(obj.hide),
        "data": obj.data.name if obj.data else ""
    }

    if obj.parent:
        pm = obj.matrix_parent_inverse
        node["parent"] = all_objects.index(obj.parent)
        node["parentMatrix"] = [
            pm[0][0], pm[1][0], pm[2][0], pm[3][0],
            pm[0][1], pm[1][1], pm[2][1], pm[3][1],
            pm[0][2], pm[1][2], pm[2][2], pm[3][2],
            pm[0][3], pm[1][3], pm[2][3], pm[3][3]
        ]

    if obj.animation_data:
        node["animation"] = export_animation(obj.animation_data)

    return node

def export_object_type(type):
    if type == "CAMERA":
        return 0
    if type == "MESH":
        return 1
    if type == "LAMP":
        return 2
    return -1

def export_marker(marker, camera_objects):
    return {
        "camera": camera_objects.index(marker.camera),
        "time": marker.frame
    }

def export_material(mtl):

    blobs = {}
    generated_textures = {}
    generated_images = {}

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
    data = {
        "albedo": [mtl.diffuse_color.r, mtl.diffuse_color.g, mtl.diffuse_color.b],
        "emit": mtl.emit,
        "albedoTexture": mtl.texture_slots[0].name if mtl.texture_slots[0] and mtl.texture_slots[0].use else make_albedo_texture(mathutils.Color((1.0, 1.0, 1.0))),
        "normalTexture": mtl.texture_slots[1].name if mtl.texture_slots[1] and mtl.texture_slots[1].use else make_default_normal_map(),
        "metalnessTexture": mtl.texture_slots[2].name if mtl.texture_slots[2] and mtl.texture_slots[2].use else make_metalness_texture(lmtl.metalness),
        "roughnessTexture": mtl.texture_slots[3].name if mtl.texture_slots[3] and mtl.texture_slots[3].use else make_roughness_texture(lmtl.roughness)
    }

    if mtl.animation_data:
        data["animation"] = export_animation(mtl.animation_data)

    return json.dumps(data).encode("utf-8")


def export_texture(tex):
    # filter out unsupported types
    if tex.type not in ["IMAGE"]:
        data = {
            "type": "IMAGE",
            "image": "__default"
        }
        return json.dumps(data).encode("utf-8")

    output = {
        "type": tex.type
    }

    if tex.type == "IMAGE":
        output["image"] = tex.image.name if tex.image else "__default"

    return json.dumps(output).encode("utf-8")

def export_image(img):
    if img.filepath == "":
        return b''

    script_dir = os.path.dirname(__file__)
    texture_compressor_path = os.path.join(script_dir, "LeafTextureCompressor.exe")

    sourcePath = bpy.path.abspath(img.filepath)
    targetPath = os.path.join(bpy.app.tempdir, next(tempfile._get_candidate_names()))
    args = [
        texture_compressor_path,
        "-repeat",
        #"-nocuda",
        "-dds10"
    ]

    if img.colorspace_settings.name == "Linear":
        args.append("-normal")
        args.append("-bc1n")
    else:
        args.append("-color")
        args.append("-bc1")
        args.append("-srgb")

    args.append(sourcePath)
    args.append(targetPath)

    print("running: " + str(args))
    subprocess.run(args)

    with open(targetPath, mode="rb") as outputFile:
        return outputFile.read()

def export_mesh(mesh):
    vertices = []
    vertexCount = 0

    uv_layer = None

    if len(mesh.uv_layers) > 0:
        uv_layer = mesh.uv_layers[0]

        # will also compute split tangents according to sharp edges
        mesh.calc_tangents()

    def output_vertex(loop_index):
        loop = mesh.loops[loop_index]
        vertex = mesh.vertices[loop.vertex_index]
        uv = uv_layer.data[loop_index].uv if uv_layer else (vertex.co.x, vertex.co.y + vertex.co.z)
        vertices.append(vertex.co.x)
        vertices.append(vertex.co.y)
        vertices.append(vertex.co.z)
        vertices.append(loop.normal.x)
        vertices.append(loop.normal.y)
        vertices.append(loop.normal.z)
        vertices.append(loop.tangent.x)
        vertices.append(loop.tangent.y)
        vertices.append(loop.tangent.z)
        vertices.append(loop.bitangent_sign)
        vertices.append(uv[0])
        vertices.append(uv[1])

    # build triangles out of n-gons
    for face in mesh.polygons:
        for i in range(len(face.loop_indices) - 2):
            output_vertex(face.loop_indices[0])
            output_vertex(face.loop_indices[i + 1])
            output_vertex(face.loop_indices[i + 2])
            vertexCount += 3

    data = {
        "vertices": vertices,
        "vertexCount": vertexCount,
        "material": mesh.materials[0].name if (len(mesh.materials) > 0 and mesh.materials[0]) else "__default"
    }

    return json.dumps(data).encode("utf-8")

def export_action(action):
    data = {
        "fcurves": [export_fcurve(fcurve) for fcurve in action.fcurves],
    }

    return json.dumps(data).encode("utf-8")

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
    data = {
        "color": [light.color.r * light.energy, light.color.g * light.energy, light.color.b * light.energy],
        "radius": light.distance
    }

    if light.animation_data:
        data["animation"] = export_animation(light.animation_data)

    return json.dumps(data).encode("utf-8")

def export_camera(camera):
    data = {
        "lens": camera.lens,
        "ortho_scale": camera.ortho_scale,
        "clip_start": camera.clip_start,
        "clip_end": camera.clip_end,
        "sensor_height": camera.sensor_height,
        "type": export_camera_type(camera.type)
    }

    if camera.animation_data:
        data["animation"] = export_animation(camera.animation_data)

    return json.dumps(data).encode("utf-8")

def export_camera_type(type):
    if type == "PERSP":
        return 0
    if type == "ORTHO":
        return 1
    if type == "PANO":
        return 2

    return 0
