#include <iostream>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <time.h>
#include <math.h>
#include <string>
#include <vector>
#include <stack>

#include "mmIO.h"
#include "mmModel.h"
#include "mmImage.h"
#include "mmSample.h"
#include "mmGeometry.h"
#include "mmColor.h"
#include "mmRendererSw.h"
#include "mmRendererHw.h"
#include "mmStatistics.h"
#include "mmCompareTFAN.h"

using namespace mm;

bool CompareTFAN::processTriangle(const Model& inputA, bool unoriented, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, int focusVertexA, IntVect& tagsTA,
	std::vector<int>& vertexMap_A, IntVect& sortedConquestedVLA, std::vector<int>& TLA, IntMultiVect& triangle2Triangle_temp, IntVect& vfan_degeneratedface, VectIntVect& vertex2TriangleA, int compNum) {
	int vA = -1;
	int tA = -1;
	int v0A = -1;
	// we get all free/conquested triangles adjacent to focusVertex
	// we get all conquested vertices adjacent to focusVertex
	std::map<int, int> conquestedVLA;
	int connec[3] = { -1, -1, -1 };
	//store unprocessed triangles
	for (int j = 0; j != vertex2TriangleA[focusVertexA].size(); j++) {
		tA = vertex2TriangleA[focusVertexA][j];
		if (tagsTA[tA] == 0) {
			TLA.push_back(tA);
		}
		else {
			GetCoordIndex(trianglesA, tA, connec);
			for (int k = 0; k != 3; k++) {
				vA = vertexMap_A[connec[k]];
				if ((connec[k] != focusVertexA) && (vA > vertexMap_A[focusVertexA])) {// we get only the visited vertices
					conquestedVLA[vA] = connec[k];
				}
			}
		}
	}
	// sort the vertices by an incresing order of traversal
	for (std::map<int, int>::iterator it = conquestedVLA.begin(); it != conquestedVLA.end(); it++) {
		sortedConquestedVLA.push_back(it->first);
	}

	//----------------------------------
	if (TLA.size() != 0) {
		// Compute trinagles adjacency
		int vertexNT[3] = { -1, -1, -1 };
		int vertexT[3] = { -1, -1, -1 };
		// triangle with minimum of connectivity
		for (int i = 0; i < (int)TLA.size(); i++) {
			GetCoordIndex(trianglesA, TLA[i], vertexT);
			for (int j = 0; j < (int)TLA.size(); j++) {
				if (i != j) {
					GetCoordIndex(trianglesA, TLA[j], vertexNT);
					int found = 0;
					for (int k1 = 0; k1 < 3; k1++) {
						for (int k2 = 0; k2 < 3; k2++) {
							if (vertexNT[k1] == vertexT[k2]) found++;
						}
					}
					if ((found == 2) || (found == 3)) {
						int flag = 0;
						for (int h = 0; h < (int)triangle2Triangle_temp[TLA[i]].size(); h++) {
							if (triangle2Triangle_temp[TLA[i]][h] == TLA[j]) flag = 1;
						}
						if (flag == 0) triangle2Triangle_temp[TLA[i]].push_back(TLA[j]);
					}
				}
			}
		}//the adjacent triangles of the unprocessed triangle
		// handle degenerated faces
		for (int i = 0; i < (int)TLA.size(); i++)
		{
			if (tagsTA[TLA[i]] == 0)
			{
				bool flag = false;
				GetCoordIndex(trianglesA, TLA[i], vertexT);
				if (compNum == 3) {
					if ((verticesA[vertexT[0] * 3] == verticesA[vertexT[1] * 3] && verticesA[vertexT[0] * 3 + 1] == verticesA[vertexT[1] * 3 + 1] && verticesA[vertexT[0] * 3 + 2] == verticesA[vertexT[1] * 3 + 2]) ||
						(verticesA[vertexT[0] * 3] == verticesA[vertexT[2] * 3] && verticesA[vertexT[0] * 3 + 1] == verticesA[vertexT[2] * 3 + 1] && verticesA[vertexT[0] * 3 + 2] == verticesA[vertexT[2] * 3 + 2]) ||
						(verticesA[vertexT[1] * 3] == verticesA[vertexT[2] * 3] && verticesA[vertexT[1] * 3 + 1] == verticesA[vertexT[2] * 3 + 1] && verticesA[vertexT[1] * 3 + 2] == verticesA[vertexT[2] * 3 + 2])) {
						flag = true;
					}
				}
				else if (compNum == 2) {
					if ((verticesA[vertexT[0] * 2] == verticesA[vertexT[1] * 2] && verticesA[vertexT[0] * 2 + 1] == verticesA[vertexT[1] * 2 + 1]) ||
						(verticesA[vertexT[0] * 2] == verticesA[vertexT[2] * 2] && verticesA[vertexT[0] * 2 + 1] == verticesA[vertexT[2] * 2 + 1]) ||
						(verticesA[vertexT[1] * 2] == verticesA[vertexT[2] * 2] && verticesA[vertexT[1] * 2 + 1] == verticesA[vertexT[2] * 2 + 1])) {
						flag = true;
					}
				}
				if (flag) {
					tagsTA[TLA[i]] = 1;
					for (int k = 0; k < 3; k++) {
						vfan_degeneratedface.push_back(vertexT[k]);
					}
				}
			}
		}
	}
	return true;
}
bool CompareTFAN::buildTriangleFanA(const Model& inputA, bool unoriented, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, int focusVertexA, IntVect& tagsTA,
	std::vector<VectIntVect>& triangleFansList, std::vector<int>& TLA, IntMultiVect& triangle2Triangle_temp, int& referTmin, int& referNextT, bool& remainTriangles, int& compNum) {
	if (TLA.size() != 0) {
		int tMin = 0;
		int connecMin = 0;
		int nextT = 0;
		int nextTOld = 0;
		bool firstTime = true;
		std::vector<int> tMinL;
		std::vector<std::vector<int>> nextTLists;
		VectIntVect triangleFans;
		bool computeflag = true; // compute triangleA, flag as a switch
		computeTriangleFan(inputA, trianglesA, verticesA, verticesA, focusVertexA, tMin, triangle2Triangle_temp, TLA, tagsTA, triangleFans, referTmin, referNextT, trianglesA, computeflag, compNum);
		if (triangleFans.size() != 0) {
			triangleFansList.push_back(triangleFans);
		}
		remainTriangles = false;
		for (int i = 0; i < TLA.size(); i++) {
			if (tagsTA[TLA[i]] == 0) {
				remainTriangles = true;
			}
		}
	}
	else {
		remainTriangles = false;
	}
	return true;
}
bool CompareTFAN::buildTriangleFanB(const Model& inputA, bool unoriented, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, int focusVertexA, IntVect& tagsTA, IntVect& tagsV_B_temp, VectIntVect& tagsV_B_Temp, VectIntVect& tagsTListA, std::vector<VectIntVect>& triangleFansList, std::vector<int>& TLA, IntMultiVect& triangle2Triangle_temp, const std::vector<int>& referInput1, const std::vector<float>& referInput2, int& referTmin, int& referNextT, int referFocusVertex, IntVect& remainFlagList, int compNum) {

	int tMin = 0;
	int connecMin = 0;
	int nextT = 0;
	int nextTOld = 0;
	bool firstTime = true;
	std::vector<int> tMinL;
	std::vector<std::vector<int>> nextTLists;
	VectIntVect triangleFans;
	if (referTmin != -1) {
		connecMin = 10000000;
		// to set the same tMin with reference mesh
		int vertexR_tmin[3] = { -1, -1, -1 };
		int vertexR_tnext[3] = { -1, -1, -1 };
		GetCoordIndex(referInput1, referTmin, vertexR_tmin);
		for (int i = 0; i < (int)TLA.size(); i++) {
			if (tagsTA[TLA[i]] == 0) {
				tMin = -1;
				int vertexT[3] = { -1, -1, -1 };
				GetCoordIndex(trianglesA, TLA[i], vertexT);
				for (int j = 0; j < 3; j++) {
					if (areVertexEqual(verticesA, referInput2, vertexT[0], vertexR_tmin[0 + j], compNum)
						&& areVertexEqual(verticesA, referInput2, vertexT[1], vertexR_tmin[(1 + j) % 3], compNum)
						&& areVertexEqual(verticesA, referInput2, vertexT[2], vertexR_tmin[(2 + j) % 3], compNum)
						&& ((vertexT[0] == focusVertexA && vertexR_tmin[0 + j] == referFocusVertex)
							|| (vertexT[1] == focusVertexA && vertexR_tmin[(1 + j) % 3] == referFocusVertex)
							|| (vertexT[2] == focusVertexA && vertexR_tmin[(2 + j) % 3] == referFocusVertex))) {
						if (connecMin >= (int)triangle2Triangle_temp[TLA[i]].size()) {
							connecMin = (int)triangle2Triangle_temp[TLA[i]].size();
							tMin = TLA[i];
							break;
						}
					}
				}
				if (tMin != -1) {
					// set the same nextT with reference mesh
					std::vector<int> nextTL;
					if (referNextT != -1) {
						GetCoordIndex(referInput1, referNextT, vertexR_tnext);
						for (int h = 0; h < (int)triangle2Triangle_temp[tMin].size(); h++) {
							if (tagsTA[triangle2Triangle_temp[tMin][h]] == 0) {
								GetCoordIndex(trianglesA, triangle2Triangle_temp[tMin][h], vertexT);
								for (int j = 0; j < 3; j++) {
									if (areVertexEqual(verticesA, referInput2, vertexT[0], vertexR_tnext[0 + j], compNum)
										&& areVertexEqual(verticesA, referInput2, vertexT[1], vertexR_tnext[(1 + j) % 3], compNum)
										&& areVertexEqual(verticesA, referInput2, vertexT[2], vertexR_tnext[(2 + j) % 3], compNum)) {
										nextTL.push_back(h);
										break;
									}
								}
							}
						}
					}
					if (nextTL.size() == 0) {
						nextTL.push_back(-1);
					}
					tMinL.push_back(tMin);
					nextTLists.push_back(nextTL);
				}

			}
		}
		for (int i = 0; i < tMinL.size(); i++) {
			if (referNextT != -1) {
				for (int j = 0; j < nextTLists[i].size(); j++) {
					IntVect tagsTATemp(tagsTA);
					IntMultiVect triangle2TriangleTemp(triangle2Triangle_temp);
					if (nextTLists[i][j] != -1) {
						int temp = triangle2TriangleTemp[tMinL[i]][nextTLists[i][j]];
						triangle2TriangleTemp[tMinL[i]][nextTLists[i][j]] = triangle2TriangleTemp[tMinL[i]][0];
						triangle2TriangleTemp[tMinL[i]][0] = temp;
					}
					//compute triangle FAN
					VectIntVect triangleFan(triangleFans);
					bool computeflag = false;  // compute triangleB
					computeTriangleFan(inputA, trianglesA, verticesA, referInput2, focusVertexA, tMinL[i], triangle2TriangleTemp, TLA, tagsTATemp, triangleFan, referTmin, referNextT, referInput1, computeflag, compNum);
					triangleFansList.push_back(triangleFan);
					tagsTListA.push_back(tagsTATemp);
					tagsV_B_Temp.push_back(tagsV_B_temp);
					bool remainTriangles = false;
					for (int i = 0; i < TLA.size(); i++) {
						if (tagsTATemp[TLA[i]] == 0) {
							remainTriangles = true;
							break;
						}
					}
					remainFlagList.push_back(remainTriangles);
				}
			}
			else {
				//compute triangle FAN
				VectIntVect triangleFan(triangleFans);
				IntVect tagsTATemp(tagsTA);
				bool computeflag = false; // compute triangleB
				computeTriangleFan(inputA, trianglesA, verticesA, referInput2, focusVertexA, tMinL[i], triangle2Triangle_temp, TLA, tagsTATemp, triangleFan, referTmin, referNextT, referInput1, computeflag, compNum);
				triangleFansList.push_back(triangleFan);
				tagsTListA.push_back(tagsTATemp);
				tagsV_B_Temp.push_back(tagsV_B_temp);
				bool remainTriangles = false;
				for (int i = 0; i < TLA.size(); i++) {
					if (tagsTATemp[TLA[i]] == 0) {
						remainTriangles = true;
						break;
					}
				}
				remainFlagList.push_back(remainTriangles);
			}

		}
	}
	return true;
}
bool CompareTFAN::computeTriangleFan(const Model& inputA, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, const std::vector<float>& referInput2, int focusVertexA, int tMin, IntMultiVect& triangle2Triangle_temp, std::vector<int>& TLA, std::vector<int>& tagsT_A, VectIntVect& triangleFans, int& referTmin, int& referNextT, const std::vector<int>& referInput1, bool computeflag, int& compNum) {
	int connecMin = 0;
	int nextT = 0;
	int nextTOld = 0;
	int vertexNT[3] = { -1, -1, -1 };
	int vertexNT_cmp[3] = { -1, -1, -1 };
	int vertexT[3] = { -1, -1, -1 };
	if (tMin != -1) {
		if (computeflag == true) {
			bool firstTime = true;
			tMin = -1;
			connecMin = 100000;
			//get the first triangle for building triangle fan 
			for (int i = 0; i < (int)TLA.size(); i++) {
				if (tagsT_A[TLA[i]] == 0) {
					if (connecMin > (int) triangle2Triangle_temp[TLA[i]].size()) {
						connecMin = (int)triangle2Triangle_temp[TLA[i]].size();
						tMin = TLA[i];
					}
				}
			}
			//referTmin will be referenced by inputB
			if (firstTime) {
				referTmin = tMin;
				firstTime = false;
			}
		}
		if (tMin != -1) {
			IntVect vfan;
			IntVect tfan;
			GetCoordIndex(trianglesA, tMin, vertexT);
			for (int k = 0; k < 3; k++) {
				if (vertexT[k] != focusVertexA) vfan.push_back(vertexT[k]);
			}
			nextT = tMin;
			tagsT_A[tMin] = 1; tfan.push_back(tMin);
			int nbrT = 0;
			int a = -1;
			int b = -1;
			do {
				nextTOld = nextT;
				nextT = -1;
				int candidate = 0;
				for (int h = 0; h < (int)triangle2Triangle_temp[nextTOld].size(); h++) {
					if (tagsT_A[triangle2Triangle_temp[nextTOld][h]] == 0) {
						candidate++;
					}
				}
				//candidate > 1 means non-manifest structure
				if (candidate > 1) {
					break;
				}
				//build tfan
				for (int h = 0; h < (int)triangle2Triangle_temp[nextTOld].size(); h++) {
					if (tagsT_A[triangle2Triangle_temp[nextTOld][h]] == 0) {
						GetCoordIndex(trianglesA, triangle2Triangle_temp[nextTOld][h], vertexNT);
						if (nbrT == 0) {
							if (nextT == -1 && referNextT == -1 && computeflag) {
								referNextT = triangle2Triangle_temp[nextTOld][h];     //TFAN A transfer order information
							}
							nextT = triangle2Triangle_temp[nextTOld][h];
							tagsT_A[nextT] = 1; tfan.push_back(nextT);
							if (!computeflag) {
								if (referNextT != -1) {
									GetCoordIndex(referInput1, referNextT, vertexNT_cmp);
									for (int j = 0; j < 3; j++) {
										if (areVertexEqual(verticesA, referInput2, vertexNT[0 + j], vertexNT_cmp[0], compNum)
											&& areVertexEqual(verticesA, referInput2, vertexNT[(1 + j) % 3], vertexNT_cmp[1], compNum)
											&& areVertexEqual(verticesA, referInput2, vertexNT[(2 + j) % 3], vertexNT_cmp[2], compNum)) {
											for (int k = 0; k < 3; k++) {
												int vertexNT_temp[3] = { -1, -1, -1 };
												GetCoordIndex(trianglesA, triangle2Triangle_temp[nextTOld][h], vertexNT_temp);
												vertexNT[k] = vertexNT_temp[(k + j) % 3];
											}
										}
									}
								}
							}  //handle repeat faces
							for (int k = 0; k < 3; k++) {
								if (vertexNT[k] == vfan[0]) {
									a = vfan[1];
									b = vfan[0];
									break;
								}
								if (vertexNT[k] == vfan[1]) {
									b = vfan[1];
									a = vfan[0];
									break;
								}
							}
							vfan.clear();
							vfan.push_back(a);
							vfan.push_back(b);
							nbrT++;
							int newV = -1;
							for (int k = 0; k < 3; k++) {
								if ((vertexNT[k] != b) && (vertexNT[k] != focusVertexA)) {
									newV = vertexNT[k];
									break;
								}
							}
							a = b;
							b = newV;
							vfan.push_back(b);
							break;
						}
						else {
							int found = 0;
							for (int k = 0; k < 3; k++) {
								if (vertexNT[k] == b) {
									found = 1;
									break;
								}
							}
							if (found == 1) {
								nextT = triangle2Triangle_temp[nextTOld][h];
								tagsT_A[nextT] = 1;  tfan.push_back(nextT);
								int newV = -1;
								for (int k = 0; k < 3; k++) {
									if ((vertexNT[k] != b) && (vertexNT[k] != focusVertexA)) {
										newV = vertexNT[k];
										break;
									}
								}
								a = b;
								b = newV;
								vfan.push_back(b);
							}
						}
					}
				}
			} while (nextT != -1);
			if (vfan.size() > 1) {

				int orientation = 1;
				for (int u = 0; u != 3; u++) {
					if ((trianglesA[tfan[0] * 3 + u] == focusVertexA) &&
						(trianglesA[tfan[0] * 3 + (u + 1) % 3] == vfan[0]) &&
						(trianglesA[tfan[0] * 3 + (u + 2) % 3] == vfan[1]))
					{
						orientation = 0;
					}
				}
				if (orientation == 0) {
					triangleFans.push_back(vfan);
				}
				else {
					IntVect vfanR;
					for (int i = (int)vfan.size() - 1; i >= 0; i--) {
						vfanR.push_back(vfan[i]);
					}
					triangleFans.push_back(vfanR);
				}
			}
		}
	}
	return true;
}
bool CompareTFAN::getTrianglesFansStatus(bool unoriented, const Model& inputA,
	int focusVertexA, std::vector<int>& VLA, std::vector<int>& tagsV_A, std::vector<int>& tagsV_A_temp,
	std::vector<int>& vertexMap_A, int& vertexCount_A, VectIntVect& triangleFans,
	IntVect sortedConquestedVLA, IntVect& nbrFans_A, IntVect& degree_A, IntVect& cases_A,
	IntVect& ops_A, IntVect& vertices_A)
{
	if (triangleFans.size() != 0) {
		nbrFans_A.push_back((int)triangleFans.size());
	}
	int v0A = -1;
	//	printf("#fans %i\t", (int) triangleFans.size());

	if (triangleFans.size() != 0) {
		for (int f = 0; f != triangleFans.size(); f++) {
			degree_A.push_back((int)triangleFans[f].size());
			IntVect vertices;
			IntVect ops;

			for (int k = 0; k != triangleFans[f].size(); k++) {
				v0A = triangleFans[f][k];
				bool flag = true;
				for (int i = 0; i < k; i++) {
					if (v0A == triangleFans[f][i]) {
						flag = false;
					}
				}
				if (!flag) {
					tagsV_A[v0A] = 1;
				}
				if (tagsV_A[v0A] == 0) {
					ops.push_back(0);
				}
				if (tagsV_A_temp[v0A] == 0) {
					vertexMap_A[v0A] = vertexCount_A++;
					sortedConquestedVLA.push_back(vertexMap_A[v0A]);
					VLA.push_back(v0A);
				}
				if (tagsV_A[v0A] != 0) {
					ops.push_back(1);
					int pos = 0;
					int found = 0;
					for (int u = 0; u != sortedConquestedVLA.size(); u++) {
						pos++;
						if (sortedConquestedVLA[u] == vertexMap_A[v0A]) {
							found = 1;
							break;
						}
					}
					if (found == 1) {
						vertices.push_back(-pos);  //relative index information
					}
					else {
						vertices.push_back(vertexMap_A[v0A] - vertexMap_A[focusVertexA]);
					}
				}
				if (tagsV_A_temp[v0A] == 0) {
					tagsV_A_temp[v0A] = 1;
				}
			}
			//-----------------------------------------------
			int degree = degree_A[degree_A.size() - 1];
			if (IsCase0(degree, ops, vertices)) {
				// ops: 1000001 vertices: -1 -2
				cases_A.push_back(0);
				//				printf("Case 0\t");
			}
			else if (IsCase1(degree, ops, vertices)) {
				// ops: 1xxxxxx1 vertices: -1 x x x x x -2
				for (int u = 1; u != degree - 1; u++) {
					ops_A.push_back(ops[u]);
				}
				for (int u = 1; u != vertices.size() - 1; u++) {
					vertices_A.push_back(vertices[u]);
				}
				cases_A.push_back(1);
				//				printf("Case 1\t");
			}
			else if (IsCase2(degree, ops, vertices)) {
				// ops: 00000001 vertices: -1
				cases_A.push_back(2);
				//				printf("Case 2\t");
			}
			else if (IsCase3(degree, ops, vertices)) {
				// ops: 00000001 vertices: -2
				cases_A.push_back(3);
				//				printf("Case 3\t");
			}
			else if (IsCase4(degree, ops, vertices)) {
				// ops: 10000000 vertices: -1
				cases_A.push_back(4);
				//				printf("Case 4\t");
			}
			else if (IsCase5(degree, ops, vertices)) {
				// ops: 10000000 vertices: -2
				cases_A.push_back(5);
				//				printf("Case 5\t");
			}
			else if (IsCase6(degree, ops, vertices)) {
				// ops: 00000000 vertices:
				cases_A.push_back(6);
				//				printf("Case 6\t");
			}
			else if (IsCase7(degree, ops, vertices)) {
				// ops: 1000001 vertices: -1 -2
				cases_A.push_back(7);
				//				printf("Case 7\t");
			}
			else if (IsCase8(degree, ops, vertices)) {
				// ops: 1xxxxxx1 vertices: -2 x x x x x -1
				for (int u = 1; u != degree - 1; u++) {
					ops_A.push_back(ops[u]);
				}
				for (int u = 1; u != vertices.size() - 1; u++) {
					vertices_A.push_back(vertices[u]);
				}
				cases_A.push_back(8);
				//				printf("Case 8\t");
			}
			else {
				for (int u = 0; u != degree; u++) {
					ops_A.push_back(ops[u]);
				}
				for (int u = 0; u != vertices.size(); u++) {
					vertices_A.push_back(vertices[u]);
				}
				cases_A.push_back(9);
			}
		}
	}
	return true;
}
void CompareTFAN::compareTriangleFans(const Model& inputA, const Model& inputB, bool& unoriented, const std::vector<int>& trianglesA,
	const std::vector<float>& verticesA, const std::vector<int>& trianglesB,
	const std::vector<float>& verticesB, int focusVertex, bool& found, int& vertexCountA, std::vector<int>& vertexMapA, std::vector<int>& tagsVA, std::vector<int>& tagsTA, int& vertexCountB, std::vector<int>& vertexMapB, std::vector<int>& tagsVB, std::vector<int>& tagsTB, VectIntVect& vertexMapTemList, VectIntVect& tagsVTempList, VectIntVect& tagsTTempList, IntVect& vertexCountTempList, VectIntVect& vertex2TriangleA, VectIntVect& vertex2TriangleB, int compNum) {
	int v = focusVertex;
	bool remainTrianglesA = true;
	std::stack<TFanStateInfo> stateStack;
	std::vector<VectIntVect> triangleFansListA;
	std::vector<int> TLA;
	IntVect sortedConquestedVLA;
	VectIntVect nbrFansListA;
	VectIntVect degreeListA;
	VectIntVect caseListA;
	VectIntVect opsListA;
	VectIntVect verticesListA;

	IntVect inputATMinList;
	IntVect inputANextTList;
	VectIntVect VLAList;
	vertexMapA[v] = vertexCountA++;
	int iterationTimesA = 0;
	IntVect vfanxA;
	IntVect vfanxB;
	//build triangle fans of vA
	while (remainTrianglesA) {
		IntVect nbrFansA;
		IntVect degreeA;
		IntVect caseA;
		IntVect opsA;
		IntVect vertices_A;
		IntMultiVect triangle2TriangleA_temp;
		int inputATMin = -1;
		int inputANextT = -1;
		std::vector<int> VLA;
		VLA.push_back(v);
		TLA.clear();
		processTriangle(inputA, unoriented, trianglesA, verticesA, v, tagsTA, vertexMapA, sortedConquestedVLA, TLA, triangle2TriangleA_temp, vfanxA, vertex2TriangleA, compNum);
		buildTriangleFanA(inputA, unoriented, trianglesA, verticesA, v, tagsTA, triangleFansListA, TLA, triangle2TriangleA_temp, inputATMin, inputANextT, remainTrianglesA, compNum);
		if (triangleFansListA.size() != 0) {
			getTrianglesFansStatus(unoriented, inputA, v, VLA,
				tagsVA, tagsVA, vertexMapA, vertexCountA, triangleFansListA[iterationTimesA], sortedConquestedVLA, nbrFansA, degreeA, caseA, opsA, vertices_A);
			nbrFansListA.push_back(nbrFansA);
			degreeListA.push_back(degreeA);
			caseListA.push_back(caseA);
			opsListA.push_back(opsA);
			verticesListA.push_back(vertices_A);
			VLAList.push_back(VLA);
		}
		inputATMinList.push_back(inputATMin);
		inputANextTList.push_back(inputANextT);
		iterationTimesA++;
	}
	size_t vB = 0;
	//find vB which is match with vA in inputB 
	while (!found && vB != verticesB.size() / compNum) {
		if (tagsVB[vB]) {
			++vB;
			continue;
		}
		bool flag = true; // juedge coordinate
		for (int i = 0; i < compNum; i++) {
			if (verticesB[vB * compNum + i] != verticesA[v * compNum + i])
				flag = false;
		}
		if (flag) {
			std::vector<int> TLB;
			IntMultiVect triangle2TriangleB_temp;
			std::vector<int> tagsT_B_temp(tagsTB);
			IntVect vertexMap_B_temp(vertexMapB);
			IntVect tagsV_B_temp(tagsVB);
			VectIntVect tagsV_B_Temp;
			int vertexCount_B_temp = vertexCountB;
			tagsV_B_temp[vB] = 1; vertexMap_B_temp[vB] = vertexCount_B_temp++;

			std::vector<VectIntVect> triangleFansListB;
			IntVect sortedConquestedVLB;
			VectIntVect tagsTListB;
			int iterationTimesB = 0;
			TFanStateInfo si;
			bool needBuildTFan = true;
			int pid = 0;
			bool remainTrianglesB = true;
			IntVect remainFlagList;
			while (!stateStack.empty()) {
				stateStack.pop();
			}
			//iteration times means the size of triangleFansList
			while (iterationTimesB < iterationTimesA) {
				found = false;
				if (needBuildTFan) {
					TLB.clear();
					processTriangle(inputB, unoriented, trianglesB, verticesB, vB, tagsT_B_temp, vertexMap_B_temp, sortedConquestedVLB, TLB, triangle2TriangleB_temp, vfanxB, vertex2TriangleB, compNum);
					buildTriangleFanB(inputB, unoriented, trianglesB, verticesB, vB, tagsT_B_temp, tagsV_B_temp, tagsV_B_Temp, tagsTListB, triangleFansListB, TLB, triangle2TriangleB_temp, trianglesA, verticesA, inputATMinList[iterationTimesB], inputANextTList[iterationTimesB], v, remainFlagList, compNum);
					pid = 0;
					si.iterationTimes = iterationTimesB;
					si.tFansList = triangleFansListB;
					si.tFanListID = 0;
					si.tagsTList = tagsTListB;
					si.tagsV = tagsV_B_temp;
					si.vCount = vertexCount_B_temp;
					si.vertexMap = vertexMap_B_temp;
					si.remainFlagList = remainFlagList;
				}
				if (triangleFansListB.size() != 0) {
					for (int p = pid; p < triangleFansListB.size(); p++) {
						std::vector<int> VLB;
						IntVect nbrFansB;
						IntVect degreeB;
						IntVect caseB;
						IntVect opsB;
						IntVect vertices_B;
						VLB.push_back(vB);
						remainTrianglesB = remainFlagList[p];
						getTrianglesFansStatus(unoriented, inputB, vB, VLB,
							tagsV_B_Temp[p], tagsV_B_temp, vertexMap_B_temp, vertexCount_B_temp,
							triangleFansListB[p], sortedConquestedVLB, nbrFansB, degreeB, caseB, opsB, vertices_B);
						if (areIntVecEqual(caseB, caseListA[iterationTimesB]) && areIntVecEqual(degreeB, degreeListA[iterationTimesB])
							&& areVLEqual(verticesA, verticesB, VLAList[iterationTimesB], VLB, compNum)
							&& areIntVecEqual(opsB, opsListA[iterationTimesB]) && areIntVecEqual(vertices_B, verticesListA[iterationTimesB]) && areVLEqual(verticesA, verticesB, vfanxA, vfanxB, compNum)) {
							found = true;
							tagsT_B_temp = tagsTListB[p];
							iterationTimesB++;
							needBuildTFan = true;
							si.tFanListID = p;
							stateStack.push(si);
							break;
						}
						else {
							iterationTimesB = si.iterationTimes;
							remainFlagList = si.remainFlagList;
							tagsV_B_temp = si.tagsV;
							triangleFansListB = si.tFansList;
							vertexCount_B_temp = si.vCount;
							vertexMap_B_temp = si.vertexMap;
							tagsTListB = si.tagsTList;
							found = false;
						}
					}
				}
				else {
					if (areVLEqual(verticesA, verticesB, vfanxA, vfanxB, compNum) && (triangleFansListA.size() == 0)) {
						found = true;
						iterationTimesB++;
					}
				}
				if (!found && stateStack.empty()) {
					break;
				}
				if (!found && !stateStack.empty()) {
					si = stateStack.top();
					stateStack.pop();
					iterationTimesB = si.iterationTimes;
					remainFlagList = si.remainFlagList;
					tagsV_B_temp = si.tagsV;
					triangleFansListB = si.tFansList;
					vertexCount_B_temp = si.vCount;
					vertexMap_B_temp = si.vertexMap;
					tagsTListB = si.tagsTList;
					pid = si.tFanListID + 1;
					needBuildTFan = false;
				}
			}
			if (iterationTimesB == iterationTimesA && found) {
				tagsVTempList.push_back(tagsV_B_temp);
				tagsTTempList.push_back(tagsT_B_temp);
				vertexCountTempList.push_back(vertexCount_B_temp);
				vertexMapTemList.push_back(vertexMap_B_temp);
				found = false; //find all candidate vB
			}
		}
		vfanxB.clear();
		++vB;

	}
}
bool CompareTFAN::compareConnectivity(const Model& inputA, const Model& inputB,
	bool unoriented, const std::vector<int>& trianglesA,
	const std::vector<float>& verticesA, const std::vector<int>& trianglesB,
	const std::vector<float>& verticesB, bool earlyReturn, int compNum, size_t& diffs) {

	VectIntVect vertex2VertexA; VectIntVect vertex2VertexB;
	VectIntVect vertex2TriangleA; VectIntVect vertex2TriangleB;
	VectIntVect triangle2TriangleA; VectIntVect triangle2TriangleB;

	vertex2VertexA.clear(); vertex2TriangleA.clear(); triangle2TriangleA.clear();
	vertex2VertexB.clear(); vertex2TriangleB.clear(); triangle2TriangleB.clear();

	ComputeAdjacency(inputA, inputB, trianglesA, verticesA, trianglesB, verticesB, vertex2VertexA, vertex2VertexB, vertex2TriangleA, vertex2TriangleB, triangle2TriangleA, triangle2TriangleB, compNum); 

	unsigned int maxV2TA = 0;
	for (int v = 0; v != verticesA.size() / compNum; v++) {
		if (maxV2TA < vertex2TriangleA[v].size()) maxV2TA = (unsigned int)vertex2TriangleA[v].size();
	}
	unsigned int maxV2TB = 0;
	for (int v = 0; v != verticesB.size() / compNum; v++) {
		if (maxV2TB < vertex2TriangleB[v].size()) maxV2TB = (unsigned int)vertex2TriangleB[v].size();
	}
	if (maxV2TA != maxV2TB) {
		++diffs;
		std::cout << "meshes are not equal, maxV2TA_ != maxV2TB_ " << std::endl;
		return true;
	}
	std::vector<int> tagsTA(trianglesA.size() / 3, 0);//the triangle traverse information
	std::vector<int> vertexMapA(verticesA.size() / compNum, -1);//vertex traversal order
	std::vector<int> tagsVA(verticesA.size() / compNum, 0);//the vertices traverse information
	std::vector<int> repateProcessVertex;

	bool test = true;
	std::vector<int> tagsTB(trianglesB.size() / 3, 0);
	std::vector<int> vertexMapB(verticesB.size() / compNum, -1);
	std::vector<int> tagsVB(verticesB.size() / compNum, 0);

	int vertexCountA = 0;// number of traversed vertices
	int vertexCountB = 0;

	for (int v = 0; v < verticesA.size() / compNum; v++) {
		if (tagsVA[v] == 0) {
			// mesh state information
			CompareStateInfo compareStateA;
			compareStateA.tagsTA = tagsTA;
			compareStateA.tagsVA = tagsVA;
			compareStateA.vCountA = vertexCountA;
			compareStateA.vertexMapA = vertexMapA;

			tagsVA[v] = 1;
			bool found = false;
			VectIntVect vertexMapTemList;
			VectIntVect tagsVTempList;
			VectIntVect tagsTTempList;
			IntVect vertexCountTempList;
			// build TFAN, find matching vertex
			compareTriangleFans(inputA, inputB, unoriented, trianglesA, verticesA, trianglesB, verticesB, v, found, vertexCountA, vertexMapA, tagsVA, tagsTA,
				vertexCountB, vertexMapB, tagsVB, tagsTB, vertexMapTemList, tagsVTempList, tagsTTempList, vertexCountTempList, vertex2TriangleA, vertex2TriangleB, compNum);
			// no matching vertex, mesh is not equal
			if (!tagsTTempList.size()) {
				++diffs;
				if (earlyReturn) {
					if (compNum == 3) {
						std::cout << "the geometry connectivity of meshes is not equal, early return." << std::endl;
						return true;
					}
					else if (compNum == 2) {
						std::cout << "the texture connectivity of meshes is not equal, early return." << std::endl;
						return true;
					}
				}
			}
			if (tagsTTempList.size()) {
				if (tagsTTempList.size() > 1) { // vA is duplicate matching points

					tagsTA = compareStateA.tagsTA;
					tagsVA = compareStateA.tagsVA;
					vertexCountA = compareStateA.vCountA;
					vertexMapA = compareStateA.vertexMapA;
					// final processing
					repateProcessVertex.push_back(v);
				}
				//one matching points
				else {
					tagsTB = tagsTTempList[0];
					tagsVB = tagsVTempList[0];
					vertexMapB = vertexMapTemList[0];
					vertexCountB = vertexCountTempList[0];
				}
			}
		}
	}

	// handle duplicate matching points
	std::stack<CompareStateInfo> CSIStack;
	for (int i = 0; i < repateProcessVertex.size(); i++) {
		int v = repateProcessVertex[i];
		if (tagsVA[v] == 0) {
			int v0 = v;
			tagsVA[v0] = 1;
			bool found = false;

			VectIntVect vertexMapTemList;
			VectIntVect tagsVTempList;
			VectIntVect tagsTTempList;
			IntVect vertexCountTempList;

			compareTriangleFans(inputA, inputB, unoriented, trianglesA, verticesA, trianglesB, verticesB, v, found, vertexCountA, vertexMapA, tagsVA, tagsTA,
				vertexCountB, vertexMapB, tagsVB, tagsTB, vertexMapTemList, tagsVTempList, tagsTTempList,
				vertexCountTempList, vertex2TriangleA, vertex2TriangleB, compNum);

			if (!tagsTTempList.size()) {
				if (CSIStack.size() == 0) {
					++diffs;
					if (earlyReturn) {
						if (compNum == 3) {
							std::cout << "the geometry connectivity of meshes is not equal, early return." << std::endl;
							return true;
						}
						else if (compNum == 2) {
							std::cout << "the texture connectivity of meshes is not equal, early return." << std::endl;
							return true;
						}
					}
				}
				else {
					CompareStateInfo csi = CSIStack.top();
					CSIStack.pop();
					tagsTA = csi.tagsTA;
					tagsVA = csi.tagsVA;
					vertexMapA = csi.vertexMapA;
					vertexCountA = csi.vCountA;

					tagsTB = csi.tagsTB;
					tagsVB = csi.tagsVB;
					vertexMapB = csi.vertexMapB;
					vertexCountB = csi.vCountB;

					v = csi.vA;
				}

			}
			if (tagsTTempList.size()) {
				if (tagsTTempList.size() > 1) {
					for (int i = 1; i < tagsTTempList.size(); i++) {
						CompareStateInfo csi;
						csi.tagsTA = tagsTA;
						csi.tagsVA = tagsVA;
						csi.vertexMapA = vertexMapA;
						csi.vCountA = vertexCountA;

						csi.tagsTB = tagsTTempList[i];
						csi.tagsVB = tagsVTempList[i];
						csi.vertexMapB = vertexMapTemList[i];
						csi.vCountB = vertexCountTempList[i];

						csi.vA = v;
						CSIStack.push(csi);
					}
				}
				tagsTB = tagsTTempList[0];
				tagsVB = tagsVTempList[0];
				vertexMapB = vertexMapTemList[0];
				vertexCountB = vertexCountTempList[0];
			}
		}
	}

	if (diffs == 0) {
		if (compNum == 3) {
			std::cout << "the geometry connectivity of meshes is equal" << std::endl;
			return true;
		}
		else if (compNum == 2) {
			std::cout << "the texture connectivity of meshes is equal" << std::endl;
			return true;
		}
	}
	return true;
}

