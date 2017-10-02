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
#include "ICapture.h"

ICapture::ICapture()
{
	bInitialized = false;

	nColorFrameHeight = 0;
	nColorFrameWidth = 0;
	nDepthFrameHeight = 0;
	nDepthFrameWidth = 0;
}

ICapture::~ICapture()
{
	if( p_next_Depth != NULL )
	{
		delete[] p_next_Depth;
		p_next_Depth = NULL;
	}

	if (p_current_ColorRGBX != NULL)
	{
		delete[] p_current_ColorRGBX;
		p_current_ColorRGBX = NULL;
	}

	if( p_next_ColorRGBX != NULL )
	{
		delete[] p_next_ColorRGBX;
		p_next_ColorRGBX = NULL;
	}

	if (p_current_BodyIndex != NULL)
	{
		delete[] p_current_BodyIndex;
		p_current_BodyIndex = NULL;
	}
}