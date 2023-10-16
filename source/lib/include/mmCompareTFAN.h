#ifndef _MM_COMPARETFAN_H_
#define _MM_COMPARETFAN_H_

#include "mmModel.h"

typedef std::vector<int> IntVect;
typedef std::map<int, IntVect> IntMultiVect;
typedef std::vector<IntVect> VectIntVect;
namespace mm {
	class CompareTFAN {
	public:
		struct CompareStateInfo {
			int vA = 0;
			std::vector<int> vertexMapA;
			std::vector<int> tagsVA;
			std::vector<int> tagsTA;
			int vCountA;
			std::vector<int> vertexMapB;
			std::vector<int> tagsVB;
			std::vector<int> tagsTB;
			int vCountB;
		};
		struct TFanStateInfo {
			int iterationTimes = 0;
			int tFanListID = 0;
			std::vector<VectIntVect> tFansList;
			IntVect remainFlagList;
			VectIntVect tagsTList;

			int vCount = 0;
			std::vector<int> vertexMap;
			std::vector<int> tagsV;
		};
		void ComputeVertex2Vertex(const Model& inputA, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, VectIntVect& vertex2VertexA, int compNum);

		void ComputeVertex2Triangle(const Model& inputA, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, VectIntVect& vertex2TriangleA, int compNum);

		void ComputeTriangle2Triangle(const Model& inputA, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, VectIntVect& triangle2TriangleA, VectIntVect& vertex2TriangleA);

		void ComputeAdjacency(const Model& inputA, const Model& inputB, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, const std::vector<int>& trianglesB, const std::vector<float>& verticesB, VectIntVect& vertex2VertexA, VectIntVect& vertex2VertexB, VectIntVect& vertex2TriangleA, VectIntVect& vertex2TriangleB, VectIntVect& triangle2TriangleA, VectIntVect& triangle2TriangleB, int compNum);

		bool getTrianglesFansStatus(bool unoriented, const Model& inputA,
			int focusVertexA, std::vector<int>& VLA, std::vector<int>& tagsV_A,
			std::vector<int>& vertexMap_A, int& vertexCount_A, VectIntVect& triangleFans, IntVect sortedConquestedVLA, IntVect& nbrFans_A, IntVect& degree_A, IntVect& cases_A, IntVect& ops_A, IntVect& vertices_A);

		void compareTriangleFans(const Model& inputA, const Model& inputB, bool& unoriented, const std::vector<int>& trianglesA,
			const std::vector<float>& verticesA, const std::vector<int>& trianglesB,
			const std::vector<float>& verticesB, int focusVertex, bool& found, int& vertexCountA, std::vector<int>& vertexMapA, std::vector<int>& tagsVA, std::vector<int>& tagsTA, int& vertexCountB, std::vector<int>& vertexMapB, std::vector<int>& tagsVB, std::vector<int>& tagsTB, VectIntVect& vertexMapTemList, VectIntVect& tagsVTempList, VectIntVect& tagsTTempList, IntVect& vertexCountTempList, VectIntVect& vertex2TriangleA, VectIntVect& vertex2TriangleB, int compNum);

		bool buildTriangleFanA(const Model& inputA, bool unoriented, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, int focusVertexA, IntVect& tagsTA,
			std::vector<VectIntVect>& triangleFansList, std::vector<int>& TLA, IntMultiVect& triangle2Triangle_temp, int& referTmin, int& referNextT, bool& remainTriangles);

		bool buildTriangleFanB(const Model& inputA, bool unoriented, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, int focusVertexA, IntVect& tagsTA,
			VectIntVect& tagsTListA, std::vector<VectIntVect>& triangleFansList, std::vector<int>& TLA, IntMultiVect& triangle2Triangle_temp,
			const std::vector<int>& referInput1, const std::vector<float>& referInput2, int& referTmin, int& referNextT, int referFocusVertex,
			IntVect& remainFlagList, int compNum);

