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
#include "stdafx.h"
#include "resource.h"
#include "LiveScanClient.h"
#include "filter.h"
#include <chrono>
#include <strsafe.h>
#include <fstream>
#include "zstd.h"
#include "VectorFloat.h"

std::mutex m_mSocketThreadMutex;

int APIENTRY wWinMain(    
	_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd
    )
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LiveScanClient application;
    application.Run(hInstance, nShowCmd);
}

LiveScanClient::LiveScanClient() :
    m_hWnd(NULL),
    m_nLastCounter(0),
    m_nFramesSinceUpdate(0),
    m_fFreq(0),
    m_nNextStatusTime(0LL),
    m_pD2DFactory(NULL),
    m_pDrawColor(NULL),
	m_pDepthRGBX(NULL),
	m_bCalibrate(false),
	m_bFilter(false),
	m_bStreamOnlyBodies(false),
	m_bCaptureFrame(false),
	m_bConnected(false),
	m_bConfirmCaptured(false),
	m_bConfirmCalibrated(false),
	m_bShowDepth(false),
	m_bSocketThread(true),
	m_bFrameCompression(true),
	m_iCompressionLevel(2),
	m_pClientSocket(NULL),
	m_nFilterNeighbors(10),
	m_fFilterThreshold(0.01f)
{
	pCapture = new KinectCapture();

    LARGE_INTEGER qpf = {0};
    if (QueryPerformanceFrequency(&qpf))
    {
        m_fFreq = double(qpf.QuadPart);
    }

	m_vBounds.push_back(-0.5);
	m_vBounds.push_back(-0.5);
	m_vBounds.push_back(-0.5);
	m_vBounds.push_back(0.5);
	m_vBounds.push_back(0.5);
	m_vBounds.push_back(0.5);

	calibration.LoadCalibration();
}
  
LiveScanClient::~LiveScanClient()
{
    // clean up Direct2D renderer
    if (m_pDrawColor)
    {
        delete m_pDrawColor;
        m_pDrawColor = NULL;
    }

	if (pCapture)
	{
		delete pCapture;
		pCapture = NULL;
	}

	if (m_pDepthRGBX)
	{
		delete[] m_pDepthRGBX;
		m_pDepthRGBX = NULL;
	}

	if( m_current_pCameraSpaceCoordinates )
	{
		delete[] m_current_pCameraSpaceCoordinates;
		m_current_pCameraSpaceCoordinates = NULL;
	}

	if( m_current_pColorCoordinatesOfDepth )
	{
		delete[] m_current_pColorCoordinatesOfDepth;
		m_current_pColorCoordinatesOfDepth = NULL;
	}

	if( m_next_pCameraSpaceCoordinates )
	{
		delete[] m_next_pCameraSpaceCoordinates;
		m_next_pCameraSpaceCoordinates = NULL;
	}

	if( m_next_pColorCoordinatesOfDepth )
	{
		delete[] m_next_pColorCoordinatesOfDepth;
		m_next_pColorCoordinatesOfDepth = NULL;
	}

	if( m_next_pDepthCoordinatesOfColor )
	{
		delete[] m_next_pDepthCoordinatesOfColor;
		m_next_pDepthCoordinatesOfColor = NULL;
	}

	if( vertexIndexTable )
	{
		delete[] vertexIndexTable;
		vertexIndexTable = NULL;
	}

	if (m_pClientSocket)
	{
		delete m_pClientSocket;
		m_pClientSocket = NULL;
	}
    // clean up Direct2D
    SafeRelease(m_pD2DFactory);
}

