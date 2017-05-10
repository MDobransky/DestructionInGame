#ifndef MESHMANIPULATORS_H
#define MESHMANIPULATORS_H

#include <vector>

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <LinearMath/btAlignedObjectArray.h>
#include <LinearMath/btConvexHullComputer.h>
#include <LinearMath/btQuaternion.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
namespace gg
{
    namespace MeshManipulators
    {
        btBvhTriangleMeshShape* convertMesh(irr::scene::IMeshSceneNode*);
        btConvexHullShape* convertMeshToHull(irr::scene::IMeshSceneNode *);
        irr::scene::IMesh* subtractMesh(irr::scene::IMesh* from, irr::scene::IMesh* what, irr::core::vector3df position);

        template <class HDS>
        class PolyhedronBuilder : public CGAL::Modifier_base<HDS>
        {
        public:
            PolyhedronBuilder(irr::scene::IMesh* m, irr::core::vector3df pos = irr::core::vector3df(0,0,0))
                : m_mesh(m), m_position(pos) {}

            void operator()(HDS& hds)
            {
                CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);
                typedef typename HDS::Vertex Vertex;
                typedef typename Vertex::Point Point;

                uint n_faces = 0, n_vertices = 0;
                for (irr::u32 j = 0; j < m_mesh->getMeshBufferCount(); j++)
                {
                    IMeshBuffer* meshBuffer = m_mesh->getMeshBuffer(j);
                    n_faces += meshBuffer->getIndexCount()/3;
                    n_vertices += meshBuffer->getVertexCount();
                }

                B.begin_surface(n_vertices, n_faces);
                int v = 0;
                int used_vertices = 0;
                for (irr::u32 j = 0; j < m_mesh->getMeshBufferCount(); j++)
                {
                    IMeshBuffer* meshBuffer = m_mesh->getMeshBuffer(j);
                    S3DVertex* IVertices = (S3DVertex*)meshBuffer->getVertices();
                    u16* IIndices = meshBuffer->getIndices();
                    for(irr::u32 i = 0; i < meshBuffer->getVertexCount(); i++)
                    {
                        vector3df vertex = IVertices[i].Pos;
                        B.add_vertex( Point( vertex.X + m_position.X, vertex.Y + m_position.Y, vertex.Z + m_position.Z));
                        v++;
                    }
                    for (u32 i = 0; i < meshBuffer->getIndexCount(); i+=3)
                    {
                        B.begin_facet();
                        B.add_vertex_to_facet(IIndices[i]+used_vertices);
                        B.add_vertex_to_facet(IIndices[i+1]+used_vertices);
                        B.add_vertex_to_facet(IIndices[i+2]+used_vertices);
                        B.end_facet();
                    }
                    used_vertices = v;
                }
                B.end_surface();
            }

        private:
            irr::scene::IMesh* m_mesh;
            irr::core::vector3df m_position;
        };

    }

}


#endif
