#pragma once
#include "../../Common/Maths.h"
#include "../../Common/std.h"


class PolygonalMesh
{
public:

	/*
		List of vertex indices composing triangles.
		Sent as is to openGL, reverting order flips normals.
	*/
	vector<uint32_t> m_meshTriangles;


	/*
		Now for Vertex specific data. Separate lists to simplify loading geometry data for rendering.
	*/
	vector<vec3> m_vertexPos;
	vector<vec3> m_vertexNormals;
	vector<vec3> m_vertexTangents;
	vector<vec3> m_vertexBiTangents;
	vector<vec2> m_vertexUVs;

	/*
		Now we introduce Half-Edge specific mapping to fasten up adjacency queries
	*/
	typedef struct HE_edge_t 
	{
		uint32_t destVertex; // Vertex the HE points to.
		uint32_t oppositeHE; // opposite Half-Edge
		uint32_t borderingFace; // Triangle to the left of the half-edge 
		uint32_t nextHE; // next half-edge around that face.
	} HE_edge;

	vector<HE_edge> m_halfEdges; // The list of all half edges (should be 2*E)

	vector<uint32_t> m_vertEmanatingHE; // a list of one HE index (from m_halfEdges) emanating from each vertex (should be size of m_vertexPos)
	vector<uint32_t> m_triangleHE; // a list of one HE index (from m_halfEdges) bordering each triangle (should be size of m_meshTriangles)
	int m_manifoldViolationEdges = -1;

	void BuildHalfEdgeDS();

	PolygonalMesh();
	~PolygonalMesh();

	void NormalizeModelCoordinates();
	void ComputeTangents();

	bool IsMeshValid();

	void GetHEvertices(uint32 halfEdge, pair<uint32, uint32>* vertexPair);
	void GetHETriangle(uint32 halfEdge, uint32* triangle);
	void GetSurroundingVertices(uint32 vertexIndx, vector<vec3>* surroundingVertices);
	void GetTriangleEdges(int triangle, uint32* edge0, uint32* edge1, uint32* edge2);

};