int LiveScanClient::Run(HINSTANCE hInstance, int nCmdShow)
{
    MSG       msg = {0};
    WNDCLASS  wc;

	// Dialog custom window class
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.lpfnWndProc   = DefDlgProcW;
    wc.lpszClassName = L"LiveScanClientAppDlgWndClass";

    if (!RegisterClassW(&wc))
    {
        return 0;
    }

    // Create main application window
	HWND hWndApp = CreateDialogParamW(
        NULL,
        MAKEINTRESOURCE(IDD_APP),
        NULL,
        (DLGPROC)LiveScanClient::MessageRouter, 
        reinterpret_cast<LPARAM>(this));

    // Show window
    ShowWindow(hWndApp, nCmdShow);

	std::thread t1(&LiveScanClient::SocketThreadFunction, this);
    // Main message loop
    while (WM_QUIT != msg.message)
    {
		//HandleSocket();
		UpdateFrame();

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // If a dialog message will be taken care of by the dialog proc
            if (hWndApp && IsDialogMessageW(hWndApp, &msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

	m_bSocketThread = false;
	t1.join();
    return static_cast<int>(msg.wParam);
}



void LiveScanClient::UpdateFrame()
{
	if (!pCapture->bInitialized)
	{
		return;
	}

	//copy next to current
	memcpy( m_current_pCameraSpaceCoordinates, m_next_pCameraSpaceCoordinates, sizeof( Point3f ) * pCapture->nDepthFrameWidth * pCapture->nDepthFrameHeight );
	memcpy( m_current_pColorCoordinatesOfDepth, m_next_pColorCoordinatesOfDepth, sizeof( Point2f ) * pCapture->nDepthFrameWidth * pCapture->nDepthFrameHeight );

	//acquire next frame
	bool bNewFrameAcquired = pCapture->AcquireFrame();

	if (!bNewFrameAcquired)
		return;

	pCapture->MapDepthFrameToCameraSpace(m_next_pCameraSpaceCoordinates);
	pCapture->MapDepthFrameToColorSpace(m_next_pColorCoordinatesOfDepth);

	{
		std::lock_guard<std::mutex> lock(m_mSocketThreadMutex);
		StoreFrame( pCapture->p_prev_BodyIndex, m_current_pCameraSpaceCoordinates, pCapture->p_current_BodyIndex, pCapture->p_next_BodyIndex,
					m_current_pColorCoordinatesOfDepth, pCapture->p_current_ColorRGBX, pCapture->v_current_Bodies );

		if (m_bCaptureFrame)
		{
			m_framesFileWriterReader.writeFrame( m_vLastFrameVertices, m_vLastFrameNormals, m_vLastFrameUVs, m_vLastFrameRGB, m_vLastFrameIndices );
			m_bConfirmCaptured = true;
			m_bCaptureFrame = false;
		}
	}

	if (m_bCalibrate)
	{		
		std::lock_guard<std::mutex> lock(m_mSocketThreadMutex);
		Point3f *pCameraCoordinates = new Point3f[pCapture->nColorFrameWidth * pCapture->nColorFrameHeight];
		pCapture->MapColorFrameToCameraSpace(pCameraCoordinates);

		bool res = calibration.Calibrate(pCapture->p_next_ColorRGBX, pCameraCoordinates, pCapture->nColorFrameWidth, pCapture->nColorFrameHeight);

		delete[] pCameraCoordinates;

		if (res)
		{
			m_bConfirmCalibrated = true;
			m_bCalibrate = false;
		}
	}

	if (!m_bShowDepth)
		ProcessColor(pCapture->p_next_ColorRGBX, pCapture->nColorFrameWidth, pCapture->nColorFrameHeight);
	else
		ProcessDepth(pCapture->p_next_Depth, pCapture->nDepthFrameWidth, pCapture->nDepthFrameHeight);

	ShowFPS();
}

LRESULT CALLBACK LiveScanClient::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LiveScanClient* pThis = NULL;
    
    if (WM_INITDIALOG == uMsg)
    {
        pThis = reinterpret_cast<LiveScanClient*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<LiveScanClient*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pThis)
    {
        return pThis->DlgProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

LRESULT CALLBACK LiveScanClient::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Bind application window handle
            m_hWnd = hWnd;

            // Init Direct2D
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

            // Get and initialize the default Kinect sensor
			bool res = pCapture->Initialize();
			if (res)
			{
				m_pDepthRGBX = new RGB[pCapture->nColorFrameWidth * pCapture->nColorFrameHeight];
				
				m_current_pCameraSpaceCoordinates = new Point3f[pCapture->nDepthFrameWidth * pCapture->nDepthFrameHeight];
				m_current_pColorCoordinatesOfDepth = new Point2f[pCapture->nDepthFrameWidth * pCapture->nDepthFrameHeight];

				m_next_pCameraSpaceCoordinates = new Point3f[pCapture->nDepthFrameWidth * pCapture->nDepthFrameHeight];
				m_next_pColorCoordinatesOfDepth = new Point2f[pCapture->nDepthFrameWidth * pCapture->nDepthFrameHeight];
				m_next_pDepthCoordinatesOfColor = new Point2f[pCapture->nColorFrameWidth * pCapture->nColorFrameHeight];

				vertexIndexTable = new int[pCapture->nDepthFrameWidth * pCapture->nDepthFrameHeight];
			}
			else
			{
				SetStatusMessage(L"Capture device failed to initialize!", 10000, true);
			}

			// Create and initialize a new Direct2D image renderer (take a look at ImageRenderer.h)
			// We'll use this to draw the data we receive from the Kinect to the screen
			HRESULT hr;
			m_pDrawColor = new ImageRenderer();
			hr = m_pDrawColor->Initialize(GetDlgItem(m_hWnd, IDC_VIDEOVIEW), m_pD2DFactory, pCapture->nColorFrameWidth, pCapture->nColorFrameHeight, pCapture->nColorFrameWidth * sizeof(RGB));
			if (FAILED(hr))
			{
				SetStatusMessage(L"Failed to initialize the Direct2D draw device.", 10000, true);
			}

			ReadIPFromFile();
        }
        break;

        // If the titlebar X is clicked, destroy app
		case WM_CLOSE:	
			WriteIPToFile();
			DestroyWindow(hWnd);						 
			break;
        case WM_DESTROY:
            // Quit the main message pump
            PostQuitMessage(0);
            break;
			
        // Handle button press
        case WM_COMMAND:
			if (IDC_BUTTON_CONNECT == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
			{
				std::lock_guard<std::mutex> lock(m_mSocketThreadMutex);
				if (m_bConnected)
				{
					delete m_pClientSocket;
					m_pClientSocket = NULL;

					m_bConnected = false;
					SetDlgItemTextA(m_hWnd, IDC_BUTTON_CONNECT, "Connect");
				}
				else
				{
					try
					{
						char address[20];
						GetDlgItemTextA(m_hWnd, IDC_IP, address, 20);
						m_pClientSocket = new SocketClient(address, 48001);

						m_bConnected = true;
						if (calibration.bCalibrated)
							m_bConfirmCalibrated = true;

						SetDlgItemTextA(m_hWnd, IDC_BUTTON_CONNECT, "Disconnect");
						//Clear the status bar so that the "Failed to connect..." disappears.
						SetStatusMessage(L"", 1, true);
					}
					catch (...)
					{
						SetStatusMessage(L"Failed to connect. Did you start the server?", 10000, true);
					}
				}
			}
			if (IDC_BUTTON_SWITCH == LOWORD(wParam) && BN_CLICKED == HIWORD(wParam))
			{
				m_bShowDepth = !m_bShowDepth;

				if (m_bShowDepth)
				{
					SetDlgItemTextA(m_hWnd, IDC_BUTTON_SWITCH, "Show color");
				}
				else
				{
					SetDlgItemTextA(m_hWnd, IDC_BUTTON_SWITCH, "Show depth");
				}
			}
            break;
    }

    return FALSE;
}

void LiveScanClient::ProcessDepth(const UINT16* pBuffer, int nWidth, int nHeight)
{
	// Make sure we've received valid data
	if (m_pDepthRGBX && m_next_pDepthCoordinatesOfColor && pBuffer && (nWidth == pCapture->nDepthFrameWidth) && (nHeight == pCapture->nDepthFrameHeight))
	{
		// end pixel is start + width*height - 1
		const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);

		pCapture->MapColorFrameToDepthSpace( m_next_pDepthCoordinatesOfColor );

		for (int i = 0; i < pCapture->nColorFrameWidth * pCapture->nColorFrameHeight; i++)
		{
			Point2f depthPoint = m_next_pDepthCoordinatesOfColor[i];
			BYTE intensity = 0;
			
			if (depthPoint.X >= 0 && depthPoint.Y >= 0)
			{
				int depthIdx = (int)(depthPoint.X + depthPoint.Y * pCapture->nDepthFrameWidth);
				USHORT depth = pBuffer[depthIdx];
				intensity = static_cast<BYTE>(depth % 256);
			}

			m_pDepthRGBX[i].rgbRed = intensity;
			m_pDepthRGBX[i].rgbGreen = intensity;
			m_pDepthRGBX[i].rgbBlue = intensity;
		}

		// Draw the data with Direct2D
		m_pDrawColor->Draw(reinterpret_cast<BYTE*>(m_pDepthRGBX), pCapture->nColorFrameWidth * pCapture->nColorFrameHeight * sizeof(RGB), pCapture->v_next_Bodies);
	}
}

void LiveScanClient::ProcessColor(RGB* pBuffer, int nWidth, int nHeight) 
{
    // Make sure we've received valid data
	if (pBuffer && (nWidth == pCapture->nColorFrameWidth) && (nHeight == pCapture->nColorFrameHeight))
    {
        // Draw the data with Direct2D
		m_pDrawColor->Draw(reinterpret_cast<BYTE*>(pBuffer), pCapture->nColorFrameWidth * pCapture->nColorFrameHeight * sizeof(RGB), pCapture->v_next_Bodies);
    }
}

bool LiveScanClient::SetStatusMessage(_In_z_ WCHAR* szMessage, DWORD nShowTimeMsec, bool bForce)
{
    INT64 now = GetTickCount64();

    if (m_hWnd && (bForce || (m_nNextStatusTime <= now)))
    {
        SetDlgItemText(m_hWnd, IDC_STATUS, szMessage);
        m_nNextStatusTime = now + nShowTimeMsec;

        return true;
    }

    return false;
}

void LiveScanClient::SocketThreadFunction()
{
	while (m_bSocketThread)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		HandleSocket();
	}
}

