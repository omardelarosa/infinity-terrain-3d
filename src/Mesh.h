#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <stdio.h>
#include <string>

// GLM
#include "glm/gtx/string_cast.hpp"
#include "lib/Helpers.h"
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/normal.hpp>

using namespace std;

class Mesh {
  private:
    string filename;
    string file_ext;
    bool isIgnorableLineInFile(string line);
    double scale = 1.0;

    unsigned int depth;
    float (*noise_callback)(float x, float y);
    float (*E)(int x, int y); // callback for elevations

  public:
    Mesh(string filename_, int _id) {
        id = _id;
        filename = filename_;
        loadFromFile(filename);
    }
    Mesh(int _id, unsigned int _width, unsigned int _height,
         float (*_noise_callback)(float x, float y)) {
        id = _id;
        width = _width;
        height = _height;
        noise_callback = _noise_callback;
        std::cout << "Procedural mesh!" << std::endl;
        generateVertexes(width, height);
    }
    unsigned int width;
    unsigned int height;
    void generateVertexes(unsigned int w, unsigned int h);
    void loadFromFile(string filename_);
    bool loadOffFile(string filename_);
    bool loadObjFile(const char *filename_);
    void setVectorsAndBuffers();
    void updateVectorsAndBuffers();
    vector<glm::vec3> GetWorldVertices(const glm::mat4 &modelMatrix,
                                       const glm::mat4 &viewMatrix,
                                       const glm::mat4 &projectionMatrix);
    glm::vec3 GetWorldCenter(const glm::mat4 &modelMatrix);
    void prepareVectors();
    void readFileData(ifstream &mesh_file);
    bool DoesHit(const glm::vec3 ray, const glm::vec3 orig,
                 const glm::mat4 &modelMatrix, float scale_factor,
                 glm::vec3 center);
    int id;
    std::string vertex_key_name;
    std::string vertex_color_key_name;
    std::string vertex_normal_key_name;
    vector<glm::vec3> vertices;
    vector<glm::vec3> vertex_colors;
    vector<glm::vec2> uvs;
    vector<glm::vec3> faces;
    vector<glm::vec3> faces_uv;
    vector<glm::vec3> triangle_normals;
    vector<glm::vec3> vertex_normals;
    vector<glm::vec3> barycentrics;
    VertexBufferObject VBO;    // vertex
    VertexBufferObject VBO_VN; // Vertex normals
    VertexBufferObject VBO_C;  // vertex color
    vector<float> vertices_vec;
    vector<float> vertex_colors_vec;
    vector<float> vertex_normals_vec;
    vector<unsigned int> indices; // index buffer for openGL
    GLuint *buffer_id;
    int vertex_offset = 0; // location offset of vertices in buffer
    int index_offset = 0;  // location offset of indices in buffer
    int num_indices = 0;   // total indices in faces
    glm::vec3 center;
    float mesh_radius;
    std::map<unsigned int, vector<int>> vertex_to_triangles_map;
    vector<vector<glm::vec3>> edges;
    bool has_loaded = false;
};

bool Mesh::isIgnorableLineInFile(string line) {
    regex re("(\#.*)|(\s+)|(OFF)");
    if (regex_match(line, re) || line == "") {
        return true;
    }

    return false;
}

