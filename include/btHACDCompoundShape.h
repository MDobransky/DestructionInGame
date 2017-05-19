/*
Copyright (c) 2011 Filippo Mariani

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

// Requirement: HACD library available at: http://sourceforge.net/projects/hacd/
// The best documentation available about HACD parameters can be found here:
// http://kmamou.blogspot.com/


#ifndef BTHACDCOMPOUNDSHAPE_H__
#define BTHACDCOMPOUNDSHAPE_H__
#pragma once


#include <hacdHACD.h>

#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <BulletCollision/CollisionShapes/btStridingMeshInterface.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btBvhTriangleMeshShape.h>
#include <BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.h>
#include <BulletCollision/Gimpact/btGImpactShape.h>
#include <LinearMath/btGeometryUtil.h>
#include <LinearMath/btSerializer.h>
#include <ctime>
#include <vector>
#include <algorithm>

#pragma region class  btHACDCompoundShape : public btCompoundShape
// Does NOT delete child shape on exiting (user may want to reuse them for other shapes, like when making a btCompoundShape out of uniform scaled children of this shape)
ATTRIBUTE_ALIGNED16(class)  btHACDCompoundShape : public btCompoundShape
{
public:

struct Params {
	// ORIGINAL PARAMS FROM CURRENT HACD SVN
	size_t nClusters;				// (1) Min number of resulting sub shapes. Can be used to increase the number of sub shapes.
	size_t maxHullVertices;			// (16) Max number of vertices in each convex hull sub shape. Can be 0 (= no limit).
	double concavity;				// (200) Higher seems to reduce the number of sub shapes and viceversa. Can be used to increase or decrease the number of sub shapes.
	bool addExtraDistPoints;		// (true) Specifies whether extra points should be added when computing the concavity. Meshes with holes may need this param to be true.
	//bool addNeighboursDistPoints;	// (REMOVED FROM LAST VERSION OF HACD) (false) Specifies whether extra points should be added when computing the concavity. Meshes with holes may need this param to be true.
	bool addFacesPoints;			// (true) Specifies whether faces points should be added when computing the concavity. Meshes with a small number of vertices or with holes may need this param to be true.   	
	
	double compacityWeight;			// (0.1) Sets the compacity weight (i.e. parameter alpha in ftp://ftp.elet.polimi.it/users/Stefano.Tubaro/ICIP_USB_Proceedings_v2/pdfs/0003501.pdf). = 0.1;
	double volumeWeight;			// (0.0) Sets the volume weight (i.e. parameter beta). = 0.0;
	std::string optionalVRMLSaveFilePath;	// (""). when set, it's supposed to save the decomposed mesh in VRML 2.0 format. Ignored if "keepSubmeshesSeparated" is true.
	double connectionDistance;		// (30.0) maximum distance to get CCs connected. Can be increased to decrease the number of convex hulls when increasing "concavity" has no effect. Very slow.	
	size_t targetNTrianglesDecimatedMesh; // (0) Target number of triangles in the decimated mesh. The decimation stage was added mainly to decrease the computation costs for dense meshes.
	bool displayDebugInfo;			// (false) With printf
	double scaleFactor;				// (1000.0) Normalization factor used to ensure that the other parameters (e.g. concavity) are expressed w.r.t. a fixed size. DO NOT USE IT TO SCALE YOUR MESH!
	double smallClusterThreshold;	// (0.25) Threshold on the clusters area (expressed as a percentage of the entire mesh area) under which the cluster is considered small and it is forced to be merged with other clusters at the price of a high concavity. 
	size_t heapManagerChunkSize;	// please ignore this. (65536*1000) Memory consumption of the J.Ratcliff micro allocator (default should be 32768 (32k), but in HACD test program 65536*1000 is used (64M)).


	// W.I.P.: ADDITIONAL PARAMS ADDED BY ME (TO BE TESTED)
        #define BTHACDCOMPOUNDSHAPE_H_USES_STDVECTOR_AS_VECTOR       // To check: without this definition, I got errors in some configurtions (but in another part of the code).
            #ifdef BTHACDCOMPOUNDSHAPE_H_USES_STDVECTOR_AS_VECTOR
            typedef std::vector< int > IntVector;
            #else //BTHACDCOMPOUNDSHAPE_H_USES_STDVECTOR_AS_VECTOR
            typedef btAlignedObjectArray< int > IntVector;            
            #endif //BTHACDCOMPOUNDSHAPE_H_USES_STDVECTOR_AS_VECTOR

        IntVector decomposeOnlySelectedSubmeshes;                   // if size()==0, decompose all submeshes
        bool keepSubmeshesSeparated;									// (false). True is slower, and must be used when submesh index is needed.
																// A btAlignedObjectArray< int > submeshIndexOfChildShapes can be retrieved from the class instance to map the child shape index vs the submesh index.
		IntVector keepSubmeshesSeparatedNClusters;				// Optional (used if size()>0 and keepSubmeshesSeparated==true). If any value is missing or <= 0, the global nClusters is used instead.
																// Ordering refers to the whole set of submeshes (so that it's independent on decomposeOnlySelectedSubmeshes).
		IntVector keepSubmeshesSeparatedMaxHullVertices;		// Optional (used if size()>0 and keepSubmeshesSeparated==true). If any value is missing or < 0, the global maxHullVertices is used instead.
																// Ordering refers to the whole set of submeshes (so that it's independent on decomposeOnlySelectedSubmeshes).														
	bool decomposeACleanCopyOfTheMesh;							// (true). Handles duplicated vertices and degenerate triangles. Slower.
	bool decomposeADecimatedCopyOfTheMesh;						// (false). (DEPRECATED: Use targetNTrianglesDecimatedMesh instead). True can be used to reduce the number of child shapes when increasing "concavity" and "connectionDist" does not help (but some detail gets lost). Slow? Maybe, but can speed up the decomposition process.
	float decimationDistanceInAabbHalfExtentsUnits;				// (0.1). In (0,1]. Bigger results in bigger decimation (= possibly less child shapes)
	bool decimationDistanceUniformInXYZ;						// (true). When true: DDx = DDy = DDz = decimationDistanceInHalfAabbExtentsUnits * min(AabbHalfExtents)xyz;
																//		   When false:  DDx = decimationDistanceInHalfAabbExtentsUnits * AabbHalfExtents.x;	
																//						DDy = decimationDistanceInHalfAabbExtentsUnits * AabbHalfExtents.y;	
																//						DDz = decimationDistanceInHalfAabbExtentsUnits * AabbHalfExtents.z;	
	bool decomposeACenteredCopyOfTheMesh;						// (false). The mesh center is always calculated keeping into account all the subparts of the whole btStridingMeshInterface.
	btVector3 decomposeATranslatedCopyOfTheMesh;				// btVector3(0,0,0). In unscaled units. The center of mass will be shifted in the opposite way.
	btVector3 decomposeAScaledCopyOfTheMesh;					// btVector3(1,1,1).
        std::string optionalBCSSaveFilePath;						// (""). when set, the Bullet Collision Shape is saved as btCompoundShape to disk (*).
	float convexHullsCollisionMargin;							// (0.01f) 
	bool reduceHullVerticesUsingBtShapeHull;					// (false) when set, uses btShapeHull (implemented by John McCutchan) to simplify the original btConvexHullShapes (from HACD decomposition), so that the number of hull vertices in each child shape is less than 42. It should result in a lower number of vertices per hull (when this does not happen, the original btConvexHullShape is used).
	bool convexHullsEnablePolyhedralContactClipping;			// (false) better quality, slightly slower
	bool shrinkObjectInwardsToCompensateCollisionMargin;		// (false)	// Based on the code in appConvexDecompositionDemo. Slow. And seems to produce artifacts...

	// (*) To load a btCollisionShape saved to disk:
	/*
	#include <BulletWorldImporter/btBulletWorldImporter.h>		// In "Bullet/Extra/Serialize". Needs link to: BulletWorldImporter.lib
	btCollisionShape* Load(const char* filename,bool verbose)	{	
		btBulletWorldImporter loader(0);//don't store info into the world
		loader.setVerboseMode(verbose);
		if (!loader.loadFile(filename)) return NULL;
	
		btCollisionShape* shape = NULL;
		if (loader.getNumCollisionShapes()>0) shape = loader.getCollisionShapeByIndex(0);
	
		//TODO: Cleaner way:
		// 1) Deep clone Collision Shape	
		// 2) loader.deleteAllData(); // deletes all (collision shapes included)
		// 3) return Deep cloned Collision Shape
	
		// Here we don't delete all data from the loader. (leaks?)
		return shape;	
	}	
	*/

	void reset() {*this = Params();}
	void resetForSpeed()	{
		reset();
		targetNTrianglesDecimatedMesh = 500;
		addFacesPoints = addExtraDistPoints = false;
		connectionDistance = 0;
		nClusters = 6;
		concavity = 300;
		maxHullVertices = 64;	// Not sure, but I suspect that greater values are faster (and 0 is the fastest): but in Bullet I can't afford it to be too high...
		decomposeACleanCopyOfTheMesh = false;
	}
	void resetWithHACDDefaults()	{
		reset();
		targetNTrianglesDecimatedMesh = 1000;
		addFacesPoints = addExtraDistPoints = true;
		maxHullVertices = 100;
		nClusters = 1;
		concavity = 100;
		compacityWeight = 0.0001;
		decomposeACleanCopyOfTheMesh = false;
	}
	// This setting should produce (sub-optimal but) good results in most cases:
	void resetForCommonUsage()	{
		reset();
		targetNTrianglesDecimatedMesh = 1000;
		addFacesPoints = addExtraDistPoints = true;
		connectionDistance = 30;
		nClusters = 6;
		concavity = 150;
		maxHullVertices = 16;	
		decomposeACleanCopyOfTheMesh = true;				
	}
	
	void display(bool modifiedParamsOnly=true)	{
		Params d;
		if (!modifiedParamsOnly || d.maxHullVertices!=maxHullVertices) 	printf("params.maxHullVertices	=				%1d\n",(int) maxHullVertices);
		if (!modifiedParamsOnly || d.concavity!=concavity) 				printf("params.concavity	=				%1.6f\n",concavity);
		if (!modifiedParamsOnly || d.nClusters!=nClusters) 				printf("params.nClusters	=				%1d\n",(int) nClusters);
		if (!modifiedParamsOnly || d.addExtraDistPoints!=addExtraDistPoints) {
			if (addExtraDistPoints) 									printf("params.addExtraDistPoints	=			true\n");
			else 														printf("params.addExtraDistPoints	=			false\n");
		}	
		/*
		if (!modifiedParamsOnly || d.addNeighboursDistPoints!=addNeighboursDistPoints) {
			if (addNeighboursDistPoints) 								printf("params.addNeighboursDistPoints	=			true\n");
			else 														printf("params.addNeighboursDistPoints	=			false\n");
		}
		*/	
		if (!modifiedParamsOnly || d.addFacesPoints!=addFacesPoints) {
			if (addFacesPoints) 										printf("params.addFacesPoints	=				true\n");
			else 														printf("params.addFacesPoints	=				false\n");
		}
		
		if (!modifiedParamsOnly || d.compacityWeight!=compacityWeight) 	printf("params.compacityWeight	=				%1.6f\n",compacityWeight);
		if (!modifiedParamsOnly || d.volumeWeight!=volumeWeight) 		printf("params.volumeWeight	=				%1.6f\n",volumeWeight);
		if (!modifiedParamsOnly || optionalVRMLSaveFilePath.size()>0) 				printf("params.optionalVRMLSaveFilePath	=			\"%s\"\n",optionalVRMLSaveFilePath.c_str());
		if (!modifiedParamsOnly || d.connectionDistance!=connectionDistance) 	printf("params.connectionDistance	=			%1.6f\n",connectionDistance);
		if (!modifiedParamsOnly || d.targetNTrianglesDecimatedMesh!=targetNTrianglesDecimatedMesh) 	printf("params.targetNTrianglesDecimatedMesh=			%d\n",(int)targetNTrianglesDecimatedMesh);		
		if (!modifiedParamsOnly || d.displayDebugInfo!=displayDebugInfo) {
			if (displayDebugInfo) 										printf("params.displayDebugInfo	=				true\n");
			else 														printf("params.displayDebugInfo	=				false\n");
		}		
		if (!modifiedParamsOnly || d.scaleFactor!=scaleFactor) 			printf("params.scaleFactor	=				%1.6f\n",scaleFactor);
		if (!modifiedParamsOnly || d.smallClusterThreshold!=smallClusterThreshold) 			printf("params.smallClusterThreshold	=			%1.6f\n",smallClusterThreshold);		
		if (!modifiedParamsOnly || d.heapManagerChunkSize!=heapManagerChunkSize) 	printf("params.heapManagerChunkSize	=			%d\n",(int)heapManagerChunkSize);		
		
		if (!modifiedParamsOnly || d.decomposeOnlySelectedSubmeshes.size()!=decomposeOnlySelectedSubmeshes.size()) 	{
			printf ("params.decomposeOnlySelectedSubmeshes	=		");
			if (decomposeOnlySelectedSubmeshes.size()==0) printf("No, thanks\n");
			else	{
				printf("(");
				for (int i=0,sz=decomposeOnlySelectedSubmeshes.size();i<sz;i++)	{
					printf("%1d",decomposeOnlySelectedSubmeshes[i]);
					if (i!=sz-1) printf(",");
				}
				printf(")\n");	
			}	
		}
		if (!modifiedParamsOnly || d.keepSubmeshesSeparated!=keepSubmeshesSeparated) {
			if (keepSubmeshesSeparated) 									printf("params.keepSubmeshesSeparated	=			true\n");
			else 														printf("params.keepSubmeshesSeparated	=			false\n");
		}		
		if (!modifiedParamsOnly || d.keepSubmeshesSeparatedNClusters.size()!=keepSubmeshesSeparatedNClusters.size()) 	{
			printf ("params.keepSubmeshesSeparatedNClusters	=		");
			if (keepSubmeshesSeparatedNClusters.size()==0) printf("No, thanks\n");
			else	{
				printf("(");
				for (int i=0,sz=keepSubmeshesSeparatedNClusters.size();i<sz;i++)	{
					printf("%1d",keepSubmeshesSeparatedNClusters[i]);
					if (i!=sz-1) printf(",");
				}
				printf(")\n");	
			}	
		}
		if (!modifiedParamsOnly || d.keepSubmeshesSeparatedMaxHullVertices.size()!=keepSubmeshesSeparatedMaxHullVertices.size()) 	{
			printf ("params.keepSubmeshesSeparatedMaxHullVertices	=		");
			if (keepSubmeshesSeparatedMaxHullVertices.size()==0) printf("No, thanks\n");
			else	{
				printf("(");
				for (int i=0,sz=keepSubmeshesSeparatedMaxHullVertices.size();i<sz;i++)	{
					printf("%1d",keepSubmeshesSeparatedMaxHullVertices[i]);
					if (i!=sz-1) printf(",");
				}
				printf(")\n");	
			}	
		}
		if (!modifiedParamsOnly || d.decomposeACleanCopyOfTheMesh!=decomposeACleanCopyOfTheMesh) {
			if (decomposeACleanCopyOfTheMesh) 							printf("params.decomposeACleanCopyOfTheMesh	=		true\n");
			else 														printf("params.decomposeACleanCopyOfTheMesh	=		false\n");
		}		
		if (!modifiedParamsOnly || d.decomposeADecimatedCopyOfTheMesh!=decomposeADecimatedCopyOfTheMesh) {
			if (decomposeADecimatedCopyOfTheMesh) 						printf("params.decomposeADecimatedCopyOfTheMesh	=		true\n");
			else 														printf("params.decomposeADecimatedCopyOfTheMesh	=		false\n");
		}		
		if (!modifiedParamsOnly || d.decimationDistanceInAabbHalfExtentsUnits!=decimationDistanceInAabbHalfExtentsUnits) 	
																		printf("params.decimationDistanceInAabbHalfExtentsUnits	=	%1.6f\n",decimationDistanceInAabbHalfExtentsUnits);		
		if (!modifiedParamsOnly || d.decimationDistanceUniformInXYZ!=decimationDistanceUniformInXYZ) {
			if (decimationDistanceUniformInXYZ) 						printf("params.decimationDistanceUniformInXYZ	=		true\n");
			else 														printf("params.decimationDistanceUniformInXYZ	=		false\n");
		}		
		if (!modifiedParamsOnly || d.decomposeACenteredCopyOfTheMesh!=decomposeACenteredCopyOfTheMesh) {
			if (decomposeACenteredCopyOfTheMesh) 						printf("params.decomposeACenteredCopyOfTheMesh	=		true\n");
			else 														printf("params.decomposeACenteredCopyOfTheMesh	=		false\n");
		}
		if (!modifiedParamsOnly || d.decomposeATranslatedCopyOfTheMesh!=decomposeATranslatedCopyOfTheMesh) 		
																		printf("params.decomposeATranslatedCopyOfTheMesh	=		(%1.3f,%1.3f,%1.3f)\n",decomposeATranslatedCopyOfTheMesh.x(),decomposeATranslatedCopyOfTheMesh.y(),decomposeATranslatedCopyOfTheMesh.z());
		if (!modifiedParamsOnly || d.decomposeAScaledCopyOfTheMesh!=decomposeAScaledCopyOfTheMesh) 		
																		printf("params.decomposeAScaledCopyOfTheMesh	=		(%1.3f,%1.3f,%1.3f)\n",decomposeAScaledCopyOfTheMesh.x(),decomposeAScaledCopyOfTheMesh.y(),decomposeAScaledCopyOfTheMesh.z());
		if (!modifiedParamsOnly || optionalBCSSaveFilePath.size()>0) 				printf("params.optionalBCSSaveFilePath	=			\"%s\"\n",optionalBCSSaveFilePath.c_str());
		if (!modifiedParamsOnly || d.convexHullsCollisionMargin!=convexHullsCollisionMargin) 		printf("params.convexHullsCollisionMargin	=		%1.3f\n",convexHullsCollisionMargin);
		if (!modifiedParamsOnly || d.reduceHullVerticesUsingBtShapeHull!=reduceHullVerticesUsingBtShapeHull) {
			if (reduceHullVerticesUsingBtShapeHull) 			printf("params.reduceHullVerticesUsingBtShapeHull	=			true\n");
			else 												printf("params.reduceHullVerticesUsingBtShapeHull	=			false\n");
		}	
		if (!modifiedParamsOnly || d.convexHullsEnablePolyhedralContactClipping!=convexHullsEnablePolyhedralContactClipping) {
			if (convexHullsEnablePolyhedralContactClipping) 			printf("params.convexHullsEnablePolyhedralContactClipping	=			true\n");
			else 														printf("params.convexHullsEnablePolyhedralContactClipping	=			false\n");
		}	
		if (!modifiedParamsOnly || d.shrinkObjectInwardsToCompensateCollisionMargin!=shrinkObjectInwardsToCompensateCollisionMargin) {
			if (shrinkObjectInwardsToCompensateCollisionMargin) 		printf("params.shrinkObjectInwardsToCompensateCollisionMargin=	true\n");
			else 														printf("params.shrinkObjectInwardsToCompensateCollisionMargin=	false\n");
		}												
	}
	
	//============
	// DEFAULTS:
	//============
	Params() : 	nClusters(							1),
				concavity(							200),
				maxHullVertices(					16),
				addExtraDistPoints(					false),
				//addNeighboursDistPoints(			false),
				addFacesPoints(						false),
				compacityWeight(					0.1),	
				volumeWeight(						0.0),
				optionalVRMLSaveFilePath(			""),
				targetNTrianglesDecimatedMesh(		0),//1000),
				displayDebugInfo(					false),
				scaleFactor(						1000.0),				
				connectionDistance(					30.0),				
				smallClusterThreshold(				0.25),
				heapManagerChunkSize(				65536*1000),//32768),//65536*1000),
				keepSubmeshesSeparated(				false),
				decomposeACleanCopyOfTheMesh(		true),
				decomposeADecimatedCopyOfTheMesh(	false),
				decimationDistanceInAabbHalfExtentsUnits(0.1f),
				decimationDistanceUniformInXYZ(		true),
				decomposeACenteredCopyOfTheMesh(	false),
				decomposeATranslatedCopyOfTheMesh(	btVector3(0,0,0)),
				decomposeAScaledCopyOfTheMesh(		btVector3(1,1,1)),
				optionalBCSSaveFilePath(			""),				
				convexHullsCollisionMargin(			0.01f),
				reduceHullVerticesUsingBtShapeHull(	false),
				convexHullsEnablePolyhedralContactClipping(false),
				shrinkObjectInwardsToCompensateCollisionMargin(false)
				{}
};