void LiveScanClient::HandleSocket()
{
	char byteToSend;
	std::lock_guard<std::mutex> lock(m_mSocketThreadMutex);

	if (!m_bConnected)
	{
		return;
	}

	string received = m_pClientSocket->ReceiveBytes();
	for (unsigned int i = 0; i < received.length(); i++)
	{
		//capture a frame
		if (received[i] == MSG_CAPTURE_FRAME)
			m_bCaptureFrame = true;
		//calibrate
		else if (received[i] == MSG_CALIBRATE)
			m_bCalibrate = true;
		//receive settings
		//TODO: what if packet is split?
		else if (received[i] == MSG_RECEIVE_SETTINGS)
		{
			vector<float> bounds(6);
			i++;
			int nBytes = *(int*)(received.c_str() + i);
			i += sizeof(int);

			for (int j = 0; j < 6; j++)
			{
				bounds[j] = *(float*)(received.c_str() + i);
				i += sizeof(float);
			}
				
			m_bFilter = (received[i]!=0);
			i++;

			m_nFilterNeighbors = *(int*)(received.c_str() + i);
			i += sizeof(int);

			m_fFilterThreshold = *(float*)(received.c_str() + i);
			i += sizeof(float);

			m_vBounds = bounds;

			int nMarkers = *(int*)(received.c_str() + i);
			i += sizeof(int);

			calibration.markerPoses.resize(nMarkers);

			for (int j = 0; j < nMarkers; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					for (int l = 0; l < 3; l++)
					{
						calibration.markerPoses[j].R[k][l] = *(float*)(received.c_str() + i);
						i += sizeof(float);
					}
				}

				for (int k = 0; k < 3; k++)
				{
					calibration.markerPoses[j].t[k] = *(float*)(received.c_str() + i);
					i += sizeof(float);
				}

				calibration.markerPoses[j].markerId = *(int*)(received.c_str() + i);
				i += sizeof(int);
			}

			m_bStreamOnlyBodies = (received[i] != 0);
			i += 1;

			m_iCompressionLevel = *(int*)(received.c_str() + i);
			i += sizeof(int);
			if (m_iCompressionLevel > 0)
				m_bFrameCompression = true;
			else
				m_bFrameCompression = false;

			//so that we do not lose the next character in the stream
			i--;
		}
		//send stored frame
		else if (received[i] == MSG_REQUEST_STORED_FRAME)
		{
			byteToSend = MSG_STORED_FRAME;
			m_pClientSocket->SendBytes(&byteToSend, 1);

			vector<Point3s> points;
			vector<Point3s> normals;
			vector<Point2s> uvs;
			vector<unsigned short> indices;
			vector<RGB> colors;
			bool res = m_framesFileWriterReader.readFrame( points, normals, uvs, colors, indices );

			if( res == false )
			{
				int size = -1;
				m_pClientSocket->SendBytes((char*)&size, 4);
			}
			else
			{
				if( m_bFilter )
				{
					vector<Point3f> pointsF( points.size() );
					vector<Point3f> normalsF( points.size() );

					for( unsigned int i = 0; i < pointsF.size(); i++ )
					{
						pointsF[i].X = points[i].X / 1000.0f;
						pointsF[i].Y = points[i].Y / 1000.0f;
						pointsF[i].Z = points[i].Z / 1000.0f;
					}
					
					medianFilter( pointsF, normalsF, indices, m_nFilterNeighbors, m_fFilterThreshold );

					for( unsigned int i = 0; i < pointsF.size(); i++ )
					{
						points[i] = Point3s( pointsF[i], 1000.0f );
						normals[i] = Point3s( normalsF[i], 30000.0f );
					}
				}
				SendFrame( points, normals, uvs, colors, indices, m_vLastFrameBody );
			}
		}
		//send last frame
		else if (received[i] == MSG_REQUEST_LAST_FRAME)
		{
			byteToSend = MSG_LAST_FRAME;
			m_pClientSocket->SendBytes(&byteToSend, 1);

			SendFrame(m_vLastFrameVertices, m_vLastFrameNormals, m_vLastFrameUVs, m_vLastFrameRGB, m_vLastFrameIndices, m_vLastFrameBody);
		}
		//receive calibration data
		else if (received[i] == MSG_RECEIVE_CALIBRATION)
		{
			i++;
			for (int j = 0; j < 3; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					calibration.worldR[j][k] = *(float*)(received.c_str() + i);
					i += sizeof(float);
				}
			}
			for (int j = 0; j < 3; j++)
			{
				calibration.worldT[j] = *(float*)(received.c_str() + i);
				i += sizeof(float);
			}

			//so that we do not lose the next character in the stream
			i--;
		}
		else if (received[i] == MSG_CLEAR_STORED_FRAMES)
		{
			m_framesFileWriterReader.closeFileIfOpened();
		}
	}

	if (m_bConfirmCaptured)
	{
		byteToSend = MSG_CONFIRM_CAPTURED;
		m_pClientSocket->SendBytes(&byteToSend, 1);
		m_bConfirmCaptured = false;
	}

	if (m_bConfirmCalibrated)
	{
		int size = (9 + 3) * sizeof(float) + sizeof(int) + 1;
		char *buffer = new char[size];
		buffer[0] = MSG_CONFIRM_CALIBRATED;
		int i = 1;

		memcpy(buffer + i, &calibration.iUsedMarkerId, 1 * sizeof(int));
		i += 1 * sizeof(int);
		memcpy(buffer + i, calibration.worldR[0].data(), 3 * sizeof(float));
		i += 3 * sizeof(float);
		memcpy(buffer + i, calibration.worldR[1].data(), 3 * sizeof(float));
		i += 3 * sizeof(float);
		memcpy(buffer + i, calibration.worldR[2].data(), 3 * sizeof(float));
		i += 3 * sizeof(float);
		memcpy(buffer + i, calibration.worldT.data(), 3 * sizeof(float));
		i += 3 * sizeof(float);

		m_pClientSocket->SendBytes(buffer, size);
		m_bConfirmCalibrated = false;
	}
}