void CompareTFAN::ComputeVertex2Vertex(const Model& inputA, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, VectIntVect& vertex2VertexA, int compNum) {
	int m_nVertices = verticesA.size() / compNum;
	int m_nTriangles = trianglesA.size() / 3;
	vertex2VertexA.resize(m_nVertices);
	for (int v = 0; v < m_nVertices; v++)
	{
		vertex2VertexA[v].reserve(20);  //the adjacency of vertex to vertex
	}
	for (int f = 0; f < m_nTriangles; f++) {
		//		printf("%i \t %i \t %i\n", m_triangles[f*3+0], m_triangles[f*3+1], m_triangles[f*3+2]);
		AddNeighborVertex2Vertex(inputA, vertex2VertexA, trianglesA[f * 3 + 0], trianglesA[f * 3 + 1]);
		AddNeighborVertex2Vertex(inputA, vertex2VertexA, trianglesA[f * 3 + 0], trianglesA[f * 3 + 2]);

		AddNeighborVertex2Vertex(inputA, vertex2VertexA, trianglesA[f * 3 + 1], trianglesA[f * 3 + 0]);
		AddNeighborVertex2Vertex(inputA, vertex2VertexA, trianglesA[f * 3 + 1], trianglesA[f * 3 + 2]);

		AddNeighborVertex2Vertex(inputA, vertex2VertexA, trianglesA[f * 3 + 2], trianglesA[f * 3 + 0]);
		AddNeighborVertex2Vertex(inputA, vertex2VertexA, trianglesA[f * 3 + 2], trianglesA[f * 3 + 1]);
	}
}

