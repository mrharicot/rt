#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>

#include "mesh.h"

#include "string_tools.h"

typedef unsigned char uchar;


Hit Mesh::intersect(const ray &r, float t_min, float t_max)
{

    std::pair<bool, float> hit;
    Triangle* face = NULL;
    float min_depth = t_max;
    bool did_hit = false;
    for (int k = 0; k < nb_faces(); ++k)
    {
        hit = m_faces[k].intersect(r);
        if (hit.first && hit.second < min_depth)
        {
            face = &m_faces[k];
            min_depth = hit.second;
            did_hit = true;
        }
    }
    return Hit(did_hit, min_depth, face);
}

Mesh read_ply(const char* file_path)
{
    enum Read_mode {ASCII, BINARY};

    Mesh mesh;
    std::vector<float3> vertices;
    std::vector<float3> normals;
    std::vector<int3>   faces;

    std::ifstream file(file_path, std::ifstream::in | std::ifstream::binary);

    std::string line;
    std::vector<std::string> tokens;

    Read_mode read_mode;

    std::getline(file, line);
    if (line != "ply")
    {
        std::cout << "File is not ply" << std::endl;
        file.close();
        return mesh;
    }

    std::getline(file, line);
    tokens = split_whitespaces(line);
    if (tokens[1] == "ascii")
    {
        read_mode = ASCII;
    }
    else if (tokens[1] == "binary_little_endian")
    {
        read_mode = BINARY;
    }

    int nb_verts = 0;
    int nb_faces = 0;
    std::vector<std::string> vert_properties;

    while (std::getline(file, line))
    {
        tokens = split_whitespaces(line);

        if (tokens[0] == "end_header")
            break;

        if (tokens[0] == "comment")
            continue;

        if (tokens[0] == "element")
        {
            if (tokens[1] == "vertex")
            {
                nb_verts = std::stoi(tokens[2]);
            }
            else if (tokens[1] == "face")
            {
                nb_faces = std::stoi(tokens[2]);
            }
        }

        if (tokens[0] == "property")
        {
            if (nb_verts && !nb_faces)
            {
                vert_properties.push_back(tokens[2]);
            }
            else
            {
                if (tokens[2] != "uchar" && tokens[3] != "int")
                    std::cout << "list type not supported yet" << std::endl;
            }
        }
    }

    //read vertices
    vertices.reserve(nb_verts);
    normals.reserve (nb_verts);
    float buff[6 * nb_verts];
    file.read((char*) buff, 6 * nb_verts * sizeof(float) / sizeof(char));

    for (int i = 0; i < nb_verts; ++i)
    {
        vertices.push_back(float3(buff + 6 * i));
//        vertices.push_back(buf[6 * i + 0]);
//        vertices.push_back(buf[6 * i + 1]);
//        vertices.push_back(buf[6 * i + 2]);

        normals.push_back(float3(buff + 6 * i + 3));
//        normals.push_back(buf[6 * i + 3]);
//        normals.push_back(buf[6 * i + 4]);
//        normals.push_back(buf[6 * i + 5]);
    }

    //read faces
    for (int i = 0; i < nb_faces; ++i)
    {
        uchar nb_el;
        file.read((char*) &nb_el, sizeof(uchar) / sizeof(char));
        if (nb_el != 3)
        {
            std::cout << "Error: " << nb_el << " vertices on face " << i << std::endl;
            break;
        }

        int idxs[3];
        file.read((char*) &idxs[0], 3 * sizeof(int) / sizeof(char));
        for (int j = 0; j < 3; ++j)
            faces.push_back(idxs);
    }

    file.close();

    std::cout << "PLY file loaded: " << nb_verts << " vertices and " << nb_faces << " faces." << std::endl;

//    mesh.set_face_indices(faces);
//    mesh.set_vertices(vertices);
//    mesh.set_normals(normals);

    return Mesh(vertices, faces, normals);

}
