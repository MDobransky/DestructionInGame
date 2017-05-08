#include "MeshManipulators.h"
#include "../cork/include/cork.h"
#include <iostream>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


namespace
{
    CorkTriMesh convertMeshToCork(irr::scene::IMesh* from, irr::core::vector3df position = irr::core::vector3df(0,0,0))
    {
        uint n_indices = 0, n_vertices = 0;
        for (irr::u32 j = 0; j < from->getMeshBufferCount(); j++)
        {
            IMeshBuffer* meshBuffer = from->getMeshBuffer(j);
            n_indices += meshBuffer->getIndexCount();
            n_vertices += meshBuffer->getVertexCount()*3;
        }
        uint* indices = new uint[n_indices];
        float* vertices = new float[n_vertices];
        int ind = 0, v = 0;

        int used_vertices = 0;
        for (irr::u32 j = 0; j < from->getMeshBufferCount(); j++)
        {
            IMeshBuffer* meshBuffer = from->getMeshBuffer(j);
            S3DVertex* IVertices = (S3DVertex*)meshBuffer->getVertices();
            u16* IIndices = meshBuffer->getIndices();
            for (u32 i = 0; i < meshBuffer->getIndexCount(); i++)
            {
                indices[ind] = IIndices[i]+used_vertices;
                ind++;
            }
            for(irr::u32 i = 0; i < meshBuffer->getVertexCount(); i++)
            {
                vector3df vertex = IVertices[i].Pos;
                vertices[v] = vertex.X + position.X;
                vertices[v+1] = vertex.Y + position.Y;
                vertices[v+2] = vertex.Z + position.Z;
                v += 3;
            }
            used_vertices = v;
        }

        CorkTriMesh mesh = {n_indices/3, n_vertices/3, indices, vertices};
        return mesh;
    }

    irr::scene::IMesh* convertMeshFromCork(CorkTriMesh* cork)
    {
        irr::scene::SMesh* mesh = new SMesh();
        irr::scene::SMeshBuffer *buf = 0;
        buf = new SMeshBuffer();
        mesh->addMeshBuffer(buf);
        buf->drop();
        buf->Vertices.reallocate(cork->n_vertices);
        buf->Vertices.set_used(cork->n_vertices);
        buf->Indices.reallocate(cork->n_triangles*3);
        buf->Indices.set_used(cork->n_triangles*3);
        for(int i = 0; i < cork->n_vertices; i++)
        {
            buf->Vertices[i] = S3DVertex(cork->vertices[i*3],
                                         cork->vertices[i*3+1],
                                         cork->vertices[i*3+2],
                                         cork->vertices[i*3],
                                         cork->vertices[i*3+1],
                                         cork->vertices[i*3],
                                         video::SColor(100,100,100,100), 0, 1);

        }
        for(int i = 0; i < cork->n_triangles*3; i++)
        {
            buf->Indices[i] = cork->triangles[i];
        }
        buf->recalculateBoundingBox();

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
    CorkTriMesh Cork_from = convertMeshToCork(from);
    CorkTriMesh Cork_what = convertMeshToCork(what, position);
    CorkTriMesh* Cork_result = new CorkTriMesh;
    computeDifference(Cork_from, Cork_what, Cork_result);
    IMesh* res = convertMeshFromCork(Cork_result);
    freeCorkTriMesh(Cork_result);

return res;
}