void Mesh::generateVertexes(unsigned int w, unsigned int h) {
    // Create vertices
    vertices.reserve(w * h);
    float spacing = 1.0;
    // Rows
    for (int r = 0; r < h; r++) {
        // Cols
        for (int c = 0; c < w; c++) {
            // NOTE: origin is not at center of mesh
            float x = c; // col
            float z = r; // row
            float y =
                noise_callback((float)c / w,
                               (float)r / h); // look up in elevation table
            glm::vec3 v(x * spacing, y * spacing, z); //
            vertices.emplace_back(v);

            // Vertex colors not supported!
            vertex_colors.push_back(glm::vec3(0.0, 0.0, 0.0));

            // Create a vector in the mapping
            vector<int> triangles;
            vertex_to_triangles_map.emplace(vertices.size() - 1, triangles);
        }
    }

    // Generate faces

    // Rows (-1 for last)
    for (int r = 0; r < h - 1; r++) {
        // Cols (-1 for last)
        for (int c = 0; c < w - 1; c++) {
            // Upper triangle
            /*

                v0 -- v2
                |    /
                |  /
                v1
            */
            int f0_0 = (r * w) + c;
            int f0_1 = ((r + 1) * w) + c;
            int f0_2 = (r * w) + c + 1;
            glm::vec3 f0(f0_0, f0_1, f0_2);
            faces.push_back(f0); // TODO: emplace

            // Lower triangle
            /*

                      v2
                     / |
                   /   |
                v0 --- v1
            */
            int f1_0 = ((r + 1) * w) + c;
            ;
            int f1_1 = ((r + 1) * w) + c + 1;
            int f1_2 = (r * w) + c + 1;

            glm::vec3 f1(f1_0, f1_1, f1_2);
            faces.push_back(f1); // TODO: emplace
        }
    }

    prepareVectors();

    setVectorsAndBuffers();

    has_loaded = true;
}

void Mesh::prepareVectors() {

    // Compute bounding sphere
    center = glm::vec3(0.0, 0.0, 0.0);
    for (glm::vec3 vec : vertices) {
        center = center + vec;
    }
    center = center * float(1.0 / vertices.size());

    // Set number of indices
    num_indices = faces.size() * 3; // assuming only triangle primitive faces

    // Set radius
    glm::vec3 max_vertex(0, 0, 0);
    float max_dist = 0;
    for (glm::vec3 vec : vertices) {
        float d = glm::abs(glm::distance(center, vec));
        if (d >= max_dist) {
            max_vertex = vec;
            max_dist = d;
        }
    }

    mesh_radius = max_dist;

    std::cout << "Mesh Radius: " << mesh_radius << std::endl;

    // Compute triangle_normals
    triangle_normals.reserve(faces.size());
    for (int i = 0; i < faces.size(); i++) {

        int v0i = faces[i][0];
        int v1i = faces[i][1];
        int v2i = faces[i][2];

        // Link vertex to triangle
        vertex_to_triangles_map.at(v0i).push_back(i);
        vertex_to_triangles_map.at(v1i).push_back(i);
        vertex_to_triangles_map.at(v2i).push_back(i);

        glm::vec3 v0 = vertices[v0i];
        glm::vec3 v1 = vertices[v1i];
        glm::vec3 v2 = vertices[v2i];
        glm::vec3 tn = glm::triangleNormal(v0, v1, v2);
        triangle_normals.emplace_back(tn);
    }

    // Compute vertex normals
    vertex_normals.reserve(vertices.size());
    for (int i = 0; i < vertices.size(); i++) {
        vector<int> triangle_indices = vertex_to_triangles_map[i];
        glm::vec3 v_sum = glm::vec3(0.0f);

        for (int j : triangle_indices) {
            v_sum = v_sum + triangle_normals[j];
        }
        v_sum = glm::normalize(v_sum);
        vertex_normals.emplace_back(v_sum);
    }
}

void Mesh::readFileData(ifstream &mesh_file) {
    string line;
    bool has_read_counts = false;
    int num_vertices = 0;
    int num_faces = 0;
    int num_edges = 0;
    int current_vertex = 0;
    int current_face = 0;
    size_t sz;
    while (getline(mesh_file, line)) {
        sz = 0;
        if (isIgnorableLineInFile(line)) {
            continue;
        } else {
            if (!has_read_counts) {
                num_vertices = stoi(line, &sz);
                line = line.substr(sz);
                num_faces = stoi(line, &sz);
                line = line.substr(sz);
                num_edges = stoi(line, &sz);
                has_read_counts = true;
                cout << "V: " << num_vertices << ", F: " << num_faces
                     << ", E: " << num_edges << endl;
                continue;
            } else if (current_vertex < num_vertices) {
                float x = stof(line, &sz);
                line = line.substr(sz);

                float y = stof(line, &sz);
                line = line.substr(sz);

                float z = stof(line, &sz);
                glm::vec3 v(x, y, z);

                vertices.push_back(v);

                // NOTE: not supported, using 0.0
                vertex_colors.push_back(glm::vec3(0.0, 0.0, 0.0));

                // Create a vector in the mapping
                vector<int> triangles;
                vertex_to_triangles_map.emplace(vertices.size() - 1, triangles);

                current_vertex += 1;
                continue;
            } else if (current_face < num_faces) {
                int num_vertices =
                    stoi(line, &sz); // Doing nothing with this for now
                sz++;
                line = line.substr(sz);

                int x = stoi(line, &sz);
                line = line.substr(sz);

                int y = stoi(line, &sz);
                line = line.substr(sz);

                int z = stoi(line, &sz);
                line = line.substr(sz);
                glm::vec3 f(x, y, z);
                faces.push_back(f);
                current_face += 1;
                continue;
            }
        }
    }
}