// Main ctr
btHACDCompoundShape(const btStridingMeshInterface* stridingMeshInterface,const Params& params=Params())	{
	init(stridingMeshInterface,params);
}
/* Creates a decomposed shape of the input collision shape that must be one of the following:
	btBvhTriangleMeshShape
	btScaledBvhTriangleMeshShape
	btGImpactShape
   btCompoundShapes with one nested compatible shape are not supported.
*/
btHACDCompoundShape(const btCollisionShape* shape,const btHACDCompoundShape::Params& HACDparams=btHACDCompoundShape::Params(),const bool considerShapeLocalScaling=true)	{
			if (!shape) return;
			const btStridingMeshInterface* smi(NULL);
			btHACDCompoundShape::Params params(HACDparams);
			if (shape->getShapeType()==GIMPACT_SHAPE_PROXYTYPE)	{
				const btGImpactShapeInterface* gimi = static_cast < const btGImpactShapeInterface* > (shape);
				if (gimi && gimi->getGImpactShapeType()==CONST_GIMPACT_TRIMESH_SHAPE)	{
					const btGImpactMeshShape* gishape = static_cast < const btGImpactMeshShape* > (gimi);
					smi = gishape->getMeshInterface();
					if (considerShapeLocalScaling) params.decomposeAScaledCopyOfTheMesh*=gishape->getLocalScaling();
				}
			}
			else if (shape->getShapeType()==TRIANGLE_MESH_SHAPE_PROXYTYPE)	{
				const btBvhTriangleMeshShape* bvhShape = static_cast < const btBvhTriangleMeshShape* > (shape);
					smi = bvhShape->getMeshInterface();
					if (considerShapeLocalScaling) params.decomposeAScaledCopyOfTheMesh*=bvhShape->getLocalScaling();
			}
			else if (shape->getShapeType()==SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE)	{
				const btScaledBvhTriangleMeshShape* sbvhShape = static_cast < const btScaledBvhTriangleMeshShape* > (shape);
				if (sbvhShape)	{
					const btBvhTriangleMeshShape* bvhShape = static_cast < const btBvhTriangleMeshShape* > (sbvhShape->getChildShape());
					if (bvhShape)	{
						smi = bvhShape->getMeshInterface();
						params.decomposeAScaledCopyOfTheMesh*=sbvhShape->getLocalScaling();		//This must be left outside 'considerShapeLocalScaling'
						if (considerShapeLocalScaling) params.decomposeAScaledCopyOfTheMesh*=bvhShape->getLocalScaling();
					}	
				}	
			}
			if (!smi) return;
			init(smi,params);
}
virtual ~btHACDCompoundShape() {}

