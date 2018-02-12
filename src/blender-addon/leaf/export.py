import bpy
import math
import mathutils
import ctypes
import json
import io
import struct
import sys
import os.path
import subprocess
import tempfile

from . import cooking

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
            mesh_data = export_mesh(mesh)
            if mesh_data is not None:
                data["Mesh"][mesh.name] = mesh_data
            else:
                print("failed to export mesh " + mesh.name)

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
    leaf_scene = scene.leaf

    markers = [marker for marker in scene.timeline_markers if marker.camera]
    markers = sorted(markers, key = lambda m: m.frame)

    # sort objects by parenting depth to ensure child transform correctness
    objects = scene.objects[:]
    objects = sorted(objects, key = lambda obj: compute_parent_depth(obj))

    ambient = [0.0, 0.0, 0.0]
    mist = 0.0
    world = scene.world
    environmentMap = "__default"
    if world:
        ambient = [world.ambient_color.r, world.ambient_color.g, world.ambient_color.b]
        mist = world.mist_settings.intensity
        environmentMap = world.active_texture.name if world.active_texture else "__default"

    data = {
        "nodes": [export_scene_node(obj, objects) for obj in objects],
        "markers": [export_marker(marker, objects) for marker in markers],
        "activeCamera": objects.index(scene.camera) if scene.camera else 0,
        "ambientColor": ambient,
        "mist": mist,
        "environmentMap": environmentMap,
        "bloom": {
            "threshold": leaf_scene.bloom_threshold,
            "intensity": leaf_scene.bloom_intensity,
            "debug": leaf_scene.bloom_debug
        }
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
    lmtl = mtl.leaf

    export_bsdf_function = {
        "STANDARD": export_bsdf_standard,
        "UNLIT": export_bsdf_unlit
    }
    data = export_bsdf_function[lmtl.bsdf](mtl)

    data["bsdf"] = lmtl.bsdf

    if mtl.animation_data:
        data["animation"] = export_animation(mtl.animation_data)

    return json.dumps(data).encode("utf-8")

def export_bsdf_standard(mtl):
    lmtl = mtl.leaf
    return {
        "baseColorMultiplier": [mtl.diffuse_color.r, mtl.diffuse_color.g, mtl.diffuse_color.b],
        "emissive": [lmtl.emissive.r, lmtl.emissive.g, lmtl.emissive.b],
        "metallicOffset": lmtl.metallic_offset,
        "roughnessOffset": lmtl.roughness_offset,
        "baseColorMap": export_texture_slot(mtl, 0, "__default_white"),
        "normalMap": export_texture_slot(mtl, 1, "__default_normal"),
        "metallicMap": export_texture_slot(mtl, 2, "__default_black"),
        "roughnessMap": export_texture_slot(mtl, 3, "__default_black")
    }

def export_bsdf_unlit(mtl):
    lmtl = mtl.leaf
    return {
        "emissive": [lmtl.emissive.r, lmtl.emissive.g, lmtl.emissive.b],
        "emissiveMap": export_texture_slot(mtl, 0, "__default_white")
    }

def export_texture_slot(mtl, slot_index, default):
    slot = mtl.texture_slots[slot_index]

    if not slot: return default
    if not slot.use: return default
    if not slot.texture: return default

    return slot.texture.name

def export_texture(tex):
    # filter out unsupported types
    if tex.type not in ["IMAGE", "ENVIRONMENT_MAP"]:
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
    if tex.type == "ENVIRONMENT_MAP":
        output["image"] = tex.image.name if tex.image else "__default"

    return json.dumps(output).encode("utf-8")

def export_image(img):
    options = {
        "cuda": bpy.context.user_preferences.addons[__package__].preferences.cuda_enabled,
        "linear": img.colorspace_settings.name == "Linear",
        "normal_map": img.leaf.is_normal_map,
        "hdr": img.filepath[-4:] == ".hdr"
    }

    return cooking.cooker.cook("image", img.filepath, options)


def export_mesh(mesh):
    #min_bound = (math.inf, math.inf, math.inf)
    #max_bound = (-math.inf, -math.inf, -math.inf)

    uv_layer_data = None
    
    import time
    t0 = time.perf_counter()
    
    if len(mesh.uv_layers) > 0 and len(mesh.uv_layers[0].data) > 0:
        uv_layer_data = mesh.uv_layers[0].data

        # will also compute split tangents according to sharp edges
        try:
            mesh.calc_tangents()
        except:
            print("Failed to compute tangents for mesh '%s': %s" % (mesh.name, sys.exc_info()[0]))

    t1 = time.perf_counter()

    vertexCount = len(mesh.loops)
    if vertexCount == 0:
        return None

    output = io.BytesIO()

    # vertex (loop) count
    output.write(struct.pack("=I", len(mesh.loops)))

    # export all loops
    default_uv = (0.0, 0.0)
    vertices = mesh.vertices[:]
    for loop in mesh.loops:
        vertex = vertices[loop.vertex_index]
        uv = uv_layer_data[loop.index].uv if uv_layer_data else default_uv

        #min_bound = min(min_bound[0], vertex.co.x), min(min_bound[1], vertex.co.y), min(min_bound[2], vertex.co.y)
        #max_bound = max(max_bound[0], vertex.co.x), max(max_bound[1], vertex.co.y), max(max_bound[2], vertex.co.y)
        
        position = vertex.co.to_3d()
        normal = loop.normal.to_3d()
        tangent = loop.tangent.to_3d()
        
        output.write(struct.pack(
            "=ffffffffffff",
            position[0],
            position[1],
            position[2],
            normal[0],
            normal[1],
            normal[2],
            tangent[0],
            tangent[1],
            tangent[2],
            loop.bitangent_sign,
            uv[0],
            uv[1]
        ))

    t2 = time.perf_counter()

    polygons = mesh.polygons
    
    # build per-material index lists
    indices = []
    for material_index in range(len(mesh.materials)):
        indices.append([])
        for face in (polygon for polygon in polygons if polygon.material_index == material_index):
            face_indices = face.loop_indices
            for i in range(len(face_indices) - 2):
                indices[material_index].append(face_indices[0])
                indices[material_index].append(face_indices[i + 1])
                indices[material_index].append(face_indices[i + 2])
    
    # output each material as a separate index buffer
    output.write(struct.pack("=I", len(mesh.materials)))
    for material_index, material in enumerate(mesh.materials):
        index_list = indices[material_index]

        # material name
        material_name_bytes = material.name.encode("utf-8") if material else "__default".encode("utf-8")
        output.write(struct.pack("=I", len(material_name_bytes)))
        output.write(material_name_bytes)

        # index buffer
        output.write(struct.pack("=I", len(index_list)))
        for index in index_list:
            output.write(struct.pack("=I", index))

    t3 = time.perf_counter()

    print("Mesh export timings (seconds):")
    print("  Tangents  " + str(t1 - t0))
    print("  Vertices  " + str(t2 - t1))
    print("  Indices   " + str(t3 - t2))
    print("  Total     " + str(t3 - t0))

    return output.getvalue()

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
        "action": anim_data.action.name if anim_data.action else "__default"
    }

def export_light(light):
    data = {
        "color": [light.color.r, light.color.g, light.color.b],
        "energy": light.energy,
        "radius": light.distance,
        "type": export_light_type(light.type),
        "spotAngle": light.spot_size if light.type == "SPOT" else 0,
        "spotBlend": light.spot_blend if light.type == "SPOT" else 0,
        "scattering": light.leaf.scattering
    }

    if light.animation_data:
        data["animation"] = export_animation(light.animation_data)

    return json.dumps(data).encode("utf-8")

def export_light_type(type):
    if type == "POINT": return 0
    if type == "SPOT": return 1
    return 0

def export_camera(camera):
    data = {
        "lens": camera.lens,
        "ortho_scale": camera.ortho_scale,
        "clip_start": camera.clip_start,
        "clip_end": camera.clip_end,
        "dof_blades": camera.gpu_dof.blades,
        "dof_distance": camera.dof_distance,
        "dof_fstop": camera.gpu_dof.fstop,
        "sensor_height": camera.sensor_height,
        "type": export_camera_type(camera.type),
        "shutter_speed": camera.leaf.shutter_speed
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
