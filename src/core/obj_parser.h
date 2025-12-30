#ifndef OBJ_PARSER_H
#define OBJ_PARSER_H

#include <fstream>
#include <string>
#include <sstream>

#include "rtweekend.h"
#include "hittable.h"
#include "hittable_list.h"
#include "triangle.h"

class obj_parser {
public:
    obj_parser() = default;

    bool load(const std::string& path)
    {
        std::vector<point3> temp_vertices;
        std::vector<std::vector<int>> temp_faces;
        std::vector<std::vector<int>> temp_face_tex_coords;
        std::vector<point2> temp_tex_coords;

        std::ifstream file(path);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << path << "\n";
            return false;
        }

        std::string line;

        while (std::getline(file, line))
        {
            if (line.size() >= 2 && line.substr(0, 2) == "v ")
            {
                double x, y, z;
                std::istringstream iss(line.substr(2));
                if(iss >> x >> y >> z)
                {
                    temp_vertices.push_back(point3(x, y, z));
                }
            }
            else if (line.size() >= 2 && line.substr(0, 2) == "vt ")
            {
                double u, v;
                std::istringstream iss(line.substr(2));
                if(iss >> u >> v)
                {
                    temp_tex_coords.push_back(point2(u, v));
                }
            }
            else if (line.size() >= 2 && line.substr(0, 2) == "f ")
            {
                std::vector<int> face;
                std::vector<int> face_tex_coords;
                std::istringstream iss(line.substr(2));

                std::string token;
                while (iss >> token)
                {
                    size_t first_slash = token.find('/');
                    size_t second_slash = token.find('/', first_slash + 1);
                    int vertex_idx = std::stoi(token.substr(0, first_slash));

                    int tex_idx = -1;
                    if (first_slash != std::string::npos && second_slash != first_slash + 1)
                    {
                        size_t tex_start = first_slash + 1;
                        size_t tex_end = (second_slash == std::string::npos) ? token.size() : second_slash;
                        tex_idx = std::stoi(token.substr(tex_start, tex_end - tex_start));
                    }
                    face.push_back(vertex_idx - 1);
                    face_tex_coords.push_back(tex_idx - 1);
                }
                temp_faces.push_back(face);
                temp_face_tex_coords.push_back(face_tex_coords);
            }
        }

        vertices = temp_vertices;
        faces = temp_faces;
        tex_coords = temp_tex_coords;
        face_tex_coords = temp_face_tex_coords;

        return true;
    }

    hittable_list parse(shared_ptr<material> mat)
    {
        hittable_list triangles;
        for (int i = 0; i < faces.size(); i++)
        {
            if (faces[i].size() == 3)
            {
                // Check if face_tex_coords[i] contains -1, if so, pass default tex_coords (point2(0, 0))
                point2 t1 = face_tex_coords[i][0] < 0 ? point2(0, 0) : tex_coords[face_tex_coords[i][0]];
                point2 t2 = face_tex_coords[i][1] < 0 ? point2(0, 0) : tex_coords[face_tex_coords[i][1]];
                point2 t3 = face_tex_coords[i][2] < 0 ? point2(0, 0) : tex_coords[face_tex_coords[i][2]];

                triangles.add(make_shared<triangle>(vertices[faces[i][0]], vertices[faces[i][1]], vertices[faces[i][2]], t1, t2, t3, mat));
            }
            else if (faces[i].size() == 4)
            {
                point2 t1 = face_tex_coords[i][0] < 0 ? point2(0, 0) : tex_coords[face_tex_coords[i][0]];
                point2 t2 = face_tex_coords[i][1] < 0 ? point2(0, 0) : tex_coords[face_tex_coords[i][1]];
                point2 t3 = face_tex_coords[i][2] < 0 ? point2(0, 0) : tex_coords[face_tex_coords[i][2]];
                point2 t4 = face_tex_coords[i][3] < 0 ? point2(0, 0) : tex_coords[face_tex_coords[i][3]];
                
                triangles.add(make_shared<triangle>(vertices[faces[i][0]], vertices[faces[i][1]], vertices[faces[i][2]], t1, t2, t3, mat));
                triangles.add(make_shared<triangle>(vertices[faces[i][0]], vertices[faces[i][2]], vertices[faces[i][3]], t1, t3, t4, mat));
            }

        }
        return triangles;
    }

private:
    std::vector<point3> vertices;
    std::vector<std::vector<int>> faces;
    std::vector<point2> tex_coords;
    std::vector<std::vector<int>> face_tex_coords;
};

#endif