/*
Adapted From:
http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
*/
bool Mesh::loadObjFile(const char *filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Impossible to open the file !\n");
        return false;
    }

    glm::vec3 BLACK(0.0, 0.0, 0.0);
    glm::vec3 vertex_rgb_current_color = BLACK;

    // Keep reading lines in file
    while (1) {

        char lineHeader[128]; // NOTE: this might have to be made larger

        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; // EOF = End Of File. Quit the loop.

        // cout << "line: " << res << endl;
        // printf("%s", lineHeader);
        // else : parse lineHeader
        // process vertices
        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            vertices.push_back(vertex);
            // cout << "v: " << glm::to_string(vertex) << endl;

            // NOTE: not supported, using 0.0
            vertex_colors.push_back(glm::vec3(0.0, 0.0, 0.0));

            vector<int> triangles;
            vertex_to_triangles_map.emplace(vertices.size() - 1, triangles);

            // Process vts
        } else if (strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uvs.push_back(uv);
            // cout << "uv: " << glm::to_string(uv) << endl;
            // Process normals
        } else if (strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            vertex_normals.push_back(normal);
            // cout << "n: " << glm::to_string(normal) << endl;
            // Process faces
        } else if (strcmp(lineHeader, "usergb") == 0) {
            glm::vec3 color;
            fscanf(file, "%f %f %f\n", &color.x, &color.y, &color.z);
            vertex_rgb_current_color = color;
            // vertex_normals.push_back(normal);
            // Update current color
        } else if (strcmp(lineHeader, "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            int vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
                                 &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                                 &vertexIndex[1], &uvIndex[1], &normalIndex[1],
                                 &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9) {
                printf("File can't be read by our simple parser : ( Try "
                       "exporting with other options\n");
                return false;
            }
            glm::vec3 face(vertexIndex[0] - 1, vertexIndex[1] - 1,
                           vertexIndex[2] - 1);

            // Assign color override to vertex
            vertex_colors[face[0]] = vertex_rgb_current_color;
            vertex_colors[face[1]] = vertex_rgb_current_color;
            vertex_colors[face[2]] = vertex_rgb_current_color;

            faces.push_back(face);

            // glm::vec3 face_uv(uvIndex[0], uvIndex[1], uvIndex[2]);
            // faces_uv.push_back(face_uv);

            // TODO: get these from the file
            // normalIndices.push_back(normalIndex[0]);
            // normalIndices.push_back(normalIndex[1]);
            // normalIndices.push_back(normalIndex[2]);
        }
    }

    prepareVectors();
    setVectorsAndBuffers();

    return true;
}

bool Mesh::loadOffFile(string filename) {
    ifstream mesh_file(filename);

    if (mesh_file.is_open()) {
        readFileData(mesh_file);
        prepareVectors();
        mesh_file.close();
    }
    setVectorsAndBuffers();
    return true;
}

void Mesh::loadFromFile(string filename) {

    std::string::size_type idx = filename.rfind('.');
    std::string extension("");
    std::string OFF_EXT("off");
    std::string OBJ_EXT("obj");

    if (idx != std::string::npos) {
        extension = filename.substr(idx + 1);
        file_ext = extension;
    }

    cout << ("Loading mesh from file: " + filename) << " of type ." << extension
         << endl;

    if (extension == OFF_EXT) {
        loadOffFile(filename);
    } else if (extension == OBJ_EXT) {
        const char *c = filename.c_str();
        loadObjFile(c);
    } else {
        cout << "ERROR: Unsupported mesh extension: ." << extension << endl;
        exit(1);
    }

    has_loaded = true;
}