int getSubmeshIndexOfChildShape(int childShape) const {
	return m_submeshIndexOfChildShapes.size()>childShape ? m_submeshIndexOfChildShapes[childShape] : -1;
}
const btAlignedObjectArray< int >& getSubmeshIndexOfChildShapesMap() const {return m_submeshIndexOfChildShapes;}
btAlignedObjectArray< int >& getSubmeshIndexOfChildShapesMap() {return m_submeshIndexOfChildShapes;}
void getSubmeshIndexOfChildShapesMapCopy(btAlignedObjectArray< int >& out) const {
	out.copyFromArray(m_submeshIndexOfChildShapes);
}

inline static btHACDCompoundShape* upcast(btCollisionShape* c)	{
	return dynamic_cast < btHACDCompoundShape* > (c);
}
inline static const btHACDCompoundShape* upcast(const btCollisionShape* c)	{
	return dynamic_cast < const btHACDCompoundShape* > (c);
}


protected:
//for easy inheriting:
btHACDCompoundShape() {}

// Main Method:
void init(const btStridingMeshInterface* stridingMeshInterface,const Params& params=Params()) {
	m_submeshIndexOfChildShapes.resize(0);
	if (!stridingMeshInterface) return;
	
	const std::clock_t ck1 = std::clock();
        btHACDCompoundShape::Params::IntVector decomposeOnlySelectedSubmeshes = params.decomposeOnlySelectedSubmeshes;
	if (decomposeOnlySelectedSubmeshes.size()) {
                #ifndef BTHACDCOMPOUNDSHAPE_H_USES_STDVECTOR_AS_VECTOR
                decomposeOnlySelectedSubmeshes.quickSort(Less());
                #else   //BTHACDCOMPOUNDSHAPE_H_USES_STDVECTOR_AS_VECTOR
		std::sort(decomposeOnlySelectedSubmeshes.begin(),decomposeOnlySelectedSubmeshes.end(),Less());
                //#undef BTHACDCOMPOUNDSHAPE_H_USES_STDVECTOR_AS_VECTOR      // No undef, even if it's no more needed...
                #endif  //BTHACDCOMPOUNDSHAPE_H_USES_STDVECTOR_AS_VECTOR
	}	

	const HACD::Vec3<HACD::Real> meshTranslation = HACD::Vec3<HACD::Real>(params.decomposeATranslatedCopyOfTheMesh.x(),params.decomposeATranslatedCopyOfTheMesh.y(),params.decomposeATranslatedCopyOfTheMesh.z());
	const bool mustTranslateMesh = (params.decomposeATranslatedCopyOfTheMesh.x()!=0 || params.decomposeATranslatedCopyOfTheMesh.y()!=0 || params.decomposeATranslatedCopyOfTheMesh.z()!=0);
	const HACD::Vec3<HACD::Real> meshScalingFactor = HACD::Vec3<HACD::Real>(params.decomposeAScaledCopyOfTheMesh.x(),params.decomposeAScaledCopyOfTheMesh.y(),params.decomposeAScaledCopyOfTheMesh.z());
	const bool mustScaleMesh = (params.decomposeAScaledCopyOfTheMesh.x()!=1 || params.decomposeAScaledCopyOfTheMesh.y()!=1 || params.decomposeAScaledCopyOfTheMesh.z()!=1);
	HACD::Vec3<HACD::Real> aabbHalfExtents;
	HACD::Vec3<HACD::Real> aabbCenterPoint;
	if (params.decomposeADecimatedCopyOfTheMesh || params.decomposeACenteredCopyOfTheMesh)	{
		aabbHalfExtents = GetAabbHalfExtentsFromStridingInterface(stridingMeshInterface,&aabbCenterPoint);//,meshScalingFactor);
		aabbHalfExtents = aabbHalfExtents*meshScalingFactor;
	}

	HACD::Vec3<HACD::Real> decimationDistance(SIMD_EPSILON,SIMD_EPSILON,SIMD_EPSILON);
	if (params.decomposeADecimatedCopyOfTheMesh)	{
		if (params.decimationDistanceUniformInXYZ) {
			const HACD::Real DD = params.decimationDistanceInAabbHalfExtentsUnits * (
																					(aabbHalfExtents.X() < aabbHalfExtents.Y() && aabbHalfExtents.X() < aabbHalfExtents.Z()) ? aabbHalfExtents.X() :
																					(aabbHalfExtents.Y() < aabbHalfExtents.X() && aabbHalfExtents.Y() < aabbHalfExtents.Z()) ? aabbHalfExtents.Y() :
																					 aabbHalfExtents.Z()
																					 );
			decimationDistance = HACD::Vec3<HACD::Real>(DD,DD,DD);																					 
		}																					 
		else decimationDistance = HACD::Vec3<HACD::Real>(params.decimationDistanceInAabbHalfExtentsUnits*aabbHalfExtents.X(),params.decimationDistanceInAabbHalfExtentsUnits*aabbHalfExtents.Y(),params.decimationDistanceInAabbHalfExtentsUnits*aabbHalfExtents.Z());																															    	
	}	
	

	if (params.displayDebugInfo) printf("NumSubmeshes: %1d\n",stridingMeshInterface->getNumSubParts());
	const bool forceDegenerateTrianglesRemovalWhenACleanCopyOfTheMeshMustBeProcessed = true;	// Yes, much better
	
	
	std::vector< HACD::Vec3<HACD::Real> > points;
	std::vector< HACD::Vec3<long> > triangles;	
		
	if (!params.keepSubmeshesSeparated)	{
		GetStridingInterfaceContent(stridingMeshInterface,points,triangles,decomposeOnlySelectedSubmeshes.size()>0 ? &decomposeOnlySelectedSubmeshes : NULL);
		if (params.displayDebugInfo) printf("Numverts: %1d NumTriangles: %1d (from btStridingMeshInterface)\n",(int) points.size(),(int)triangles.size());	
		if (params.decomposeACenteredCopyOfTheMesh) ShiftPoints(points,-aabbCenterPoint);
		if (mustTranslateMesh) ShiftPoints(points,meshTranslation);
		if (mustScaleMesh) ScalePoints(points,meshScalingFactor);

		if (params.decomposeADecimatedCopyOfTheMesh || params.decomposeACleanCopyOfTheMesh)	{
			const bool removeDegenerateTrianglesToo = params.decomposeADecimatedCopyOfTheMesh | forceDegenerateTrianglesRemovalWhenACleanCopyOfTheMeshMustBeProcessed; // Usually when removing double vertices, degenerate triangles are less prone to appear...
			RemoveDoubleVertices(points,triangles,removeDegenerateTrianglesToo,decimationDistance);	// This same method is used to decimate the mesh too.
			if (params.displayDebugInfo)	{
				if (params.decomposeADecimatedCopyOfTheMesh) printf("Numverts: %1d NumTriangles: %1d (after mesh decimation)\n",(int) points.size(),(int)triangles.size());	
				else if (params.decomposeACleanCopyOfTheMesh)  printf("Numverts: %1d NumTriangles: %1d (after duplicated vertices removal)\n",(int) points.size(),(int)triangles.size());	
			}	
		}

		performHACDMainWork(params,points,triangles);
	}
	else	{
		const int subParts = stridingMeshInterface->getNumSubParts();
		int curSelSm = -1;
		int curSelSmIdx = 0;	
		const bool mustSelectSubMeshes = (decomposeOnlySelectedSubmeshes.size()>0);			
		if (mustSelectSubMeshes) curSelSm = decomposeOnlySelectedSubmeshes[curSelSmIdx];
		std::vector< HACD::Vec3<HACD::Real> > points;
		std::vector< HACD::Vec3<long> > triangles;			
		for (int subPart = 0;subPart<subParts;subPart++)	{
			if (mustSelectSubMeshes)	{
				if (curSelSm!=subPart) continue;
				if (decomposeOnlySelectedSubmeshes.size()<=curSelSmIdx+1) curSelSm = subParts;	// This will never process successive cycles
				else curSelSm =  decomposeOnlySelectedSubmeshes[++curSelSmIdx];					// Ready for next cycle	
			}
			points.clear();
			triangles.clear();
			GetStridingInterfaceContentOfSingleSubmesh(stridingMeshInterface,points,triangles,subPart);
		
			if (params.displayDebugInfo) {
				printf("Subpart: %1d. Numverts: %1d NumTriangles: %1d (From btStridingMeshInterface)\n",subPart,(int) points.size(),(int)triangles.size());	
			}
			
			if (params.decomposeACenteredCopyOfTheMesh) ShiftPoints(points,-aabbCenterPoint);
			if (mustTranslateMesh) ShiftPoints(points,meshTranslation);
			if (mustScaleMesh) ScalePoints(points,meshScalingFactor);
			
			
			if (params.decomposeADecimatedCopyOfTheMesh || params.decomposeACleanCopyOfTheMesh)	{
				const bool removeDegenerateTrianglesToo = params.decomposeADecimatedCopyOfTheMesh | forceDegenerateTrianglesRemovalWhenACleanCopyOfTheMeshMustBeProcessed; // Usually when removing double vertices, degenerate triangles are less prone to appear...
				RemoveDoubleVertices(points,triangles,removeDegenerateTrianglesToo,decimationDistance);	// This same method is used to decimate the mesh too.
				if (params.displayDebugInfo)	{
					if (params.decomposeADecimatedCopyOfTheMesh) printf("Subpart: %1d. Numverts: %1d NumTriangles: %1d (after mesh decimation)\n",subPart,(int) points.size(),(int)triangles.size());	
					else if (params.decomposeACleanCopyOfTheMesh)  printf("Subpart: %1d. Numverts: %1d NumTriangles: %1d (after duplicated vertices removal)\n",subPart,(int) points.size(),(int)triangles.size());	
				}
			}
	
			performHACDMainWork(params,points,triangles,subPart);
		}			
		for (int i=0,sz=m_submeshIndexOfChildShapes.size();i<sz;i++)	{
			printf("submeshIndexOfChildShapes[%1d]	=	%1d;\n",i,m_submeshIndexOfChildShapes[i]);		
		}
	}	
	
	if (params.optionalBCSSaveFilePath.size()>0 && this->getNumChildShapes()>0)	{
		 	btDefaultSerializer serializer;
 			serializer.startSerialization();
			this->serializeSingleShape(&serializer);
			serializer.finishSerialization();  	
			FILE* file = fopen(params.optionalBCSSaveFilePath.c_str(),"wb");
			if (file)	{
				fwrite(serializer.getBufferPointer(),serializer.getCurrentBufferSize(),1, file);
   				fclose(file);
   				if (params.displayDebugInfo) printf("btCompoundShape saved to: \"%s\"\n",params.optionalBCSSaveFilePath.c_str());
   			}
   			else {	
   				 printf("ERROR: I can't save the btCompoundShape to: \"%s\"\n",params.optionalBCSSaveFilePath.c_str());
   			}
			
	}
	const std::clock_t ck2 = std::clock();
	if (params.displayDebugInfo) printf("Operation performed in %1.2f seconds.\n",((float)(ck2-ck1))*0.001);
}