void LiveScanClient::SendFrame(vector<Point3s> vertices, vector<Point3s> normals, vector<Point2s> uvs, vector<RGB> RGB, vector<unsigned short> indices, vector<Body> body)
{
	int size = RGB.size() * ( 4 * sizeof( BYTE ) + 3 * sizeof( short ) + 3 * sizeof( short ) + 2 * sizeof( short ) ) + sizeof( int );

	vector<char> buffer(size);
	char *ptrVert = (char*)vertices.data();
	char *ptrNorm = (char*)normals.data();
	char *ptrUV = (char*)uvs.data();
	int pos = 0;

	int nVertices = RGB.size();
	memcpy(buffer.data() + pos, &nVertices, sizeof(nVertices));
	pos += sizeof(nVertices);

	for (unsigned int i = 0; i < RGB.size(); i++)
	{
		buffer[pos++] = RGB[i].rgbRed;
		buffer[pos++] = RGB[i].rgbGreen;
		buffer[pos++] = RGB[i].rgbBlue;
		buffer[pos++] = RGB[i].rgbState;

		memcpy( buffer.data() + pos, ptrVert, sizeof( short ) * 3 );
		ptrVert += sizeof(short) * 3;
		pos += sizeof(short) * 3;

		memcpy( buffer.data() + pos, ptrNorm, sizeof( short ) * 3 );
		ptrNorm += sizeof( short ) * 3;
		pos += sizeof( short ) * 3;

		memcpy( buffer.data() + pos, ptrUV, sizeof( short ) * 2 );
		ptrUV += sizeof( short ) * 2;
		pos += sizeof( short ) * 2;
	}

	size += sizeof( int ) + indices.size() * sizeof( unsigned short );
	buffer.resize( size );

	int nIndices = indices.size();
	memcpy( buffer.data() + pos, &nIndices, sizeof( nIndices ) );
	pos += sizeof( nIndices );

	memcpy( buffer.data() + pos, indices.data(), sizeof( unsigned short ) * indices.size() );
	pos += sizeof( unsigned short ) * indices.size();

	int nBodies = body.size();
	size += sizeof(nBodies);
	for (int i = 0; i < nBodies; i++)
	{
		size += sizeof(body[i].bTracked);
		int nJoints = body[i].vJoints.size();
		size += sizeof(nJoints);
		size += nJoints * (3 * sizeof(float) + 2 * sizeof(int));
		size += nJoints * 2 * sizeof(float);
	}
	buffer.resize(size);
	
	memcpy(buffer.data() + pos, &nBodies, sizeof(nBodies));
	pos += sizeof(nBodies);

	for (int i = 0; i < nBodies; i++)
	{
		memcpy(buffer.data() + pos, &body[i].bTracked, sizeof(body[i].bTracked));
		pos += sizeof(body[i].bTracked);

		int nJoints = body[i].vJoints.size();
		memcpy(buffer.data() + pos, &nJoints, sizeof(nJoints));
		pos += sizeof(nJoints);

		for (int j = 0; j < nJoints; j++)
		{
			//Joint
			memcpy(buffer.data() + pos, &body[i].vJoints[j].JointType, sizeof(JointType));
			pos += sizeof(JointType);
			memcpy(buffer.data() + pos, &body[i].vJoints[j].TrackingState, sizeof(TrackingState));
			pos += sizeof(TrackingState);
			//Joint position
			memcpy(buffer.data() + pos, &body[i].vJoints[j].Position.X, sizeof(float));
			pos += sizeof(float);
			memcpy(buffer.data() + pos, &body[i].vJoints[j].Position.Y, sizeof(float));
			pos += sizeof(float);
			memcpy(buffer.data() + pos, &body[i].vJoints[j].Position.Z, sizeof(float));
			pos += sizeof(float);

			//JointInColorSpace
			memcpy(buffer.data() + pos, &body[i].vJointsInColorSpace[j].X, sizeof(float));
			pos += sizeof(float);
			memcpy(buffer.data() + pos, &body[i].vJointsInColorSpace[j].Y, sizeof(float));
			pos += sizeof(float);
		}
	}

	int iCompression = static_cast<int>(m_bFrameCompression);

	if (m_bFrameCompression)
	{
		// *2, because according to zstd documentation, increasing the size of the output buffer above a 
		// bound should speed up the compression.
		int cBuffSize = ZSTD_compressBound(size) * 2;	
		vector<char> compressedBuffer(cBuffSize);
		int cSize = ZSTD_compress(compressedBuffer.data(), cBuffSize, buffer.data(), size, m_iCompressionLevel);
		size = cSize; 
		buffer = compressedBuffer;
	}
	char header[8];
	memcpy(header, (char*)&size, sizeof(size));
	memcpy(header + 4, (char*)&iCompression, sizeof(iCompression));

	m_pClientSocket->SendBytes((char*)&header, sizeof(int) * 2);
	m_pClientSocket->SendBytes(buffer.data(), size);
}