void CompareTFAN::ComputeVertex2Triangle(const Model& inputA, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, VectIntVect& vertex2TriangleA, int compNum) {
	int m_nVertices = verticesA.size() / compNum;
	int m_nTriangles = trianglesA.size() / 3;
	vertex2TriangleA.resize(m_nVertices);
	for (int v = 0; v < m_nVertices; v++)
	{
		vertex2TriangleA[v].reserve(20);  //the adjacency of vertex to triangle
	}
	for (int f = 0; f < m_nTriangles; f++) {
		AddNeighborVertex2Triangle(inputA, vertex2TriangleA, trianglesA[f * 3 + 0], f);
		AddNeighborVertex2Triangle(inputA, vertex2TriangleA, trianglesA[f * 3 + 1], f);
		AddNeighborVertex2Triangle(inputA, vertex2TriangleA, trianglesA[f * 3 + 2], f);
	}
}
void CompareTFAN::ComputeTriangle2Triangle(const Model& inputA, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, VectIntVect& triangle2TriangleA, VectIntVect& vertex2TriangleA) {
	int m_nTriangles = trianglesA.size() / 3;
	triangle2TriangleA.resize(m_nTriangles);
	for (int t = 0; t < m_nTriangles; t++)
	{
		triangle2TriangleA[t].reserve(20);  //the adjacency of triangle to triangle
	}
	int coordIndex[3] = { -1, -1, -1 };
	for (int t = 0; t < m_nTriangles; t++) {
		GetCoordIndex(trianglesA, t, coordIndex);
		for (int k = 0; k < 3; k++) {
			for (IntVect::iterator pt0 = vertex2TriangleA[coordIndex[k]].begin(); pt0 != vertex2TriangleA[coordIndex[k]].end(); ++pt0) {
				if ((*pt0) != t) {
					int tr0 = (*pt0);
					for (int i = 0; i < (int)vertex2TriangleA[coordIndex[(k + 1) % 3]].size(); i++) {
						int tr1 = vertex2TriangleA[coordIndex[(k + 1) % 3]][i];
						if (tr1 == tr0) {
							AddNeighborTriangle2Triangle(inputA, triangle2TriangleA, t, tr1);
						}
					}
				}
			}
		}
	}
}void CompareTFAN::ComputeAdjacency(const Model& inputA, const Model& inputB, const std::vector<int>& trianglesA, const std::vector<float>& verticesA, const std::vector<int>& trianglesB, const std::vector<float>& verticesB, VectIntVect& vertex2VertexA, VectIntVect& vertex2VertexB, VectIntVect& vertex2TriangleA, VectIntVect& vertex2TriangleB, VectIntVect& triangle2TriangleA, VectIntVect& triangle2TriangleB, int compNum) {
	ComputeVertex2Vertex(inputA, trianglesA, verticesA, vertex2VertexA, compNum);
	ComputeVertex2Triangle(inputA, trianglesA, verticesA, vertex2TriangleA, compNum);
	ComputeTriangle2Triangle(inputA, trianglesA, verticesA, triangle2TriangleA, vertex2TriangleA);
	ComputeVertex2Vertex(inputB, trianglesB, verticesB, vertex2VertexB, compNum);
	ComputeVertex2Triangle(inputB, trianglesB, verticesB, vertex2TriangleB, compNum);
	ComputeTriangle2Triangle(inputB, trianglesB, verticesB, triangle2TriangleB, vertex2TriangleB);
}