struct HACDCreatorWrapper{
	protected:
	HACD::HeapManager * heapManager;
	HACD::HACD* hacd;
	typedef HACD::HACD& HACDREF;
	size_t heapManagerChunkSize;
	
	public:
	HACDCreatorWrapper(size_t _heapManagerChunkSize=32768) : heapManagerChunkSize(_heapManagerChunkSize),heapManager(NULL),hacd(NULL) {
		reset();
	}
	void reset()	{
		//printf("START HACDCreatorWrapper::reset()\n");
		if (hacd) {HACD::DestroyHACD(hacd);hacd=NULL;}
		//#define REUSE_EXISTING_HEAPMANAGER
		#ifdef REUSE_EXISTING_HEAPMANAGER
		if (!heapManager)
		#undef REUSE_EXISTING_HEAPMANAGER
		#else //REUSE_EXISTING_HEAPMANAGER
		if (heapManager) {HACD::releaseHeapManager(heapManager);heapManager=NULL;}		
		#endif //REUSE_EXISTING_HEAPMANAGER	
		heapManager = HACD::createHeapManager(heapManagerChunkSize);
		if (heapManager) hacd = HACD::CreateHACD(heapManager);
		//printf("END HACDCreatorWrapper::reset()\n");

	}
	bool isOK() const {return hacd!=NULL;}
	inline operator HACDREF() const {return *hacd;}	// Test isOK() before calling this 
	~HACDCreatorWrapper()	{
		//printf("START ~HACDCreatorWrapper()\n");
		if (hacd)  {HACD::DestroyHACD(hacd);hacd=NULL;}
		if (heapManager) {HACD::releaseHeapManager(heapManager);heapManager=NULL;}
		//printf("END ~HACDCreatorWrapper()\n");
	}
};