#define STATE_LINEAR	0 // --
#define STATE_FADEIN	75 // /-
#define STATE_FADEOUT   150 // -\ 
#define STATE_FADEINOUT 200 // /\  


void LiveScanClient::StoreFrame( BYTE* prevBodyIndex, Point3f *currentVertices, BYTE* currentBodyIndex, BYTE* nextBodyIndex, Point2f *currentMapping, RGB *currentColor, vector<Body> &currentBodies )
{
	std::vector<Point3f> goodVertices;
	std::vector<Point3f> goodNormals;
	std::vector<Point2f> goodUVs;
	std::vector<RGB> goodColorPoints;

	unsigned int nVertices = pCapture->nDepthFrameWidth * pCapture->nDepthFrameHeight;

	memset( vertexIndexTable, -1, nVertices * sizeof( int ) );

	for (unsigned int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++)
	{
		if( m_bStreamOnlyBodies && currentBodyIndex[vertexIndex] >= currentBodies.size() )
			continue; //discard point

		//determine the state point
		unsigned char state = STATE_LINEAR;
/*		if( m_bStreamOnlyBodies )
		{
			if( prevBodyIndex[vertexIndex] != 255 && nextBodyIndex[vertexIndex] != 255 )
				state = STATE_LINEAR;
			else if( prevBodyIndex[vertexIndex] == 255 && nextBodyIndex[vertexIndex] != 255 )
				state = STATE_FADEIN; //appears in this frame
			else if( prevBodyIndex[vertexIndex] == 255 && nextBodyIndex[vertexIndex] == 255 )
			{
				state = STATE_FADEINOUT;
				//point appears for one frame
				continue; //discard
			}
			else if( prevBodyIndex[vertexIndex] != 255 && nextBodyIndex[vertexIndex] == 255 )
				state = STATE_FADEOUT; //will disappear next frame
		}*/

		if ( currentVertices[vertexIndex].Z >= 0 && currentMapping[vertexIndex].Y >= 0 && currentMapping[vertexIndex].Y < pCapture->nColorFrameHeight)
		{
			RGB tempColor = currentColor[(int)currentMapping[vertexIndex].X + (int)currentMapping[vertexIndex].Y * pCapture->nColorFrameWidth];
			tempColor.rgbState = state;

			//if (calibration.bCalibrated)
			//{
			//	temp.X += calibration.worldT[0];
			//	temp.Y += calibration.worldT[1];
			//	temp.Z += calibration.worldT[2];
			//	temp = RotatePoint(temp, calibration.worldR);

			//	if (temp.X < m_vBounds[0] || temp.X > m_vBounds[3]
			//		|| temp.Y < m_vBounds[1] || temp.Y > m_vBounds[4]
			//		|| temp.Z < m_vBounds[2] || temp.Z > m_vBounds[5])
			//		continue;
			//}

			//calculate normal and uvs
			//right
			int x = vertexIndex % pCapture->nDepthFrameWidth;
			int y = vertexIndex / pCapture->nDepthFrameWidth;

			Vec3F normal( currentVertices[vertexIndex].X, currentVertices[vertexIndex].Y, currentVertices[vertexIndex].Z );
			/*normal = normalize( -normal );

			if( x < pCapture->nDepthFrameWidth - 1 && y > 1 )
			{
				if( currentVertices[vertexIndex + 1].Z >= 0 && currentVertices[vertexIndex - pCapture->nDepthFrameWidth].Z >= 0 )
				{
					Vec3F current( currentVertices[vertexIndex].X, currentVertices[vertexIndex].Y, currentVertices[vertexIndex].Z );
					Vec3F up( currentVertices[vertexIndex - pCapture->nDepthFrameWidth].X, currentVertices[vertexIndex - pCapture->nDepthFrameWidth].Y, currentVertices[vertexIndex - pCapture->nDepthFrameWidth].Z );
					Vec3F right( currentVertices[vertexIndex + 1].X, currentVertices[vertexIndex + 1].Y, currentVertices[vertexIndex + 1].Z );

					Vec3F v1 = up - current;
					Vec3F v2 = right - current;

					normal = cross( normalize( v1 ), normalize( v2 ) );
				}
			}*/
			goodNormals.push_back( Point3f(normal.x, normal.y, normal.z) );

			goodUVs.push_back( Point2f( currentMapping[vertexIndex].X / pCapture->nColorFrameWidth, currentMapping[vertexIndex].Y / pCapture->nColorFrameHeight ) );
			
			//writing index
			if( goodVertices.size() <= 0xffff )
				vertexIndexTable[vertexIndex] = goodVertices.size();

			goodVertices.push_back( currentVertices[vertexIndex] );
			goodColorPoints.push_back(tempColor);
		}
	}

	//now build index table
	m_vLastFrameIndices.clear();

	for( unsigned int vertexIndex = 0; vertexIndex < nVertices; vertexIndex++ )
	{
		int i1 = vertexIndexTable[vertexIndex];
		if( i1 != -1 )
		{
			int x = vertexIndex % pCapture->nDepthFrameWidth;
			int y = vertexIndex / pCapture->nDepthFrameWidth;

			if( x < pCapture->nDepthFrameWidth - 1 && y > 1 )
			{
				int i2 = vertexIndexTable[vertexIndex + 1];
				int i3 = vertexIndexTable[vertexIndex - pCapture->nDepthFrameWidth];
				int i4 = vertexIndexTable[vertexIndex + 1 - pCapture->nDepthFrameWidth];

				if( i2 != -1 && i3 != -1 )
				{
					m_vLastFrameIndices.push_back( i1 );
					m_vLastFrameIndices.push_back( i3 );
					m_vLastFrameIndices.push_back( i2 );
				}
				if( i2 != -1 && i3 != -1 && i4 != -1 )
				{
					m_vLastFrameIndices.push_back( i2 );
					m_vLastFrameIndices.push_back( i3 );
					m_vLastFrameIndices.push_back( i4 );
				}
			}
		}
	}

	vector<Body> tempBodies = currentBodies;

	for (unsigned int i = 0; i < tempBodies.size(); i++)
	{
		for (unsigned int j = 0; j < tempBodies[i].vJoints.size(); j++)
		{
			if (calibration.bCalibrated)
			{
				tempBodies[i].vJoints[j].Position.X += calibration.worldT[0];
				tempBodies[i].vJoints[j].Position.Y += calibration.worldT[1];
				tempBodies[i].vJoints[j].Position.Z += calibration.worldT[2];

				Point3f tempPoint(tempBodies[i].vJoints[j].Position.X, tempBodies[i].vJoints[j].Position.Y, tempBodies[i].vJoints[j].Position.Z);

				tempPoint = RotatePoint(tempPoint, calibration.worldR);

				tempBodies[i].vJoints[j].Position.X = tempPoint.X;
				tempBodies[i].vJoints[j].Position.Y = tempPoint.Y;
				tempBodies[i].vJoints[j].Position.Z = tempPoint.Z;
			}
		}
	}
	
	//if (m_bFilter)
		//medianFilter(goodVertices, m_nFilterNeighbors, m_fFilterThreshold );
		//filter(goodVertices, goodNormals, goodUVs, goodColorPoints, m_nFilterNeighbors, m_fFilterThreshold);

	vector<Point3s> goodVerticesShort(goodVertices.size());
	vector<Point3s> goodNormalsShort( goodVertices.size() );
	vector<Point2s> goodUVsShort( goodVertices.size() );

	for (unsigned int i = 0; i < goodVertices.size(); i++)
	{
		goodVerticesShort[i] = Point3s( goodVertices[i], 1000.0f );

		goodNormalsShort[i] = Point3s( goodNormals[i], 30000.0f );

		goodUVsShort[i] = Point2s( goodUVs[i], 30000.0f );
	}

	m_vLastFrameBody = tempBodies;
	m_vLastFrameVertices = goodVerticesShort;
	m_vLastFrameNormals = goodNormalsShort;
	m_vLastFrameUVs = goodUVsShort;
	m_vLastFrameRGB = goodColorPoints;
}

