#include <misaki/logger.h>
#include <misaki/mesh.h>
#include <misaki/properties.h>

#include <fstream>
#include <iostream>

namespace misaki {

class OBJMesh final : public Mesh {
    static unsigned int to_uint(const std::string &str) {
        char *end_ptr       = nullptr;
        unsigned int result = (int) strtoul(str.c_str(), &end_ptr, 10);
        if (*end_ptr != '\0')
            Throw("Could not parse integer value \"{}\"", str);
        return result;
    }

    struct OBJVertex {
        uint32_t p  = (uint32_t) -1;
        uint32_t n  = (uint32_t) -1;
        uint32_t uv = (uint32_t) -1;

        inline OBJVertex() {}
        inline OBJVertex(const std::string &string) {
            std::vector<std::string> tokens =
                string::tokenize(string, "/", true);
            if (tokens.size() < 1 || tokens.size() > 3)
                Throw("Invalid vertex data: {}", string);
            p = to_uint(tokens[0]);
            if (tokens.size() >= 2 && !tokens[1].empty())
                uv = to_uint(tokens[1]);
            if (tokens.size() >= 3 && !tokens[2].empty())
                n = to_uint(tokens[2]);
        }
        inline bool operator==(const OBJVertex &v) const {
            return v.p == p && v.n == n && v.uv == uv;
        }
    };

    template <typename ArgumentType, typename ResultType>
    struct unary_function {
        using argument_type = ArgumentType;
        using result_type   = ResultType;
    };

    // Hash function for OBJVertex
    struct OBJVertexHash : unary_function<OBJVertex, size_t> {
        std::size_t operator()(const OBJVertex &v) const {
            size_t hash = std::hash<uint32_t>()(v.p);
            hash        = hash * 37 + std::hash<uint32_t>()(v.uv);
            hash        = hash * 37 + std::hash<uint32_t>()(v.n);
            return hash;
        }
    };

public:
    OBJMesh(const Properties &props) : Mesh(props) {
        bool filp_tex_coords = props.bool_("filp_tex_coords", true);
        auto fr              = get_file_resolver();
        fs::path file_path   = fr->resolve(props.string("filename"));
        m_name               = file_path.filename().string();
        auto fail            = [&](const char *descr, auto... args) {
            Throw(("Error while loading OBJ file \"{}\": " + std::string(descr))
                      .c_str(),
                  m_name, args...);
        };

        Log(Info, R"(Loading mesh from "{}")", m_name);
        if (!fs::exists(file_path)) {
            fail("file not found");
        }

        std::vector<Eigen::Vector3f> vertices;
        std::vector<Eigen::Vector3f> normals;
        std::vector<Eigen::Vector2f> texcoords;
        std::vector<uint32_t> triangles;
        std::vector<OBJVertex> obj_vertices;
        std::unordered_map<OBJVertex, uint32_t, OBJVertexHash> vertex_map;

        std::ifstream is(file_path.string());
        std::string line_str;
        while (std::getline(is, line_str)) {
            std::istringstream line(line_str);
            std::string prefix;
            line >> prefix;
            if (prefix == "v") {
                Eigen::Vector3f p;
                line >> p.x() >> p.y() >> p.z();
                p = m_to_world.apply_point(p);
                m_bbox.expand(p);
                vertices.emplace_back(p);
            } else if (prefix == "vt") {
                Eigen::Vector2f tc;
                line >> tc.x() >> tc.y();
                if (filp_tex_coords)
                    tc.y() = 1.f - tc.y();
                texcoords.emplace_back(tc);
            } else if (prefix == "vn") {
                Eigen::Vector3f n;
                line >> n.x() >> n.y() >> n.z();
                n = (m_to_world.apply_normal(n)).normalized();
                normals.push_back(n);
            } else if (prefix == "f") {
                std::string v1, v2, v3, v4;
                line >> v1 >> v2 >> v3 >> v4;
                OBJVertex verts[6];
                int n_vertices = 3;
                verts[0]       = OBJVertex(v1);
                verts[1]       = OBJVertex(v2);
                verts[2]       = OBJVertex(v3);

                if (!v4.empty()) {
                    /* This is a quad, split into two triangles */
                    verts[3]   = OBJVertex(v4);
                    verts[4]   = verts[0];
                    verts[5]   = verts[2];
                    n_vertices = 6;
                }
                /* Convert to an indexed vertex list */
                for (int i = 0; i < n_vertices; ++i) {
                    const OBJVertex &v = verts[i];
                    std::unordered_map<OBJVertex, uint32_t,
                                       OBJVertexHash>::const_iterator it =
                        vertex_map.find(v);
                    if (it == vertex_map.end()) {
                        vertex_map[v] = (uint32_t) obj_vertices.size();
                        triangles.emplace_back((uint32_t) obj_vertices.size());
                        obj_vertices.push_back(v);
                    } else {
                        triangles.emplace_back(it->second);
                    }
                }
            }
        }

        m_vertex_count    = static_cast<uint32_t>(obj_vertices.size());
        m_face_count      = static_cast<uint32_t>(triangles.size() / 3);
        m_normal_offset   = normals.empty() ? 0 : 3;
        m_texcoord_offset = texcoords.empty() ? 0 : 6;
        m_vertex_size     = 3 + 3 + 2;
        m_face_size       = 3;
        m_vertices        = std::unique_ptr<float[]>(
            new float[(m_vertex_count + 1) * m_vertex_size]);
        m_faces = std::unique_ptr<uint32_t[]>(
            new uint32_t[(m_face_count + 1) * m_face_size]);
        memcpy(m_faces.get(), triangles.data(),
               sizeof(uint32_t) * m_face_count * m_face_size);
        for (int i = 0; i < obj_vertices.size(); i++) {
            m_vertices[i * m_vertex_size + 0] =
                vertices[obj_vertices[i].p - 1].x();
            m_vertices[i * m_vertex_size + 1] =
                vertices[obj_vertices[i].p - 1].y();
            m_vertices[i * m_vertex_size + 2] =
                vertices[obj_vertices[i].p - 1].z();
            if (obj_vertices[i].n != -1) {
                m_vertices[i * m_vertex_size + 3] =
                    normals[obj_vertices[i].n - 1].x();
                m_vertices[i * m_vertex_size + 4] =
                    normals[obj_vertices[i].n - 1].y();
                m_vertices[i * m_vertex_size + 5] =
                    normals[obj_vertices[i].n - 1].z();
            } else if (m_normal_offset != 0) {
                m_vertices[i * m_vertex_size + 3] = 0;
                m_vertices[i * m_vertex_size + 4] = 0;
                m_vertices[i * m_vertex_size + 5] = 0;
            }
            if (obj_vertices[i].uv != -1) {
                m_vertices[i * m_vertex_size + 6] =
                    texcoords[obj_vertices[i].uv - 1].x();
                m_vertices[i * m_vertex_size + 7] =
                    texcoords[obj_vertices[i].uv - 1].y();
            } else if (m_texcoord_offset != 0) {
                m_vertices[i * m_vertex_size + 6] = 0;
                m_vertices[i * m_vertex_size + 7] = 0;
            }
        }
        Log(Info, R"("{}": read {} faces, {} vertices)", m_name, m_face_count,
            m_vertex_count);
        area_distr_build();
    }
    MSK_DECLARE_CLASS()
};

MSK_IMPLEMENT_CLASS(OBJMesh, Mesh)
MSK_INTERNAL_PLUGIN(OBJMesh, "obj")

} // namespace misaki