void performHACDMainWork(const Params& params,std::vector< HACD::Vec3<HACD::Real> >& points,std::vector< HACD::Vec3<long> >& triangles,const int subPart=-1)	{		
		HACDCreatorWrapper hacdWrapper(params.heapManagerChunkSize);
		if (!hacdWrapper.isOK()) {printf("CRITICAL ERROR: creation of HACD instance failed.\n");return;}
		HACD::HACD& myHACD = hacdWrapper;
		
		myHACD.SetPoints(&points[0]);
		myHACD.SetNPoints(points.size());
		myHACD.SetTriangles(&triangles[0]);
		myHACD.SetNTriangles(triangles.size());
		
		myHACD.SetCompacityWeight(params.compacityWeight);
		myHACD.SetVolumeWeight(params.volumeWeight);
		const size_t nClusters = 
							(params.keepSubmeshesSeparated && subPart>=0 && params.keepSubmeshesSeparatedNClusters.size()>subPart && params.keepSubmeshesSeparatedNClusters[subPart]>0) ?
							(size_t) params.keepSubmeshesSeparatedNClusters[subPart] : params.nClusters;
		myHACD.SetNClusters(nClusters);                     
		const size_t maxHullVertices = 
							(params.keepSubmeshesSeparated && subPart>=0 && params.keepSubmeshesSeparatedMaxHullVertices.size()>subPart && params.keepSubmeshesSeparatedMaxHullVertices[subPart]>=0) ?
							(size_t) params.keepSubmeshesSeparatedMaxHullVertices[subPart] : params.maxHullVertices;
		myHACD.SetNVerticesPerCH(maxHullVertices);                      
		myHACD.SetConcavity(params.concavity);                    
		myHACD.SetAddExtraDistPoints(params.addExtraDistPoints);   
		//myHACD.SetAddNeighboursDistPoints(params.addNeighboursDistPoints);   
		myHACD.SetAddFacesPoints(params.addFacesPoints); 
		myHACD.SetConnectDist(params.connectionDistance);
		myHACD.SetNTargetTrianglesDecimatedMesh(params.targetNTrianglesDecimatedMesh);
		if (params.displayDebugInfo) myHACD.SetCallBack(&btHACDCompoundShape::HACDCallBackFunction);
		myHACD.SetScaleFactor(params.scaleFactor);
		myHACD.SetSmallClusterThreshold(params.smallClusterThreshold);
		
		myHACD.Compute(maxHullVertices == 0 ? true : false);
		
		if (!params.keepSubmeshesSeparated && params.optionalVRMLSaveFilePath.size()>0) myHACD.Save(params.optionalVRMLSaveFilePath.c_str(), false);
				
		for (size_t CH = 0,nClusters = myHACD.GetNClusters(); CH < nClusters; ++CH)	{
			
			const size_t nPoints =  myHACD.GetNPointsCH(CH);
			const size_t nTriangles =  myHACD.GetNTrianglesCH(CH);
			
			std::vector < HACD::Vec3<HACD::Real> > points(nPoints);
			std::vector < HACD::Vec3<long> > triangles(nTriangles);	// unused, but they could be used to display the convex hull...
			if (myHACD.GetCH(CH, &points[0], &triangles[0]))	{
 				btConvexHullShape* convexHullShape = NULL;

 				#pragma region Calculate centroid
 				// We use the simple aabb center, not the center of the homogeneous mass in the volume.
 				// This makes sense, since HACD decomposed shapes are not suitable for being released to 'destroy' an object.
 				// That's because they can overlap in various ways (that's not always bad for collision detection, because they
 				// can avoid small objects to get stuck between clusters better).	
 				btVector3 centroid(0,0,0);
				btVector3 minValue,maxValue,tempVert;
				if (nPoints>0)	{
					minValue=maxValue=btVector3(points[0].X(),points[0].Y(),points[0].Z());
				}
				else minValue=maxValue=btVector3(0,0,0);
 				for (size_t i=1; i<nPoints; i++)	{
 					tempVert = btVector3(points[i].X(),points[i].Y(),points[i].Z());
					minValue.setMin(tempVert);
					maxValue.setMax(tempVert);
				}
				centroid = (minValue+maxValue)*btScalar(0.5);			
				#pragma endregion
			
				#pragma region Calculate convexHullShape
				btAlignedObjectArray < btVector3 > verts;
				verts.resize(nPoints);
				for (size_t i=0; i<nPoints; i++)	{
					verts[i]=btVector3(points[i].X()-centroid.x(),points[i].Y()-centroid.y(),points[i].Z()-centroid.z());
				}	
				if (params.shrinkObjectInwardsToCompensateCollisionMargin)	{
					btAlignedObjectArray<btVector3> planeEquations;
					btGeometryUtil::getPlaneEquationsFromVertices(verts,planeEquations);

					btAlignedObjectArray<btVector3> shiftedPlaneEquations;
					btVector3 plane;
					for (int p=0,psz = planeEquations.size();p<psz;p++)
					{
						plane = planeEquations[p];
						plane[3] += params.convexHullsCollisionMargin;
						shiftedPlaneEquations.push_back(plane);
					}
					btAlignedObjectArray<btVector3> shiftedVertices;
					btGeometryUtil::getVerticesFromPlaneEquations(shiftedPlaneEquations,shiftedVertices);
					
					convexHullShape = new btConvexHullShape(&(shiftedVertices[0].getX()),shiftedVertices.size());				
				}							
				else convexHullShape = new btConvexHullShape(&verts[0].x(),verts.size());
				convexHullShape->setMargin(params.convexHullsCollisionMargin);
				if (params.reduceHullVerticesUsingBtShapeHull)	{
					//create a hull approximation
   					btShapeHull* hull = new btShapeHull(convexHullShape);
					if (hull)	{
						hull->buildHull(params.convexHullsCollisionMargin);
						if (hull->numVertices() < verts.size())	{
							delete convexHullShape;convexHullShape = NULL;
							convexHullShape = new btConvexHullShape((btScalar*)hull->getVertexPointer(),hull->numVertices());
							convexHullShape->setMargin(params.convexHullsCollisionMargin);
						}
   						delete hull;hull = NULL;
   					}	
				}
				if (params.convexHullsEnablePolyhedralContactClipping) convexHullShape->initializePolyhedralFeatures();	
				#pragma endregion
			
 				this->addChildShape(btTransform(btQuaternion::getIdentity(),centroid),convexHullShape);	
 				if (params.keepSubmeshesSeparated) m_submeshIndexOfChildShapes.push_back(subPart);				
			}
		}
}		


