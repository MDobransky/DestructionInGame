#ifndef MESHMANIPULATORS_H
#define MESHMANIPULATORS_H

#define CGAL_DISABLE_ROUNDING_MATH_CHECK

#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Nef_polyhedron_3.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>

#include <voro++/voro++.hh>
#include <chrono>

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
    public:
        typedef CGAL::Exact_predicates_exact_constructions_kernel Kernel;
        typedef CGAL::Polyhedron_3<Kernel> Polyhedron;
        typedef Polyhedron::HalfedgeDS HalfedgeDS;
        typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron;

        static irr::scene::IMesh* convertPolyToMesh(Polyhedron& poly);
        static irr::scene::IMesh* convertPolyToMesh(Nef_polyhedron& poly);
        static btBvhTriangleMeshShape* convertMesh(irr::scene::IMeshSceneNode*);
        static irr::scene::IMesh* convertMesh(voro::voronoicell& cell);
        static irr::scene::IMesh* subtractMesh(irr::scene::IMesh* from, irr::scene::IMesh* what, irr::core::vector3df position);
        static Nef_polyhedron subtractMesh(Nef_polyhedron& nef, irr::scene::IMesh* what, irr::core::vector3df position);
        static Nef_polyhedron makeNefPolyhedron(irr::scene::IMesh*);

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
    class Timer
    {
    public:
        Timer() : beg_(clock_::now()) {}
        void reset() { beg_ = clock_::now(); }
        double elapsed() const {
            return std::chrono::duration_cast<second_>
                (clock_::now() - beg_).count(); }

    private:
        typedef std::chrono::high_resolution_clock clock_;
        typedef std::chrono::duration<double, std::ratio<1> > second_;
        std::chrono::time_point<clock_> beg_;
    };
}


#endif
