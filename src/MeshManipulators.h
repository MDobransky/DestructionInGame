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
#include <CGAL/convex_decomposition_3.h>

#include <voro++/voro++.hh>
#include <btHACDCompoundShape.h>

#include <chrono>
#include <vector>

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
        typedef CGAL::Nef_polyhedron_3<Kernel, CGAL::SNC_indexed_items> Nef_polyhedron;

        static std::tuple<irr::scene::IMesh *, irr::core::vector3df> convertPolyToMesh(Polyhedron &poly);

        static std::tuple<irr::scene::IMesh *, irr::core::vector3df> convertPolyToMesh(Nef_polyhedron &poly);

        static btCollisionShape *convertMesh(IMesh* mesh);

        static btCollisionShape *convertMesh(irr::scene::IMeshSceneNode *node);

        static btCollisionShape *nefToShape(Nef_polyhedron &poly);

        static irr::scene::IMesh *convertMesh(voro::voronoicell &cell);

        static std::tuple<Nef_polyhedron, Nef_polyhedron> subtractMesh(Nef_polyhedron &nef, irr::scene::IMesh *what, irr::core::vector3df position);

        static Nef_polyhedron makeNefPolyhedron(irr::scene::IMesh *);

        static std::vector<Nef_polyhedron> splitPolyhedron(Nef_polyhedron poly);

    private:
        class PolyhedronBuilder : public CGAL::Modifier_base<HalfedgeDS>
        {
        public:
            PolyhedronBuilder(irr::scene::IMesh *m, irr::core::vector3df pos = irr::core::vector3df(0, 0, 0)) : m_mesh(
                    m), m_position(pos)
            {}

            void operator()(HalfedgeDS &hds);

        private:
            irr::scene::IMesh *m_mesh;
            irr::core::vector3df m_position;
        };

        template<class Handle>
        class handle_compare
        {
        public:
            bool operator()(const Handle& h1, const Handle& h2) const
            {
                return &(*h1) < &(*h2);
            }
        };

        class SplitModifier : public CGAL::Modifier_base<HalfedgeDS>
        {
        private:
            typedef typename Kernel::Triangle_3 Triangle;
            typedef typename HalfedgeDS::Face_handle Face_handle;
            typedef typename HalfedgeDS::Vertex_handle Vertex_handle;
            typedef typename HalfedgeDS::Halfedge_handle Halfedge_handle;
            typedef typename Polyhedron::Halfedge_around_facet_circulator F_circulator;
            typedef typename CGAL::Polyhedron_incremental_builder_3<HalfedgeDS> builder;

            typedef typename std::set<Face_handle, handle_compare<Face_handle> > Faces;
            typedef typename std::set<Vertex_handle, handle_compare<Vertex_handle> > Vertices;
            typedef typename std::set<Halfedge_handle, handle_compare<Halfedge_handle> > Halfedges;

            Halfedge_handle m_seed_halfedge;
            Halfedges& m_halfedges;


        public:
            SplitModifier(Halfedge_handle seed_halfedge, Halfedges& halfedges)
                    : m_seed_halfedge(seed_halfedge), m_halfedges(halfedges)
            {
            }
            ~SplitModifier() {}

            void operator()( HalfedgeDS& hds);
        };

        class PolyhedronSplitter
        {
        public:
            typedef typename Polyhedron::HalfedgeDS HalfedgeDS;
            typedef typename Polyhedron::Halfedge_handle Halfedge_handle;
            typedef typename Polyhedron::Halfedge_iterator Halfedge_iterator;

            PolyhedronSplitter() {}
            ~PolyhedronSplitter() {}

            std::vector<Nef_polyhedron> run(Polyhedron &polyhedron);
        };

    };

    class Timer
    {
    public:
        Timer() : beg_(clock_::now())
        {}

        void reset()
        { beg_ = clock_::now(); }

        double elapsed() const
        {
            return std::chrono::duration_cast<second_>(clock_::now() - beg_).count();
        }

    private:
        typedef std::chrono::high_resolution_clock clock_;
        typedef std::chrono::duration<double, std::ratio<1> > second_;
        std::chrono::time_point<clock_> beg_;
    };


}


#endif