virtual const char*	getName()const {return "btHACDCompoundShape";}

btAlignedObjectArray< int > m_submeshIndexOfChildShapes;

static void GetStridingInterfaceContentOfSingleSubmesh(const btStridingMeshInterface* sti,std::vector< HACD::Vec3<HACD::Real> >& vertsOut,std::vector< HACD::Vec3<long> >& trianglesOut,int subMeshIndex)	{
	typedef HACD::Real vertexType;
	typedef long indexType;

	vertsOut.clear();trianglesOut.clear();
	if (!sti) return;
				
	const int subparts = sti->getNumSubParts();
	if (subparts<=0 || subMeshIndex<0 || subMeshIndex>=subparts) return;		
								
	const unsigned char *pverts;int numVerts;	PHY_ScalarType type;int stride;
	const unsigned char *pinds;int indsStride;int numTriangles;PHY_ScalarType indsType;
	
	unsigned int vCnt=0;
	unsigned int tCnt=0;
		
	const int subpart = subMeshIndex;
						 
	 	{
		sti->getLockedReadOnlyVertexIndexBase(&pverts,numVerts,type,stride,&pinds,indsStride,numTriangles,indsType,subpart);
		const size_t startVerts = vertsOut.size();
		const size_t startTriangles = trianglesOut.size();
		
		try	{
			
							
			switch (type)	{
				case PHY_FLOAT:	{
					typedef float glVertex;
					const glVertex* verts = (const glVertex*) pverts; 
					const int vertexDeltaStrideInGLVertexUnits = stride <= sizeof(glVertex)*3 ? 0 : (stride-sizeof(glVertex)*3)/sizeof(glVertex); 			
					
					vertsOut.resize(startVerts+numVerts);
					for (size_t i = 0; i < numVerts;++i)	{
						vertsOut[vCnt].X()=(vertexType) *verts++;
						vertsOut[vCnt].Y()=(vertexType) *verts++;
						vertsOut[vCnt++].Z()=(vertexType) *verts++;
						
						verts+=vertexDeltaStrideInGLVertexUnits;
					}					
			 	break;
		 		}
				case PHY_DOUBLE:{
					typedef double glVertex;
					const glVertex* verts = (const glVertex*) pverts; 
					const int vertexDeltaStrideInGLVertexUnits = stride <= sizeof(glVertex)*3 ? 0 : (stride-sizeof(glVertex)*3)/sizeof(glVertex); 			

					vertsOut.resize(startVerts+numVerts);
					for (size_t i = 0; i < numVerts;++i)	{
						vertsOut[vCnt].X()=(vertexType) *verts++;
						vertsOut[vCnt].Y()=(vertexType) *verts++;
						vertsOut[vCnt++].Z()=(vertexType) *verts++;
						
						verts+=vertexDeltaStrideInGLVertexUnits;
					}
			 	break;
		 		}
				default:
				btAssert((type == PHY_FLOAT) || (type == PHY_DOUBLE));
			}
		

			switch (indsType)	{
				case PHY_INTEGER:	{
			 		typedef int glIndex;
			 		const glIndex* inds = (const glIndex*) pinds; 
			 		const int indexDeltaStrideInGLIndexUnits = indsStride <= sizeof(indsStride)*3 ? 0 : (stride-sizeof(indsStride)*3)/sizeof(indsStride); 
			 		
					trianglesOut.resize(startTriangles+numTriangles);
					for (size_t i = 0; i < numTriangles;++i)	{
						trianglesOut[tCnt].X() = (indexType) *inds++;
						trianglesOut[tCnt].Y() = (indexType) *inds++;
						trianglesOut[tCnt++].Z() = (indexType) *inds++;
						
						inds+=indexDeltaStrideInGLIndexUnits;
					}
				 	break;
				}
				case PHY_SHORT:	{
					typedef unsigned short glIndex;
			 		const glIndex* inds = (const glIndex*) pinds;
			 		const int indexDeltaStrideInGLIndexUnits = indsStride <= sizeof(indsStride)*3 ? 0 : (stride-sizeof(indsStride)*3)/sizeof(indsStride); 			 		
			 		
					trianglesOut.resize(startTriangles+numTriangles);
					for (size_t i = 0; i < numTriangles;++i)	{
						trianglesOut[tCnt].X() = (indexType) *inds++;
						trianglesOut[tCnt].Y() = (indexType) *inds++;
						trianglesOut[tCnt++].Z() = (indexType) *inds++;
						
						inds+=indexDeltaStrideInGLIndexUnits;												
					}					 		 
				 	break;
				}
				default:
			 	btAssert((indsType == PHY_INTEGER) || (indsType == PHY_SHORT));
			}
		}
		catch (...) {
			vertsOut.resize(startVerts);
			trianglesOut.resize(startTriangles);
		}
		
		sti->unLockReadOnlyVertexBase(subpart);
		}		
}
static void GetStridingInterfaceContent(const btStridingMeshInterface* sti,std::vector< HACD::Vec3<HACD::Real> >& vertsOut,std::vector< HACD::Vec3<long> >& trianglesOut,const btHACDCompoundShape::Params::IntVector* processOnlySelectedSubmeshesInAscendingOrder=NULL)	{
	typedef HACD::Real vertexType;
	typedef long indexType;

	vertsOut.clear();trianglesOut.clear();
	if (!sti) return;
				
	const int subparts = sti->getNumSubParts();
	if (subparts<=0 || (processOnlySelectedSubmeshesInAscendingOrder && processOnlySelectedSubmeshesInAscendingOrder->size()==0)) return;		
								
	const unsigned char *pverts;int numVerts;	PHY_ScalarType type;int stride;
	const unsigned char *pinds;int indsStride;int numTriangles;PHY_ScalarType indsType;
	
	btAlignedObjectArray < const unsigned char * > vertexBaseOfProcessedSubMeshes;
	btAlignedObjectArray < int > startVertexIndexOfVertexBaseOfProcessedSubMeshes;
	//#define DONT_REUSE_SHARED_VERTEX_ARRAY	// problably safer, but much slower
	
	unsigned int vCnt=0;
	unsigned int tCnt=0;
		
	int curSelSm = -1;
	int curSelSmIdx = 0;	
	int numProcessedSubmeshes = 0;
	const bool mustSelectSubMeshes = processOnlySelectedSubmeshesInAscendingOrder;			
	if (mustSelectSubMeshes) curSelSm = (*processOnlySelectedSubmeshesInAscendingOrder)[curSelSmIdx];
	
	//::MessageBox(NULL,sw::ToString(subParts).c_str(),_T("subParts"),MB_OK);	
									
	for (int subpart=0;subpart<subparts;subpart++)	{	
		//-----------------------------------------------------------------------------------------------------------------------------------
		if (curSelSm>=0 && curSelSm!=subpart) continue;
		else if (mustSelectSubMeshes) {
			// Prepare the index for next cycle and continue:
			if (processOnlySelectedSubmeshesInAscendingOrder->size()<=curSelSmIdx+1) curSelSm = subparts;	// This will never process successive cycles
			else curSelSm =  (*processOnlySelectedSubmeshesInAscendingOrder)[++curSelSmIdx];				// Ready for next cycle			
		}
		//------------------------------------------------------------------------------------------------------------------------------------				
		sti->getLockedReadOnlyVertexIndexBase(&pverts,numVerts,type,stride,&pinds,indsStride,numTriangles,indsType,subpart);
		const size_t startVerts = vertsOut.size();
		const size_t startTriangles = trianglesOut.size();
		
		int startVertexIndex = startVerts;
		bool vertexArrayIsShared = false;	
		
		#ifndef DONT_REUSE_SHARED_VERTEX_ARRAY	
		{
			for (int i = 0,sz = vertexBaseOfProcessedSubMeshes.size();i<sz;i++)	{
				if (pverts == vertexBaseOfProcessedSubMeshes[i])	{
					startVertexIndex = startVertexIndexOfVertexBaseOfProcessedSubMeshes[i];
					vertexArrayIsShared = true;
					break;
				}
			} 
		
			vertexBaseOfProcessedSubMeshes.push_back(pverts);
			startVertexIndexOfVertexBaseOfProcessedSubMeshes.push_back(startVertexIndex);
		}
		#else //DONT_REUSE_SHARED_VERTEX_ARRAY
			#undef DONT_REUSE_SHARED_VERTEX_ARRAY
		#endif //DONT_REUSE_SHARED_VERTEX_ARRAY
		
		const indexType indsAdder = (indexType) startVertexIndex; 
				
		
		try	{
			
			if (!vertexArrayIsShared)				
			switch (type)	{
				case PHY_FLOAT:	{
					typedef float glVertex;
					const glVertex* verts = (const glVertex*) pverts; 
					const int vertexDeltaStrideInGLVertexUnits = stride <= sizeof(glVertex)*3 ? 0 : (stride-sizeof(glVertex)*3)/sizeof(glVertex); 			
					
					vertsOut.resize(startVerts+numVerts);
					for (size_t i = 0; i < numVerts;++i)	{
						vertsOut[vCnt].X()=(vertexType) *verts++;
						vertsOut[vCnt].Y()=(vertexType) *verts++;
						vertsOut[vCnt++].Z()=(vertexType) *verts++;
						
						verts+=vertexDeltaStrideInGLVertexUnits;
					}					
			 	break;
		 		}
				case PHY_DOUBLE:{
					typedef double glVertex;
					const glVertex* verts = (const glVertex*) pverts; 
					const int vertexDeltaStrideInGLVertexUnits = stride <= sizeof(glVertex)*3 ? 0 : (stride-sizeof(glVertex)*3)/sizeof(glVertex); 			

					vertsOut.resize(startVerts+numVerts);
					for (size_t i = 0; i < numVerts;++i)	{
						vertsOut[vCnt].X()=(vertexType) *verts++;
						vertsOut[vCnt].Y()=(vertexType) *verts++;
						vertsOut[vCnt++].Z()=(vertexType) *verts++;
						
						verts+=vertexDeltaStrideInGLVertexUnits;
					}
			 	break;
		 		}
				default:
				btAssert((type == PHY_FLOAT) || (type == PHY_DOUBLE));
			}
		

			switch (indsType)	{				
				case PHY_INTEGER:	{
			 		typedef int glIndex;
			 		const glIndex* inds = (const glIndex*) pinds; 
			 		const int indexDeltaStrideInGLIndexUnits = indsStride <= sizeof(indsStride)*3 ? 0 : (stride-sizeof(indsStride)*3)/sizeof(indsStride); 
			 		
					trianglesOut.resize(startTriangles+numTriangles);
					for (size_t i = 0; i < numTriangles;++i)	{
						trianglesOut[tCnt].X() = startVertexIndex + (indexType) *inds++;
						trianglesOut[tCnt].Y() = startVertexIndex + (indexType) *inds++;
						trianglesOut[tCnt++].Z() = startVertexIndex + (indexType) *inds++;
						
						inds+=indexDeltaStrideInGLIndexUnits;
					}
				 	break;
				}
				case PHY_SHORT:	{
					typedef unsigned short glIndex;
			 		const glIndex* inds = (const glIndex*) pinds;
			 		const int indexDeltaStrideInGLIndexUnits = indsStride <= sizeof(indsStride)*3 ? 0 : (stride-sizeof(indsStride)*3)/sizeof(indsStride); 			 		
			 		
					trianglesOut.resize(startTriangles+numTriangles);
					for (size_t i = 0; i < numTriangles;++i)	{
						trianglesOut[tCnt].X() = startVertexIndex + (indexType) *inds++;
						trianglesOut[tCnt].Y() = startVertexIndex + (indexType) *inds++;
						trianglesOut[tCnt++].Z() = startVertexIndex + (indexType) *inds++;
						
						inds+=indexDeltaStrideInGLIndexUnits;												
					}					 		 
				 	break;
				}
				default:
			 	btAssert((indsType == PHY_INTEGER) || (indsType == PHY_SHORT));
			}
		}
		catch (...) {
			vertsOut.resize(startVerts);
			trianglesOut.resize(startTriangles);
		}
		
		sti->unLockReadOnlyVertexBase(subpart);
		}	
		
}


