#include "MeshManipulators.h"
#include <CGAL/number_utils.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Inverse_index.h>
#include <map>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace CGAL;

namespace
{
    IMesh* convertPolyToMesh(CGAL::Polyhedron_3<CGAL::Exact_predicates_exact_constructions_kernel> &poly)
    {
        typedef Polyhedron_3<Exact_predicates_exact_constructions_kernel> Polyhedron;
        Polygon_mesh_processing::triangulate_faces(poly);

        irr::scene::SMesh* mesh = new SMesh();
        irr::scene::SMeshBuffer *buf = 0;
        buf = new SMeshBuffer();
        mesh->addMeshBuffer(buf);
        buf->drop();
        buf->Vertices.reallocate(poly.size_of_vertices()*3);
        buf->Vertices.set_used(poly.size_of_vertices()*3);
        buf->Indices.reallocate(poly.size_of_facets()*3);
        buf->Indices.set_used(poly.size_of_facets()*3);
        int i = 0;
        for(auto p = poly.points_begin(); p != poly.points_end(); p++)
        {
            buf->Vertices[i] = S3DVertex(CGAL::to_double(p->x()),
                                         CGAL::to_double(p->y()),
                                         CGAL::to_double(p->z()),
                                         CGAL::to_double(p->x()),
                                         CGAL::to_double(p->y()),
                                         CGAL::to_double(p->z()),
                                         video::SColor(255,rand()%256,rand()%256,rand()%256), 0, 0);
            i++;
        }
        typedef Polyhedron::Vertex_const_iterator VCI;
        typedef CGAL::Inverse_index<VCI> Index;
        Index index(poly.vertices_begin(), poly.vertices_end());
        i = 0;
        for (auto f = poly.facets_begin(); f != poly.facets_end(); f++)
        {
            auto hfc = f->facet_begin();
            CGAL_assertion(CGAL::circulator_size(hfc) == 3);
            for (int j = 0; j < 3; j++) {
                buf->Indices[i*3 + j] = index[VCI(hfc->vertex())];
                hfc++;
            }
            i++;
        }
        return mesh;
    }
}

btBvhTriangleMeshShape* gg::MeshManipulators::convertMesh(IMeshSceneNode * node)
{
    btTriangleMesh* btMesh = new btTriangleMesh();

    for (irr::u32 j = 0; j < node->getMesh()->getMeshBufferCount(); j++)
    {
        IMeshBuffer* meshBuffer = node->getMesh()->getMeshBuffer(j);
        S3DVertex* vertices = (S3DVertex*)meshBuffer->getVertices();
        u16* indices = meshBuffer->getIndices();

        for (u32 i = 0; i < meshBuffer->getIndexCount(); i += 3)
        {
            vector3df point1 = vertices[indices[i]].Pos;
            vector3df point2 = vertices[indices[i + 1]].Pos;
            vector3df point3 = vertices[indices[i + 2]].Pos;
            btMesh->addTriangle(btVector3(point1.X, point1.Y, point1.Z),
                                btVector3(point2.X, point2.Y, point2.Z),
                                btVector3(point3.X, point3.Y, point3.Z));
        }
    }
    return new btBvhTriangleMeshShape( btMesh, true );
}

IMesh* gg::MeshManipulators::subtractMesh(IMesh *from, IMesh *what, vector3df position)
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
        Nef_polyhedron N3(N1-N2);
        Polyhedron res;
        N3.convert_to_polyhedron(res);
        return convertPolyToMesh(res);
    }
    return from;
}

void gg::MeshManipulators::PolyhedronBuilder::operator()(gg::MeshManipulators::HalfedgeDS &hds)
{
    CGAL::Polyhedron_incremental_builder_3<HalfedgeDS> B(hds, true);
    typedef typename HalfedgeDS::Vertex Vertex;
    typedef typename Vertex::Point Point;
    std::map<irr::core::vector3df,int> vertex_map;

    uint n_faces = 0, n_vertices = 0;
    for (irr::u32 j = 0; j < m_mesh->getMeshBufferCount(); j++)
    {
        IMeshBuffer* meshBuffer = m_mesh->getMeshBuffer(j);
        n_faces += meshBuffer->getIndexCount()/3;
        n_vertices += meshBuffer->getVertexCount();
    }

    B.begin_surface(n_vertices, n_faces);
    int v = 0;
    for (irr::u32 j = 0; j < m_mesh->getMeshBufferCount(); j++)
    {
        IMeshBuffer* meshBuffer = m_mesh->getMeshBuffer(j);
        S3DVertex* IVertices = (S3DVertex*)meshBuffer->getVertices();
        u16* IIndices = meshBuffer->getIndices();

        for (u32 i = 0; i < meshBuffer->getIndexCount(); i+=3)
        {
            vector3df vertexA = IVertices[IIndices[i]].Pos;
            if(vertex_map.find(vertexA) == vertex_map.end())
            {
                B.add_vertex( Point( vertexA.X + m_position.X, vertexA.Y + m_position.Y, vertexA.Z + m_position.Z));
                vertex_map[vertexA] = v;
                v++;
            }
            vector3df vertexB = IVertices[IIndices[i+1]].Pos;
            if(vertex_map.find(vertexB) == vertex_map.end())
            {
                B.add_vertex( Point( vertexB.X + m_position.X, vertexB.Y + m_position.Y, vertexB.Z + m_position.Z));
                vertex_map[vertexB] = v;
                v++;
            }
            vector3df vertexC = IVertices[IIndices[i+2]].Pos;
            if(vertex_map.find(vertexC) == vertex_map.end())
            {
                B.add_vertex( Point( vertexC.X + m_position.X, vertexC.Y + m_position.Y, vertexC.Z + m_position.Z));
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