void LiveScanClient::ShowFPS()
{
	if (m_hWnd)
	{
		double fps = 0.0;

		LARGE_INTEGER qpcNow = { 0 };
		if (m_fFreq)
		{
			if (QueryPerformanceCounter(&qpcNow))
			{
				if (m_nLastCounter)
				{
					m_nFramesSinceUpdate++;
					fps = m_fFreq * m_nFramesSinceUpdate / double(qpcNow.QuadPart - m_nLastCounter);
				}
			}
		}

		WCHAR szStatusMessage[64];
		StringCchPrintf(szStatusMessage, _countof(szStatusMessage), L" FPS = %0.2f", fps);

		if (SetStatusMessage(szStatusMessage, 1000, false))
		{
			m_nLastCounter = qpcNow.QuadPart;
			m_nFramesSinceUpdate = 0;
		}
	}
}

void LiveScanClient::ReadIPFromFile()
{
	ifstream file;
	file.open("lastIP.txt");
	if (file.is_open())
	{
		char lastUsedIPAddress[20];
		file.getline(lastUsedIPAddress, 20);
		file.close();
		SetDlgItemTextA(m_hWnd, IDC_IP, lastUsedIPAddress);
	}
}

void LiveScanClient::WriteIPToFile()
{
	ofstream file;
	file.open("lastIP.txt");
	char lastUsedIPAddress[20];
	GetDlgItemTextA(m_hWnd, IDC_IP, lastUsedIPAddress, 20);
	file << lastUsedIPAddress;
	file.close();
}
