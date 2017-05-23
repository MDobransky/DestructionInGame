#include "MeshManipulators.h"
#include <CGAL/number_utils.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Inverse_index.h>
#include <map>
#include <iostream>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace CGAL;


IMesh *gg::MeshManipulators::convertPolyToMesh(gg::MeshManipulators::Polyhedron &poly)
{
    Polygon_mesh_processing::triangulate_faces(poly,
                                               CGAL::Polygon_mesh_processing::parameters::use_delaunay_triangulation(
                                                       true));

    if(poly.size_of_vertices() == 0)
    {
        return NULL;
    }

    irr::scene::SMesh *mesh = new SMesh();
    irr::scene::SMeshBuffer *buf = 0;
    buf = new SMeshBuffer();
    mesh->addMeshBuffer(buf);
    buf->drop();
    buf->Vertices.reallocate(poly.size_of_vertices() * 3);
    buf->Vertices.set_used(poly.size_of_vertices() * 3);
    buf->Indices.reallocate(poly.size_of_facets() * 3);
    buf->Indices.set_used(poly.size_of_facets() * 3);
    int i = 0;
    for(auto p = poly.points_begin(); p != poly.points_end(); p++)
    {
        double a = CGAL::to_double(p->x());
        double b = CGAL::to_double(p->y());
        double c = CGAL::to_double(p->z());
        buf->Vertices[i] = S3DVertex(a, b, c, a, b, c, video::SColor(255, rand() % 256, rand() % 256, rand() % 256), 0,
                                     0);
        i++;
    }
    typedef Polyhedron::Vertex_const_iterator VCI;
    typedef CGAL::Inverse_index<VCI> Index;
    Index index(poly.vertices_begin(), poly.vertices_end());
    i = 0;
    for(auto f = poly.facets_begin(); f != poly.facets_end(); f++)
    {
        auto hfc = f->facet_begin();
        CGAL_assertion(CGAL::circulator_size(hfc) == 3);
        for(int j = 0; j < 3; j++)
        {
            buf->Indices[i * 3 + j] = index[VCI(hfc->vertex())];
            hfc++;
        }
        i++;
    }
    //centralize mesh
    mesh->recalculateBoundingBox();
    vector3df center = mesh->getBoundingBox().getCenter();
    S3DVertex *vertices = (S3DVertex *) buf->getVertices();
    for(int i = 0; i < buf->getVertexCount(); i++)
    {
        vertices[i].Pos = vertices[i].Pos - center;
    }
    mesh->recalculateBoundingBox();
    return mesh;
}

IMesh *gg::MeshManipulators::convertPolyToMesh(gg::MeshManipulators::Nef_polyhedron &poly)
{
    Polyhedron res;
    poly.convert_to_polyhedron(res);
    return convertPolyToMesh(res);
}

btCollisionShape *gg::MeshManipulators::convertMesh(IMeshSceneNode *node)
{
    btTriangleMesh *btMesh = new btTriangleMesh();

    for(irr::u32 j = 0; j < node->getMesh()->getMeshBufferCount(); j++)
    {
        IMeshBuffer *meshBuffer = node->getMesh()->getMeshBuffer(j);
        S3DVertex *vertices = (S3DVertex *) meshBuffer->getVertices();
        u16 *indices = meshBuffer->getIndices();

        for(u32 i = 0; i < meshBuffer->getIndexCount(); i += 3)
        {
            vector3df point1 = vertices[indices[i]].Pos;
            vector3df point2 = vertices[indices[i + 1]].Pos;
            vector3df point3 = vertices[indices[i + 2]].Pos;
            btMesh->addTriangle(btVector3(point1.X, point1.Y, point1.Z), btVector3(point2.X, point2.Y, point2.Z),
                                btVector3(point3.X, point3.Y, point3.Z));
        }
    }

    btBvhTriangleMeshShape *sh = new btBvhTriangleMeshShape(btMesh, true);
    btCollisionShape * compound = new btHACDCompoundShape(sh);
    delete sh;
    return compound;
}

