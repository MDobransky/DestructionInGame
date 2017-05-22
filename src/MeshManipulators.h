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

#include <btHACDCompoundShape.h>

#include <voro++/voro++.hh>

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
        typedef CGAL::Nef_polyhedron_3<Kernel> Nef_polyhedron;

        static irr::scene::IMesh *convertPolyToMesh(Polyhedron &poly);

        static irr::scene::IMesh *convertPolyToMesh(Nef_polyhedron &poly);

        static btCollisionShape *convertMesh(irr::scene::IMeshSceneNode *);

        static irr::scene::IMesh *convertMesh(voro::voronoicell &cell);

        //static irr::scene::IMesh *
        //  subtractMesh(irr::scene::IMesh *from, irr::scene::IMesh *what, irr::core::vector3df position);

        static Nef_polyhedron subtractMesh(Nef_polyhedron &nef, irr::scene::IMesh *what, irr::core::vector3df position);

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

        class CModifierExplode : public CGAL::Modifier_base<HalfedgeDS>
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

            // life cycle
            CModifierExplode(Halfedge_handle seed_halfedge, Halfedges& halfedges)
                    : m_seed_halfedge(seed_halfedge), m_halfedges(halfedges)
            {
            }
            ~CModifierExplode() {}

            void operator()( HalfedgeDS& hds)
            {
                Faces faces;
                Vertices vertices;
                std::list<Vertex_handle> ordered_vertices;
                std::map<Vertex_handle,int,handle_compare<Vertex_handle> > vertex_map;

                // traverse component and fill sets
                std::stack<Halfedge_handle> stack;
                stack.push(m_seed_halfedge);
                int vertex_index = 0;
                while(!stack.empty())
                {
                    // pop halfedge from queue
                    Halfedge_handle he = stack.top();
                    stack.pop();
                    m_halfedges.insert(he);

                    // let's see its incident face
                    if(!he->is_border())
                    {
                        Face_handle f = he->facet();
                        if(faces.find(f) == faces.end())
                            faces.insert(f);
                    }

                    // let's see its end vertex
                    Vertex_handle v = he->vertex();
                    if(vertices.find(v) == vertices.end())
                    {
                        vertices.insert(v);
                        ordered_vertices.push_back(v);
                        vertex_map[v] = vertex_index++;
                    }

                    // go and discover component
                    Halfedge_handle nhe = he->next();
                    Halfedge_handle ohe = he->opposite();
                    if(m_halfedges.find(nhe) == m_halfedges.end())
                        stack.push(nhe);
                    if(m_halfedges.find(ohe) == m_halfedges.end())
                        stack.push(ohe);
                }

                builder B(hds,true);
                B.begin_surface(3,1,6);

                // add vertices
                std::list<Vertex_handle>::iterator vit;
                for(vit = ordered_vertices.begin();
                    vit != ordered_vertices.end();
                    vit++)
                {
                    Vertex_handle v = *vit;
                    B.add_vertex(v->point());
                }

                // add facets
                Faces::iterator fit;
                for(fit = faces.begin();
                    fit != faces.end();
                    fit++)
                {
                    Face_handle f = *fit;
                    B.begin_facet();
                    F_circulator he = f->facet_begin();
                    F_circulator end = he;
                    CGAL_For_all(he,end)
                    {
                        Vertex_handle v = he->vertex();
                        B.add_vertex_to_facet(vertex_map[v]);
                    }
                    B.end_facet();
                }
                B.end_surface();
            }
        };

        class Split_polyhedron
        {
        public:
            typedef typename Polyhedron::HalfedgeDS HalfedgeDS;
            typedef typename Polyhedron::Halfedge_handle Halfedge_handle;
            typedef typename Polyhedron::Halfedge_iterator Halfedge_iterator;

            Split_polyhedron() {}
            ~Split_polyhedron() {}

            void run(Polyhedron &polyhedron, std::vector<Nef_polyhedron> &out)
            {
                std::set<Halfedge_handle, handle_compare<Halfedge_handle> > halfedges;
                Polyhedron::Halfedge_iterator he;
                for(he = polyhedron.halfedges_begin();
                    he != polyhedron.halfedges_end();
                    he++)
                {
                    if(halfedges.find(he) == halfedges.end())
                    {
                        // adds one component as polyhedron
                        CModifierExplode modifier(he,halfedges);
                        Polyhedron component;
                        component.delegate(modifier);
                        out.push_back(Nef_polyhedron(component));
                    }
                }
            }
        };

    };
/*
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
    };*/


}


#endif
