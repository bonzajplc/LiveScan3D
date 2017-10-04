#include "frameFileWriterReader.h"

#include <ctime>

FrameFileWriterReader::FrameFileWriterReader()
{

}

void FrameFileWriterReader::closeFileIfOpened()
{
	if (m_pFileHandle == nullptr)
		return;

	fclose(m_pFileHandle);
	m_pFileHandle = nullptr; 
	m_bFileOpenedForReading = false;
	m_bFileOpenedForWriting = false;
}

void FrameFileWriterReader::resetTimer()
{
	recording_start_time = std::chrono::steady_clock::now();
}

int FrameFileWriterReader::getRecordingTimeMilliseconds()
{
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	return static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds >(end - recording_start_time).count());
}

void FrameFileWriterReader::openCurrentFileForReading()
{
	closeFileIfOpened();

	m_pFileHandle = fopen(m_sFilename.c_str(), "rb");

	m_bFileOpenedForReading = true;
	m_bFileOpenedForWriting = false;
}

void FrameFileWriterReader::openNewFileForWriting()
{
	closeFileIfOpened();

	char filename[1024];
	time_t t = time(0);
	struct tm * now = localtime(&t);
	sprintf(filename, "recording_%04d_%02d_%02d_%02d_%02d.bin", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);
	m_sFilename = filename; 
	m_pFileHandle = fopen(filename, "wb");

	m_bFileOpenedForReading = false;
	m_bFileOpenedForWriting = true;

	resetTimer();
}

bool FrameFileWriterReader::readFrame( std::vector<Point3s> &outPoints, std::vector<Point2s> &outNormals, std::vector<Point2s> &outUVs, std::vector<RGB> &outColors, std::vector<unsigned short> &outIndices )
{
	if (!m_bFileOpenedForReading)
		openCurrentFileForReading();

	outPoints.clear();
	outNormals.clear();
	outUVs.clear();
	outColors.clear();
	outIndices.clear();

	FILE *f = m_pFileHandle;
	int nPoints, nIndices, timestamp; 
	char tmp[1024]; 
	int nread = fscanf_s(f, "%s %d %s %d %s %d", tmp, 1024, &nPoints, tmp, 1024, &nIndices, tmp, 1024, &timestamp);

	if (nread < 4)
		return false;

	if (nPoints == 0)
		return true;

	fgetc(f);		//  '\n'
	outPoints.resize(nPoints);
	outNormals.resize( nPoints );
	outUVs.resize( nPoints );
	outColors.resize(nPoints);

	fread( (void*)outPoints.data(), sizeof( outPoints[0] ), nPoints, f );
	fread( (void*)outNormals.data(), sizeof( outNormals[0] ), nPoints, f );
	fread( (void*)outUVs.data(), sizeof( outUVs[0] ), nPoints, f );
	fread( (void*)outColors.data(), sizeof( outColors[0] ), nPoints, f );

	outIndices.resize( nIndices );
	fread( (void*)outIndices.data(), sizeof( outIndices[0] ), nIndices, f );
	fgetc(f);		// '\n'

	return true;
}


void FrameFileWriterReader::writeFrame( std::vector<Point3s> points, std::vector<Point2s> normals, std::vector<Point2s> uvs, std::vector<RGB> colors, std::vector<unsigned short> indices )
{
	if (!m_bFileOpenedForWriting)
		openNewFileForWriting();

	FILE *f = m_pFileHandle;

	int nPoints = static_cast<int>( points.size() );
	int nIndices = static_cast<int>( indices.size() );

	fprintf(f, "n_points= %d\nn_indices= %d\nframe_timestamp= %d\n", nPoints, nIndices, getRecordingTimeMilliseconds());
	if (nPoints > 0)
	{
		fwrite( (void*)points.data(), sizeof( points[0] ), nPoints, f );
		fwrite( (void*)normals.data(), sizeof( normals[0] ), nPoints, f );
		fwrite( (void*)uvs.data(), sizeof( uvs[0] ), nPoints, f );
		fwrite( (void*)colors.data(), sizeof( colors[0] ), nPoints, f );
	}
	if( nIndices > 0 )
	{
		fwrite( (void*)indices.data(), sizeof( indices[0] ), nIndices, f );
	}
	fprintf(f, "\n");
}

FrameFileWriterReader::~FrameFileWriterReader()
{
	closeFileIfOpened();
}