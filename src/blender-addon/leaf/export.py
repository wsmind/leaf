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

    data["meshes"] = {}
    for mesh in bpy.data.meshes:
        if mesh.is_updated or not updated_only:
            print("exporting mesh: " + mesh.name)
            data["meshes"][mesh.name] = export_mesh(mesh)

    return data

def export_material(mtl):
    return {
        "diffuse": [mtl.diffuse_color.r, mtl.diffuse_color.g, mtl.diffuse_color.b]
    }

def export_mesh(mesh):
    mesh.update(calc_tessface=True)

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

    return {
        "vertices": vertices,
        "vertexCount": vertexCount
    }
