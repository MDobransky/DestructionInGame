#include "ObjectCreator.h"

#include <iostream>


gg::ObjectCreator::ObjectCreator(IrrlichtDevice* irr) : irrDevice(irr)
{
}

gg::Object gg::ObjectCreator::createTerrain(std::vector<std::string>&& items)
{
    if(items.size() != 6)
    {
        std::cerr << "Terrain failed to load: not enought parameters\n";
        return Object();
    }

    IMesh* mesh = irrDevice->getSceneManager()->getMesh((media + items[0]).c_str());
    irrDevice->getSceneManager()->getMeshManipulator()->scale(mesh,core::vector3df(std::stod(items[3]),
                                                              std::stod(items[4]),std::stod(items[5])));
    IMeshSceneNode* node = irrDevice->getSceneManager()->addMeshSceneNode( mesh );
    node->setMaterialType(EMT_SOLID);
    node->setMaterialFlag(EMF_LIGHTING, 1);
    node->setMaterialTexture(0, irrDevice->getVideoDriver()->getTexture((media + items[1]).c_str()));

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(0,0,0));

    btDefaultMotionState *motionState = new btDefaultMotionState(transform);

    // Create the shape
    //TODO \/ for better hitboxes
    //https://studiofreya.com/game-maker/bullet-physics/from-irrlicht-mesh-to-bullet-physics-mesh/
    //btCollisionShape *shape = convertMesh(node);
    btVector3 HalfExtents(1*0.5, 1*0.5 , 1*0.5);
    btCollisionShape *shape = new btBoxShape(HalfExtents);

    const btScalar Mass = 0.0f;
    // Add mass
    btVector3 localInertia;
    shape->calculateLocalInertia(Mass, localInertia);

    // Create the rigid body object
    btRigidBody *rigidBody = new btRigidBody(Mass, motionState, shape, localInertia);

    // Store a pointer to the irrlicht node so we can update it later
    rigidBody->setUserPointer((void *)(node));

    //Object a(RigidBody);
    return std::move(Object(rigidBody));
}

gg::Object gg::ObjectCreator::createRigidBody(std::vector<std::string>&& items)
{
    Object a;
    return std::move(a);
}

gg::Object gg::ObjectCreator::createSolidGround(float, float)
{
    Object a;
    return std::move(a);
}

//https://studiofreya.com/game-maker/bullet-physics/from-irrlicht-mesh-to-bullet-physics-mesh/
btBvhTriangleMeshShape* gg::ObjectCreator::convertMesh(IMeshSceneNode * node)
{
    auto toBtVector = [ &](const vector3df& vec) -> btVector3
    {
        btVector3 bt( vec.X, vec.Y, vec.Z );
        return bt;
    };

    IMesh * mesh = node->getMesh();
    const size_t buffercount = mesh->getMeshBufferCount();

    // Save data here
            std::vector<irr::video::S3DVertex>    verticesList;
            std::vector<int>                  indicesList;

            for ( size_t i=0; i<buffercount; ++i )
            {
                // Current meshbuffer
                IMeshBuffer * buffer = mesh->getMeshBuffer( i );

                // EVT_STANDARD -> video::S3DVertex
                // EVT_2TCOORDS -> video::S3DVertex2TCoords
                // EVT_TANGENTS -> video::S3DVertexTangents
                const irr::video::E_VERTEX_TYPE vertexType      = buffer->getVertexType();

                // EIT_16BIT
                // EIT_32BIT
                const irr::video::E_INDEX_TYPE  indexType       = buffer->getIndexType();

                // Get working data
                const size_t numVerts       = buffer->getVertexCount();
                const size_t numInd         = buffer->getIndexCount();

                // Resize save buffers
                verticesList.resize( verticesList.size() + numVerts );
                indicesList.resize( indicesList.size() + numInd );

                void * vertices             = buffer->getVertices();
                void * indices              = buffer->getIndices();

                irr::video::S3DVertex           * standard      = reinterpret_cast<irr::video::S3DVertex*>( vertices );
                irr::video::S3DVertex2TCoords   * two2coords    = reinterpret_cast<irr::video::S3DVertex2TCoords*>( vertices );
                irr::video::S3DVertexTangents   * tangents      = reinterpret_cast<irr::video::S3DVertexTangents*>( vertices );

                int16_t * ind16     = reinterpret_cast<int16_t*>( indices );
                int32_t * ind32     = reinterpret_cast<int32_t*>( indices );

                for ( size_t v = 0; v < numVerts; ++v )
                {
                    irr::video::S3DVertex & vert = verticesList[ (verticesList.size() - numVerts) + v ];

                    switch ( vertexType )
                    {
                        case irr::video::EVT_STANDARD:
                            {
                                const auto & irrv = standard[ v ];

                                vert = irrv;
                            }
                            break;
                        case irr::video::EVT_2TCOORDS:
                            {
                                const auto & irrv = two2coords[ v ];
                                (void)irrv;

                                // Not implemented
                            }
                            //break;
                        case irr::video::EVT_TANGENTS:
                            {
                                const auto & irrv = tangents[ v ];
                                (void)irrv;

                                // Not implemented
                            }
                            //break;
                    }

                }

                for ( size_t n = 0; n < numInd; ++n )
                {
                    int & index = indicesList[ (indicesList.size() - numInd) + n ];

                    switch ( indexType )
                    {
                        case irr::video::EIT_16BIT:
                        {
                            index = ind16[ n ];
                        }
                            break;
                        case irr::video::EIT_32BIT:
                        {
                            index = ind32[ n ];
                        }
                            break;
                    }

                }

            }

            if ( ! verticesList.empty() && ! indicesList.empty() )
            {
                        // Working numbers
                        const size_t numIndices     = indicesList.size();

                        // Create triangles
                        btTriangleMesh * btmesh = new btTriangleMesh();

                        // Build btTriangleMesh
                        for ( size_t i=0; i<numIndices; i+=3 )
                        {
                            const btVector3 &A = toBtVector( verticesList[ indicesList[ i+0 ] ].Pos );
                            const btVector3 &B = toBtVector( verticesList[ indicesList[ i+1 ] ].Pos );
                            const btVector3 &C = toBtVector( verticesList[ indicesList[ i+2 ] ].Pos );

                            bool removeDuplicateVertices = true;
                            btmesh->addTriangle( A, B, C, removeDuplicateVertices );
                        }

                        return new btBvhTriangleMeshShape( btmesh, true );
            }
    return nullptr;
}












