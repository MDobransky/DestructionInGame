#ifndef MESHMANIPULATORS_H
#define MESHMANIPULATORS_H

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

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
    class MeshManipulators
    {
        typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
        typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
        typedef Polyhedron::HalfedgeDS HalfedgeDS;
        typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron;

    public:
        static btBvhTriangleMeshShape* convertMesh(irr::scene::IMeshSceneNode*);
        static irr::scene::IMesh* subtractMesh(irr::scene::IMesh* from, irr::scene::IMesh* what, irr::core::vector3df position);

    private:
        class PolyhedronBuilder : public CGAL::Modifier_base<HalfedgeDS>
        {
        public:
            PolyhedronBuilder(irr::scene::IMesh* m, irr::core::vector3df pos = irr::core::vector3df(0,0,0))
                : m_mesh(m), m_position(pos) {}
            void operator()(HalfedgeDS& hds);
        private:
            irr::scene::IMesh* m_mesh;
            irr::core::vector3df m_position;
        };
    };
}


#endif