IMesh *gg::MeshManipulators::convertMesh(voro::voronoicell &cell)
{
    std::vector<double> vertices;
    std::vector<int> face_vertices;
    cell.vertices(vertices);
    cell.face_vertices(face_vertices);

    SMesh *mesh = new SMesh();
    SMeshBuffer *buf = 0;
    buf = new SMeshBuffer();
    mesh->addMeshBuffer(buf);
    buf->drop();
    buf->Vertices.reallocate(vertices.size());
    buf->Vertices.set_used(vertices.size());
    for(int i = 0; i < static_cast<int>(vertices.size()); i++)
    {
        buf->Vertices[i] = S3DVertex(float(vertices[i]), float(vertices[i + 1]), float(vertices[i + 2]),
                                     float(vertices[i]), float(vertices[i + 1]), float(vertices[i + 2]),
                                     video::SColor(255, rand() % 256, rand() % 256, rand() % 256), 0, 0);
    }
    buf->Indices.reallocate(face_vertices.size() * 10);

    int indices = 0;
    for(int i = 0; i < static_cast<int>(face_vertices.size());)
    {
        for(int j = 1; j < face_vertices[i] - 1; j++)
        {
            buf->Indices[indices] = face_vertices[i + 1] * 3;
            buf->Indices[indices + 1] = face_vertices[i + j + 2] * 3;
            buf->Indices[indices + 2] = face_vertices[i + j + 1] * 3;
            indices += 3;
        }
        i += face_vertices[i] + 1;
    }

    buf->Indices.set_used(indices);
    buf->recalculateBoundingBox();

    return mesh;
}
/*
IMesh *gg::MeshManipulators::subtractMesh(IMesh *from, IMesh *what, vector3df position)
{
    if(from && what)
    {
        Polyhedron poly_from;
        PolyhedronBuilder mesh_from(from);
        poly_from.delegate(mesh_from);

        Polyhedron poly_what;
        PolyhedronBuilder mesh_what(what, position);
        poly_what.delegate(mesh_what);

        if(poly_from.is_closed() && poly_what.is_closed())
        {
            Nef_polyhedron N1(poly_from);
            Nef_polyhedron N2(poly_what);
            Nef_polyhedron N3(N1 - N2);
            Polyhedron res;
            N3.convert_to_polyhedron(res);
            return convertPolyToMesh(res);
        }
        return from;
    }
    return NULL;
}
*/
gg::MeshManipulators::Nef_polyhedron
    gg::MeshManipulators::subtractMesh(gg::MeshManipulators::Nef_polyhedron &nef, IMesh *what, vector3df position)
{
    if(what)
    {
        Polyhedron poly_what;
        PolyhedronBuilder mesh_what(what, position);
        poly_what.delegate(mesh_what);

        if(poly_what.is_closed())
        {
            Nef_polyhedron N2(poly_what);
            Nef_polyhedron N3(nef - N2);
            return std::move(N3);
        }
    }
    return std::move(nef);
}

gg::MeshManipulators::Nef_polyhedron gg::MeshManipulators::makeNefPolyhedron(IMesh *obj)
{
    if(obj)
    {
        Polyhedron poly_mesh;
        PolyhedronBuilder mesh_build(obj);
        poly_mesh.delegate(mesh_build);
        Nef_polyhedron N(poly_mesh);
        return std::move(N);
    }
    return Nef_polyhedron();
}

std::vector<gg::MeshManipulators::Nef_polyhedron>
    gg::MeshManipulators::splitPolyhedron(gg::MeshManipulators::Nef_polyhedron poly)
{
    std::vector<MeshManipulators::Nef_polyhedron> splitPolies;
    Split_polyhedron splitter;
    Polyhedron p;
    poly.convert_to_polyhedron(p);
    splitter.run(p, splitPolies);
    return splitPolies;
}

void gg::MeshManipulators::PolyhedronBuilder::operator()(gg::MeshManipulators::HalfedgeDS &hds)
{
    CGAL::Polyhedron_incremental_builder_3<HalfedgeDS> B(hds, true);
    typedef typename HalfedgeDS::Vertex Vertex;
    typedef typename Vertex::Point Point;
    std::map<irr::core::vector3df, int> vertex_map;

    uint n_faces = 0, n_vertices = 0;
    for(irr::u32 j = 0; j < m_mesh->getMeshBufferCount(); j++)
    {
        IMeshBuffer *meshBuffer = m_mesh->getMeshBuffer(j);
        n_faces += meshBuffer->getIndexCount() / 3;
        n_vertices += meshBuffer->getVertexCount();
    }
    B.begin_surface(n_vertices, n_faces);
    int v = 0;
    for(irr::u32 j = 0; j < m_mesh->getMeshBufferCount(); j++)
    {
        IMeshBuffer *meshBuffer = m_mesh->getMeshBuffer(j);
        S3DVertex *IVertices = (S3DVertex *) meshBuffer->getVertices();
        u16 *IIndices = meshBuffer->getIndices();

        for(u32 i = 0; i < meshBuffer->getIndexCount(); i += 3)
        {
            vector3df vertexA = IVertices[IIndices[i]].Pos;
            if(vertex_map.find(vertexA) == vertex_map.end())
            {
                B.add_vertex(Point(vertexA.X + m_position.X, vertexA.Y + m_position.Y, vertexA.Z + m_position.Z));
                vertex_map[vertexA] = v;
                v++;
            }
            vector3df vertexB = IVertices[IIndices[i + 1]].Pos;
            if(vertex_map.find(vertexB) == vertex_map.end())
            {
                B.add_vertex(Point(vertexB.X + m_position.X, vertexB.Y + m_position.Y, vertexB.Z + m_position.Z));
                vertex_map[vertexB] = v;
                v++;
            }
            vector3df vertexC = IVertices[IIndices[i + 2]].Pos;
            if(vertex_map.find(vertexC) == vertex_map.end())
            {
                B.add_vertex(Point(vertexC.X + m_position.X, vertexC.Y + m_position.Y, vertexC.Z + m_position.Z));
                vertex_map[vertexC] = v;
                v++;
            }
            B.begin_facet();
            B.add_vertex_to_facet(vertex_map[vertexA]);
            B.add_vertex_to_facet(vertex_map[vertexB]);
            B.add_vertex_to_facet(vertex_map[vertexC]);
            B.end_facet();
        }
    }
    B.end_surface();
}

/*


*/
