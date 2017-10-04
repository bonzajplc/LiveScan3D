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
#pragma once

#include "resource.h"
#include "ImageRenderer.h"
#include "SocketCS.h"
#include "calibration.h"
#include "utils.h"
#include "KinectCapture.h"
#include "frameFileWriterReader.h"
#include <thread>
#include <mutex>

class LiveScanClient
{
public:
    LiveScanClient();
    ~LiveScanClient();


    static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK        DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    int                     Run(HINSTANCE hInstance, int nCmdShow);

	bool m_bSocketThread;
private:
	Calibration calibration;

	bool m_bCalibrate;
	bool m_bFilter;
	bool m_bStreamOnlyBodies;

	ICapture *pCapture;

	int m_nFilterNeighbors;
	float m_fFilterThreshold;

	bool m_bCaptureFrame;
	bool m_bConnected;
	bool m_bConfirmCaptured;
	bool m_bConfirmCalibrated;
	bool m_bShowDepth;
	bool m_bFrameCompression;
	int m_iCompressionLevel;

	FrameFileWriterReader m_framesFileWriterReader;


	SocketClient *m_pClientSocket;
	std::vector<float> m_vBounds;

	std::vector<Point3s> m_vLastFrameVertices;
	std::vector<Point2s> m_vLastFrameNormals;
	std::vector<Point2s> m_vLastFrameUVs;
	std::vector<RGB> m_vLastFrameRGB;
	std::vector<unsigned short> m_vLastFrameIndices;
	std::vector<Body> m_vLastFrameBody;

	HWND m_hWnd;
    INT64 m_nLastCounter;
    double m_fFreq;
    INT64 m_nNextStatusTime;
    DWORD m_nFramesSinceUpdate;	

	Point3f* m_current_pCameraSpaceCoordinates = NULL;
	Point2f* m_current_pColorCoordinatesOfDepth = NULL;

	Point3f* m_next_pCameraSpaceCoordinates = NULL;
	Point2f* m_next_pColorCoordinatesOfDepth = NULL;
	Point2f* m_next_pDepthCoordinatesOfColor = NULL;
	
	// Direct2D
    ImageRenderer* m_pDrawColor;
    ID2D1Factory* m_pD2DFactory;
	RGB* m_pDepthRGBX;

	void UpdateFrame();
    void ProcessColor(RGB* pBuffer, int nWidth, int nHeight);
	void ProcessDepth(const UINT16* pBuffer, int nHeight, int nWidth);

    bool SetStatusMessage(_In_z_ WCHAR* szMessage, DWORD nShowTimeMsec, bool bForce);

	void HandleSocket();
	void SendFrame(vector<Point3s> vertices, vector<Point2s> normals, vector<Point2s> uvs, vector<RGB> RGB, vector<unsigned short> indices, vector<Body> body);

	void SocketThreadFunction();
	void StoreFrame( BYTE* prevBodyIndex, Point3f *currentVertices, BYTE* currentBodyIndex, BYTE* nextBodyIndex, Point2f *currentMapping, RGB *currentColor, vector<Body> &currentBodies );
	void ShowFPS();
	void ReadIPFromFile();
	void WriteIPToFile();
};