static HACD::Vec3<HACD::Real> GetAabbHalfExtentsFromStridingInterface(const btStridingMeshInterface* sti,HACD::Vec3<HACD::Real>* pOptionalCenterPointOut=NULL,const HACD::Vec3<HACD::Real>& scalingFactorToApply=HACD::Vec3<HACD::Real>(1,1,1))	{
	typedef HACD::Real vertexType;
	
	HACD::Vec3<vertexType> aabbHalfExtents(0,0,0);
	if (pOptionalCenterPointOut) *pOptionalCenterPointOut = HACD::Vec3<HACD::Real>(0,0,0);
	
	HACD::Vec3<vertexType> cmax(0,0,0);
	HACD::Vec3<vertexType> cmin(0,0,0);
	bool noStartingVertex = true;
	HACD::Vec3<vertexType> temp;
	
	if (!sti) return aabbHalfExtents;
				
	const int subParts = sti->getNumSubParts();
	const unsigned char *pverts;int numVerts;PHY_ScalarType type;int stride;
	const unsigned char *pinds;int indsStride;int numTriangles;PHY_ScalarType indsType;	// Unused, but needed
				
	for (int subpart=0;subpart<subParts;subpart++)	{	 
		sti->getLockedReadOnlyVertexIndexBase(&pverts,numVerts,type,stride,&pinds,indsStride,numTriangles,indsType,subpart);
		try	{						
			switch (type)	{
				case PHY_FLOAT:	{
					typedef float glVertex;
					const glVertex* verts = (const glVertex*) pverts; 
					const int vertexDeltaStrideInGLVertexUnits = stride <= sizeof(glVertex)*3 ? 0 : (stride-sizeof(glVertex)*3)/sizeof(glVertex); 			
					
					for (size_t i = 0; i < numVerts;++i)	{						
						temp.X()=(vertexType) *verts++;
						temp.Y()=(vertexType) *verts++;
						temp.Z()=(vertexType) *verts++;						
						verts+=vertexDeltaStrideInGLVertexUnits;
						
						if (noStartingVertex)	{
							noStartingVertex = false;
							cmin = temp;cmax = temp;
						}
						else	{
							if 		(cmax.X()<temp.X()) cmax.X()=temp.X();
							else if (cmin.X()>temp.X())	cmin.X()=temp.X();
							if 		(cmax.Y()<temp.Y()) cmax.Y()=temp.Y();
							else if (cmin.Y()>temp.Y())	cmin.Y()=temp.Y();
							if 		(cmax.Z()<temp.Z()) cmax.Z()=temp.Z();
							else if (cmin.Z()>temp.Z())	cmin.Z()=temp.Z();														
						}
					}					
			 	break;
		 		}
				case PHY_DOUBLE:{
					typedef double glVertex;
					const glVertex* verts = (const glVertex*) pverts; 
					const int vertexDeltaStrideInGLVertexUnits = stride <= sizeof(glVertex)*3 ? 0 : (stride-sizeof(glVertex)*3)/sizeof(glVertex); 			

					for (size_t i = 0; i < numVerts;++i)	{
						temp.X()=(vertexType) *verts++;
						temp.Y()=(vertexType) *verts++;
						temp.Z()=(vertexType) *verts++;						
						verts+=vertexDeltaStrideInGLVertexUnits;
						
						if (noStartingVertex)	{
							noStartingVertex = false;
							cmin = temp;cmax = temp;
						}
						else	{
							if 		(cmax.X()<temp.X()) cmax.X()=temp.X();
							else if (cmin.X()>temp.X())	cmin.X()=temp.X();
							if 		(cmax.Y()<temp.Y()) cmax.Y()=temp.Y();
							else if (cmin.Y()>temp.Y())	cmin.Y()=temp.Y();
							if 		(cmax.Z()<temp.Z()) cmax.Z()=temp.Z();
							else if (cmin.Z()>temp.Z())	cmin.Z()=temp.Z();														
						}
					}
			 	break;
		 		}
				default:
				btAssert((type == PHY_FLOAT) || (type == PHY_DOUBLE));
			}
		}
		catch (...) {
		}
		
		sti->unLockReadOnlyVertexBase(subpart);
	}	
	
	aabbHalfExtents = (cmax - cmin)*vertexType(0.5);
	aabbHalfExtents = HACD::Vec3<HACD::Real>(aabbHalfExtents.X()*scalingFactorToApply.X(),aabbHalfExtents.Y()*scalingFactorToApply.Y(),aabbHalfExtents.Z()*scalingFactorToApply.Z());
	if (pOptionalCenterPointOut) {
		*pOptionalCenterPointOut =  (cmax + cmin)*vertexType(0.5);
		*pOptionalCenterPointOut = HACD::Vec3<HACD::Real>(pOptionalCenterPointOut->X()*scalingFactorToApply.X(),pOptionalCenterPointOut->Y()*scalingFactorToApply.Y(),pOptionalCenterPointOut->Z()*scalingFactorToApply.Z()); 
	}	
	return aabbHalfExtents;
}

