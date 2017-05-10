#include "MeshManipulators.h"
#include <CGAL/number_utils.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>
#include <CGAL/Polyhedron_items_with_id_3.h>
#include <CGAL/Inverse_index.h>
#include <CGAL/Polygon_mesh_processing/self_intersections.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace CGAL;

namespace
{
    irr::scene::IMesh* convertPolyToMesh(Polyhedron_3<Exact_predicates_exact_constructions_kernel> &poly)
    {
        typedef Polyhedron_3<Exact_predicates_exact_constructions_kernel> Polyhedron;
        Polygon_mesh_processing::triangulate_faces(poly);

        irr::scene::SMesh* mesh = new SMesh();
        irr::scene::SMeshBuffer *buf = 0;
        buf = new SMeshBuffer();
        mesh->addMeshBuffer(buf);
        buf->drop();
        buf->Vertices.reallocate(poly.size_of_vertices());
        buf->Vertices.set_used(poly.size_of_vertices());
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

btConvexHullShape* gg::MeshManipulators::convertMeshToHull(IMeshSceneNode * node)
{
    btConvexHullShape* hull= new btConvexHullShape();
    for (irr::u32 j = 0; j < node->getMesh()->getMeshBufferCount(); j++)
    {
        IMeshBuffer* meshBuffer = node->getMesh()->getMeshBuffer(j);
        S3DVertex* vertices = (S3DVertex*)meshBuffer->getVertices();
        for(irr::u32 i = 0; i < meshBuffer->getVertexCount(); i++)
        {
            vector3df vertex = vertices[i].Pos;
            hull->addPoint(btVector3(vertex.X, vertex.Y, vertex.Z));
        }
    }
    return hull;
}

IMesh *gg::MeshManipulators::subtractMesh(IMesh *from, IMesh *what, vector3df position)
{
    typedef CGAL::Exact_predicates_exact_constructions_kernel     Kernel;
    typedef CGAL::Polyhedron_3<Kernel>  Polyhedron;
    typedef Polyhedron::HalfedgeDS             HalfedgeDS;
    typedef CGAL::Nef_polyhedron_3<Kernel>     Nef_polyhedron;

    Polyhedron poly_from;
    PolyhedronBuilder<HalfedgeDS> mesh_from(from);
    poly_from.delegate(mesh_from);

    Polyhedron poly_what;
    PolyhedronBuilder<HalfedgeDS> mesh_what(what, position);
    poly_what.delegate(mesh_what);

    if(Polygon_mesh_processing::does_self_intersect(poly_from)) std::cout << "1\n";
    //if(poly_what.is_closed()) std::cout << "2\n";

    //if(poly_from.is_closed() && poly_what.is_closed())
    {
        //Nef_polyhedron N1(poly_from);
        //Nef_polyhedron N2(poly_what);
        //Nef_polyhedron N3(N1-N2);
        //Polyhedron res;
        //N2.convert_to_polyhedron(res);
        return convertPolyToMesh(poly_from);
    }
   // else return from;


return from;
}
