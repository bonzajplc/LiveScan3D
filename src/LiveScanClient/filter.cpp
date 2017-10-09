//   Copyright (C) 2015  Marek Kowalski (M.Kowalski@ire.pw.edu.pl), Jacek Naruniec (J.Naruniec@ire.pw.edu.pl)
//   License: MIT Software License   See LICENSE.txt for the full license.

//   If you use this software in your research, then please use the following citation:

//    Kowalski, M.; Naruniec, J.; Daniluk, M.: "LiveScan3D: A Fast and Inexpensive 3D Data
//    Acquisition System for Multiple Kinect v2 Sensors". in 3D Vision (3DV), 2015 International Conference on, Lyon, France, 2015

//    @INPROCEEDINGS{Kowalski15,
//        author={Kowalski, M. and Naruniec, J. and Daniluk, M.},
//        booktitle={3D Vision (3DV), 2015 International Conference on},
//        title={LiveScan3D: A Fast and Inexpensive 3D Data Acquisition System for Multiple Kinect v2 Sensors},
//        year={2015},
//    }
#include "filter.h"
#include <set>
#include "VectorFloat.h"

using namespace std;



vector<KNNeighborsResult> KNNeighbors(PointCloud &cloud, kdTree &tree, int k)
{
	vector<KNNeighborsResult> result(cloud.pts.size());
	int nCloudPts = static_cast<int>(cloud.pts.size());

#pragma omp parallel for
	for (int i = 0; i < nCloudPts; i++)
	{
		result[i].neighbors.resize(k);
		result[i].distances.resize(k);
		tree.knnSearch((float*)(cloud.pts.data() + i), k, (size_t*)result[i].neighbors.data(), result[i].distances.data());
		result[i].kDistance = result[i].distances[k - 1];
	}

	return result;
}

void filter(std::vector<Point3f> &vertices, std::vector<Point3f> &normals, std::vector<Point2f> &uvs, std::vector<RGB> &colors, int k, float maxDist)
{
	if (k <= 0 || maxDist <= 0)
		return;

	PointCloud cloud;
	cloud.pts = vertices;

	kdTree tree(3, cloud);
	tree.buildIndex();

	vector<KNNeighborsResult> knn = KNNeighbors(cloud, tree, k);
	
	vector<int> indicesToRemove;

	float distThreshold = pow(maxDist, 2);
	for (unsigned int i = 0; i < cloud.pts.size(); i++)
	{
		if (knn[i].kDistance > distThreshold)
			indicesToRemove.push_back(i);
	}

	int lastElemIdx = 0;
	unsigned int idxToCheck = 0;
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		if (idxToCheck < indicesToRemove.size() && i == indicesToRemove[idxToCheck])
		{
			idxToCheck++;
			continue;
		}
		vertices[lastElemIdx] = vertices[i];
		normals[lastElemIdx] = normals[i];
		uvs[lastElemIdx] = uvs[i];
		colors[lastElemIdx] = colors[i];

		lastElemIdx++;
	}

	vertices.resize( lastElemIdx );
	colors.resize( lastElemIdx );
	normals.resize( lastElemIdx );
	uvs.resize( lastElemIdx );
}

void medianFilter( std::vector<Point3f> &vertices, std::vector<Point3f> &normals, std::vector<unsigned short> &indices, int k, float maxDist )
{
	if( k <= 0 )
		return;

	PointCloud cloud;
	cloud.pts = vertices;

	kdTree tree( 3, cloud );
	tree.buildIndex();

	vector<KNNeighborsResult> knn = KNNeighbors( cloud, tree, k );

	for( unsigned int i = 0; i < cloud.pts.size(); i++ )
	{
		set<float> medianList;
		float average = 0.0f;
		int num = 0;

		for( int j = 0; j < knn[i].neighbors.size(); j++ )
		{
			if( knn[i].distances[j] <= maxDist )
			{
				medianList.insert( vertices[knn[i].neighbors[j]].Z );
				//average += vertices[knn[i].neighbors[j]].Z;
				//num++;
			}
		}

		float median = *std::next( medianList.begin(), medianList.size() / 2 );

		//if(num > 0 )
		vertices[i].Z = median;
	}

	//calculate normals
	vector< uint8_t > normalCounter( vertices.size(), 0 );

	auto calculateNormal = [&]( unsigned short ind, const Vec3F &normal )
	{
		if( normalCounter[ind] == 0 )
			normals[ind] = toPoint3f( normal );
		else
		{
			float lerpVal = 1.0f - (float)normalCounter[ind] / (float)( normalCounter[ind] + 1 );
			normals[ind] = toPoint3f( lerp( lerpVal, toVec3F( normals[ind] ), normal ) );
		}

		normalCounter[ind]++;
	};

	for( int i = 0; i < indices.size(); i += 3 )
	{
		unsigned short ind0 = indices[i + 0];
		unsigned short ind1 = indices[i + 1];
		unsigned short ind2 = indices[i + 2];

		Vec3F v1 = toVec3F( vertices[ind1] ) - toVec3F( vertices[ind0] );
		Vec3F v2 = toVec3F( vertices[ind2] ) - toVec3F( vertices[ind0] );

		Vec3F normal = cross( normalize( v1 ), normalize( v2 ) );

		calculateNormal( ind0, normal );
		calculateNormal( ind1, normal );
		calculateNormal( ind2, normal );
	}

	for( int i = 0; i < normals.size(); i += 3 )
	{
		float lengthSquared = normals[i].X * normals[i].X + normals[i].Y * normals[i].Y + normals[i].Z * normals[i].Z;

		if( lengthSquared > 0.0f )
		{
			float lengthRcp = 1.0f / sqrt( lengthSquared );
			normals[i] = Point3f( normals[i].X * lengthRcp, normals[i].Y * lengthRcp, normals[i].Z * lengthRcp );
		}
	}
}