static void ScalePoints(std::vector < HACD::Vec3<HACD::Real> >& verts,const HACD::Vec3<HACD::Real>& scalingFactor)	{
	for (size_t i=0,sz=verts.size();i<sz;i++)	{
		HACD::Vec3<HACD::Real>& v = verts[i];
		v.X()*=scalingFactor.X();
		v.Y()*=scalingFactor.Y();
		v.Z()*=scalingFactor.Z();
	}
}
static void ShiftPoints(std::vector < HACD::Vec3<HACD::Real> >& verts,const HACD::Vec3<HACD::Real>& shift)	{
	for (size_t i=0,sz=verts.size();i<sz;i++)	{
		HACD::Vec3<HACD::Real>& v = verts[i];
		v+=shift;
	}
}

struct Less{
	inline bool operator()(const int& a,const int& b) const {return a<b;}
};

inline static bool AreEqual(const HACD::Real s1,const HACD::Real s2,const HACD::Real eps=SIMD_EPSILON)	{
	return fabs(s1-s2)< eps;
}

inline static bool AreEqual(const HACD::Vec3<HACD::Real>& v1,const HACD::Vec3<HACD::Real>& v2,const HACD::Vec3<HACD::Real>& eps=HACD::Vec3<HACD::Real>(SIMD_EPSILON,SIMD_EPSILON,SIMD_EPSILON) )	{
	return (AreEqual(v1.X(),v2.X(),eps.X()) && AreEqual(v1.Y(),v2.Y(),eps.Y()) && AreEqual(v1.Z(),v2.Z(),eps.Z()) );
}

// This same method can be used to decimate the mesh too. Note that if the mesh has texcoords or other additional data, these will be broken after this method is called.
void RemoveDoubleVertices(std::vector< HACD::Vec3<HACD::Real> >& verts,std::vector< HACD::Vec3<long> >& triangles,const bool removeDegenerateTrianglesToo=false,const HACD::Vec3<HACD::Real>& eps=HACD::Vec3<HACD::Real>(SIMD_EPSILON,SIMD_EPSILON,SIMD_EPSILON))	{
	
	typedef long U;
	typedef HACD::Vec3<HACD::Real> Vector3;
		
	U vertsSize = (U) verts.size();
	size_t trianglesSize = triangles.size();
	
	std::vector<U> S(vertsSize,vertsSize);	
	
	U cnt = 0, minDoubleVert = vertsSize, maxSingleVert = 0;
	
	std::vector< Vector3 > newVerts; 							
	newVerts.reserve(vertsSize);

	//---------- Optional Stuff--------------------
	// Basically we want to average the double verts before removing them
	// We can set to false the following line to speed up code...
	const bool useFineTuningForVerts = true;
	const bool useSomeFineTuning = useFineTuningForVerts;
		
	std::vector < unsigned int > C;	// counts the number a given vert is repeated
	if (useSomeFineTuning) C.reserve(vertsSize);

	//----------------------------------------------
	for (U t=0;t<vertsSize;t++)	{
		if (S[t]!=vertsSize) continue;
		S[t]=cnt;
		maxSingleVert = t;
		const Vector3* vertsT = &verts[t];
		if (useSomeFineTuning) C.push_back(1);	
		newVerts.push_back(*vertsT);
		for (U u=t+1;u<vertsSize;u++)	{
			if (AreEqual(*vertsT,verts[u],eps))	{
				S[u] = cnt;
				if (minDoubleVert > u) minDoubleVert = u;
				if (useSomeFineTuning)	{
					C[cnt]+=1;
					if (useFineTuningForVerts) newVerts[cnt]+=verts[u];	//These verts should be equal... however if we pass a bigger "eps" they could differ...
				}
			}
		}
		cnt++;
	} 

	const size_t newVertsSize = cnt;
	if (useSomeFineTuning)	{	// We must average the data so that we can replace multiple equal vertices with their average value
		for (size_t i = 0;i<newVertsSize;i++)	{
			if (C[i]==1) continue;
			const float count = (float) C[i];
			if (useFineTuningForVerts) newVerts[i]/=count;		
		}
	}
		
	
	{
		std::vector< HACD::Vec3<U> > newTriangles(trianglesSize);
		size_t numDegenerateTriangles = 0;	// Never used
		size_t cnt = 0;
		for (size_t t=0;t<trianglesSize;t++)	{
			const HACD::Vec3<U>& tri = triangles[t];
			HACD::Vec3<U>& newTri = newTriangles[cnt];
			newTri.X() = S[tri.X()];
			newTri.Y() = S[tri.Y()];
			newTri.Z() = S[tri.Z()];
			if (removeDegenerateTrianglesToo && (newTri.X()==newTri.Y() || newTri.Y()==newTri.Z() || newTri.X()==newTri.Z())) {
				++numDegenerateTriangles;continue;
			}
			++cnt;
		}
		if (trianglesSize!=cnt) newTriangles.resize(cnt);
		triangles = newTriangles;
	}

	
	verts = newVerts;	

}

static void HACDCallBackFunction(const char * msg, double progress, double concavity, size_t nVertices)
{
	printf("%s",msg);
}

};
#pragma endregion



#endif //BTHACDCOMPOUNDSHAPE_H__