		bool processTriangle(const Model& inputA, bool unoriented, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, int focusVertexA, IntVect& tagsTA,
			std::vector<int>& vertexMap_A, IntVect& sortedConquestedVLA, std::vector<int>& TLA, IntMultiVect& triangle2Triangle, IntVect& vfan_degeneratedface, VectIntVect& vertex2TriangleA, int compNum);

		bool computeTriangleFan(const Model& inputA, const std::vector<int>& trianglesA, int focusVertexA, int tMin, IntMultiVect& triangle2Triangle, std::vector<int>& TLA, std::vector<int>& tagsT_A, VectIntVect& triangleFans, int& referTmin, int& referNextT, bool       computeflag);

		bool compareConnectivity(const Model& inputA, const Model& inputB,
			bool unoriented, const std::vector<int>& trianglesA,
			const std::vector<float>& verticesA, const std::vector<int>& trianglesB,
			const std::vector<float>& verticesB, bool earlyReturn, int compNum, size_t& diffs);

		void AddNeighborVertex2Vertex(const Model& inputA, VectIntVect& vertex2VertexA, int v1, int v2)
		{
			int found = 0;
			for (IntVect::iterator posc = vertex2VertexA[v1].begin(); posc != vertex2VertexA[v1].end(); ++posc) {
				if ((*posc) == v2) {
					found = 1;
					break;
				}
			}
			if (found == 0)
				vertex2VertexA[v1].push_back(v2);
		}
		void AddNeighborVertex2Triangle(const Model& inputA, VectIntVect& vertex2TriangleA, int v, int t)
		{
			int found = 0;
			for (IntVect::iterator posc = vertex2TriangleA[v].begin(); posc != vertex2TriangleA[v].end(); ++posc) {
				if ((*posc) == t) {
					found = 1;
					break;
				}
			}
			if (found == 0)
				vertex2TriangleA[v].push_back(t);
		}
		void AddNeighborTriangle2Triangle(const Model& inputA, VectIntVect& triangle2TriangleA, int t1, int t2)
		{
			int found = 0;
			for (IntVect::iterator posc = triangle2TriangleA[t1].begin(); posc != triangle2TriangleA[t1].end(); ++posc) {
				if ((*posc) == t2) {
					found = 1;
					break;
				}
			}
			if (found == 0)
				triangle2TriangleA[t1].push_back(t2);
		}

		bool areIntVecEqual(IntVect& a, IntVect& b) {
			if (a.size() != b.size())
				return false;
			for (int i = 0; i < a.size(); i++) {
				if (a[i] != b[i])
					return false;
			}
			return true;
		}

		bool areVLEqual(const std::vector<float>& verticesA, const std::vector<float>& verticesB, IntVect& a, IntVect& b, int compNum) {
			if (a.size() != b.size())
				return false;
			std::vector<bool> flag(b.size(), false);
			for (int i = 0; i < a.size(); i++) {
				bool found = false;
				for (int j = 0; j < b.size(); j++) {
					if (flag[j])
						continue;
					int k = 0;
					for (k = 0; k < compNum; k++) {
						if (verticesB[b[j] * compNum + k] != verticesA[a[i] * compNum + k]) {
							break;
						}
					}
					if (k == compNum) {
						flag[j] = true;
						found = true;
						break;
					}

				}
				if (!found) return false;
			}
			return true;
		}

		bool areVertexEqual(const std::vector<float>& verticesA, const std::vector<float>& verticesB, int va, int vb, int compNum) {
			for (int i = 0; i < compNum; i++) {
				if (verticesB[vb * compNum + i] != verticesA[va * compNum + i]) {
					return false;
				}
			}
			return true;
		}
		bool GetCoordIndex(const std::vector<int>& trianglesA, int pos, int* coordIndex) {
			int m_nTriangles = trianglesA.size() / 3;
			if (pos < m_nTriangles) {
				for (int h = 0; h < 3; h++) {
					coordIndex[h] = trianglesA[pos * 3 + h];
				}
				return true;
			}
			return false;
		}

