#include "MeshManipulators.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

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