// Generates the world vertices list
vector<glm::vec3> Mesh::GetWorldVertices(const glm::mat4 &modelMatrix,
                                         const glm::mat4 &viewMatrix,
                                         const glm::mat4 &projectionMatrix) {
    vector<glm::vec3> result;
    result.reserve(vertices.size());
    std::cout << "ModelMatrix Local: " << glm::to_string(modelMatrix) << "\n";
    // result.reserve(vertices.size());
    for (int i = 0; i < vertices.size(); i++) {

        // result.emplace_back(vertices[i]);
        glm::vec3 vertex = vertices[i];
        glm::vec4 v_world = modelMatrix * //
                            glm::vec4(vertex.x, vertex.y, vertex.z, 1.0);
        glm::vec3 v_world_small(v_world.x, v_world.y, v_world.z);
        result.emplace_back(v_world_small);
    }
    return result;
}

glm::vec3 Mesh::GetWorldCenter(const glm::mat4 &modelMatrix) {
    glm::vec4 h_coords =
        modelMatrix * glm::vec4(center.x, center.y, center.z, 1.0);

    return glm::vec3(h_coords.x, h_coords.y, h_coords.z);
}

bool Mesh::DoesHit(const glm::vec3 ray, const glm::vec3 orig,
                   const glm::mat4 &modelMatrix, float scale_factor,
                   glm::vec3 center) {
    float t;
    glm::vec3 dir = ray;

    float t0, t1; // solutions for t if the ray intersects

    // geometric solution
    glm::vec3 L = center - orig;
    float radius2 = pow(mesh_radius * scale_factor, 2); // squared radius

    float tca = glm::dot(L, dir);
    if (tca < 0)
        return false;
    float d2 = glm::dot(L, L) - tca * tca;
    if (d2 > radius2)
        return false;
    float thc = sqrt(radius2 - d2);
    t0 = tca - thc;
    t1 = tca + thc;

    if (t0 > t1)
        std::swap(t0, t1);

    if (t0 < 0) {
        t0 = t1; // if t0 is negative, let's use t1 instead
        if (t0 < 0)
            return false; // both t0 and t1 are negative
    }

    t = t0;

    return true;
}

void Mesh::setVectorsAndBuffers() {
    for (glm::vec3 vert : vertices) {
        vertices_vec.push_back(vert.x); // x
        vertices_vec.push_back(vert.y); // y
        vertices_vec.push_back(vert.z); // z
    }

    for (glm::vec3 vc : vertex_colors) {
        vertex_colors_vec.push_back(vc.x); // x
        vertex_colors_vec.push_back(vc.y); // y
        vertex_colors_vec.push_back(vc.z); // z
    }

    for (glm::vec3 face : faces) {
        indices.push_back((unsigned int)(face.x));
        indices.push_back((unsigned int)(face.y));
        indices.push_back((unsigned int)(face.z));
    }

    for (glm::vec3 n : vertex_normals) {
        vertex_normals_vec.push_back(n.x);
        vertex_normals_vec.push_back(n.y);
        vertex_normals_vec.push_back(n.z);
    }

    // Initialize the VBO with the vertices data
    // A VBO is a data container that lives in the GPU memory
    VBO.init();
    VBO.updateWithVector(3, vertices_vec.size() / 3, vertices_vec);

    VBO_C.init();
    VBO_C.updateWithVector(3, vertex_colors_vec.size() / 3, vertex_colors_vec);

    // VBO for normals
    VBO_VN.init();
    VBO_VN.updateWithVector(3, vertex_normals_vec.size() / 3,
                            vertex_normals_vec);

    std::cout << "Mesh: " << id << std::endl;
    std::cout << "\tVertexVector: " << vertices_vec.size() << std::endl;
    std::cout << "\tVertexNormalsVector: " << vertex_normals_vec.size()
              << std::endl;
    std::cout << "\tIndicesVector: " << indices.size() << std::endl;
}