		bool IsCase0(int degree, std::vector<int>& ops, std::vector<int>& vertices) {
			// ops: 1000001 vertices: -1 -2
			if ((vertices.size() != 2) || (degree < 2)) {
				return false;
			}
			if ((vertices[0] != -1) || (vertices[1] != -2)/*(vertices[0] <= vertices[1])*/ ||
				(ops[0] != 1) || (ops[degree - 1] != 1)) return false;
			for (int u = 1; u < degree - 1; u++) {
				if (ops[u] != 0) return false;
			}
			return true;
		}
		bool  IsCase1(int degree, std::vector<int>& ops, std::vector<int>& vertices) {
			// ops: 1xxxxxx1 vertices: -1 x x x x x -2
			if ((degree < 2) || (vertices.size() < 1)) {
				return false;
			}
			if ((vertices[0] != -1) || (vertices[vertices.size() - 1] != -2) ||
				(ops[0] != 1) || (ops[degree - 1] != 1)) return false;
			return true;
		}
		bool IsCase2(int degree, std::vector<int>& ops, std::vector<int>& vertices) {
			// ops: 00000001 vertices: -1
			if ((degree < 2) || (vertices.size() != 1)) {
				return false;
			}
			if ((vertices[0] != -1) || (ops[degree - 1] != 1)) return false;
			for (int u = 0; u < degree - 1; u++) {
				if (ops[u] != 0) return false;
			}
			return true;
		}
		bool IsCase3(int degree, std::vector<int>& ops, std::vector<int>& vertices) {
			// ops: 00000001 vertices: -2
			if ((degree < 2) || (vertices.size() != 1)) {
				return false;
			}
			if ((vertices[0] != -2) || (ops[degree - 1] != 1)) return false;
			for (int u = 0; u < degree - 1; u++) {
				if (ops[u] != 0) return false;
			}
			return true;
		}
		bool IsCase4(int degree, std::vector<int>& ops, std::vector<int>& vertices) {
			// ops: 10000000 vertices: -1
			if ((degree < 2) || (vertices.size() != 1)) {
				return false;
			}
			if ((vertices[0] != -1) || (ops[0] != 1)) return false;
			for (int u = 1; u < degree; u++) {
				if (ops[u] != 0) return false;
			}
			return true;
		}
		bool IsCase5(int degree, std::vector<int>& ops, std::vector<int>& vertices) {
			// ops: 10000000 vertices: -2
			if ((degree < 2) || (vertices.size() != 1)) {
				return false;
			}
			if ((vertices[0] != -2) || (ops[0] != 1)) return false;
			for (int u = 1; u < degree; u++) {
				if (ops[u] != 0) return false;
			}
			return true;
		}
		bool IsCase6(int degree, std::vector<int>& ops, std::vector<int>& vertices) {
			// ops: 0000000 vertices: 
			if (vertices.size() != 0) {
				return false;
			}
			for (int u = 0; u < degree; u++) {
				if (ops[u] != 0) return false;
			}
			return true;
		}
		bool IsCase7(int degree, std::vector<int>& ops, std::vector<int>& vertices) {
			// ops: 1000001 vertices: -2 -1
			if ((vertices.size() != 2) || (degree < 2)) {
				return false;
			}
			if ((vertices[0] != -2) || (vertices[1] != -1)/*(vertices[0] >= vertices[1])*/ ||
				(ops[0] != 1) || (ops[degree - 1] != 1)) return false;
			for (int u = 1; u < degree - 1; u++) {
				if (ops[u] != 0) return false;
			}
			return true;
		}
		bool IsCase8(int degree, std::vector<int>& ops, std::vector<int>& vertices) {
			// ops: 1xxxxxx1 vertices: -1 x x x x x -2
			if ((degree < 2) || (vertices.size() < 1)) {
				return false;
			}
			if ((vertices[0] != -2) || (vertices[vertices.size() - 1] != -1) ||
				(ops[0] != 1) || (ops[degree - 1] != 1)) return false;
			return true;
		}

	};

}

// namespace mm

#endif
#pragma once
#pragma once
