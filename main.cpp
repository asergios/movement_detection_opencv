/*
 * Project 2 OpenCV
 *
 * Antonio Silva 76678
 * Rafael Oliveira 76525
 * 
 */

#include <iostream>

#include "opencv2/core/core.hpp"

#include "opencv2/imgproc/imgproc.hpp"

#include "opencv2/highgui/highgui.hpp"

#include <unistd.h>

#include <ctime>

using namespace cv;

using namespace std;

// Default Settings
int cameraIdx = 0; 			// index of camera to use
int width = 800;			// width of camera resolution
int height = 600;			// height of camera resolution
int sensitivity = 15;		// movement detection sensitivity (inverted, lower makes it more sensible)
int debugging = 0;			// debugging, if set to 1 shows image processing

string message = "You can change setting by running as: \n \n \
	./run <cameraIdx> <width> <height> <sensitivity> <debugging> \n \n \
	cameraIdx - index of camera to use (default: 0) \n \
	width, height - width and height of camera resolution (default: 800x600)\n \
	sensitivity - inverted sensitivity, lower makes change detector more sensible (default : 15)\n \
	debugging - set to 1 to see image processing (default: 0)\n \n \
	Antonio Silva && Rafael Oliveira \n \n";

// TRANSFORM IMAGE (GREY SCALE + GAUSSIANBLUR)
void transformImage( cv::Mat &originalImage, cv::Mat &result )
{
	// change to grayScale
	cv::cvtColor( originalImage, result, cv::COLOR_BGR2GRAY, 1 );
	// apply gaussianBlur
	cv::GaussianBlur(result, result, cv::Size(31,31), 5, 5);
}

// GETTING FRAME FROM WEBCAM
void getWebCamFrame( cv::Mat &cameraFrame, cv::Mat &transformedCameraFrame, cv::VideoCapture &webCam)
{
	// reading frame from webCam
	webCam.read(cameraFrame);

	// check for error
    if( cameraFrame.empty() )
	{

		std::cout << "Something went wrong" << std::endl;
		exit(0);

	}
	
	transformImage(cameraFrame, transformedCameraFrame);
}

// GET DIFERENCE BETWEEN 2 IMAGES, RESULT ON SECOND ARGUMENT
void getDiference(cv::Mat &original, cv::Mat &toCompare)
{
	// get absolute diference
	cv::absdiff(original, toCompare, toCompare);
	// apply threshold, everything above sensitivity will be white
	cv::threshold( toCompare, toCompare, sensitivity, 255, cv::THRESH_BINARY );
}


// MARK AREAS WHERE MOVEMENT WAS FOUND
void markAreas( cv::Mat &imageDiference , cv::Mat &originalImage)
{
	vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    int biggest = 0;
    int biggestidx = 0;

    // struturing element
    cv::Mat elementoEstruturante = cv::getStructuringElement( cv::MORPH_ELLIPSE, cv::Size(30, 30) , cv::Point(-1, -1) );
    // dilate in order to close gaps
    cv::dilate( imageDiference, imageDiference, elementoEstruturante, cv::Point(-1, -1), 2 );

    // show image if debbuging == 1
    if (debugging == 1) cv::imshow( "Debug", imageDiference );

    // find areas
	cv::findContours( imageDiference, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

	// for each area found
	for(int i = 0; i < contours.size() ; i++){
		// ignore small ones
		if(contourArea(contours[i]) > 1500)
		{
			// find biggest one
			if(contourArea(contours[i]) > biggest)
			{
				biggest = contourArea(contours[i]);
				biggestidx = i;
			}

			// mark red rectangle
			Rect rect = boundingRect(contours[i]);
			rectangle(originalImage, rect , CV_RGB(255,0,0), 2);
		}
	}

	// the biggest are will be shown on another window
	if (biggest != 0)
	{
		Rect rect = boundingRect(contours[biggestidx]);
		cv::imshow( "Movement Detected", originalImage(rect) );
		cv::putText( originalImage , "Movement Detected", cv::Point(20,20), FONT_HERSHEY_TRIPLEX, 0.5, CV_RGB(255,0,0));
	}

	// write date
	time_t now = time(0);
    char* dt = ctime(&now);
    dt[ strlen(dt) - 1 ] = '\0';
    cv::putText( originalImage , dt, cv::Point(20, originalImage.rows - 20), FONT_HERSHEY_TRIPLEX, 0.5, CV_RGB(0,255,0));
}


// SHOW IMAGE
void showImage( cv::Mat &imageToShow )
{
	cv::imshow( "Camera", imageToShow );
}


// MAIN
int main( int argc, char** argv  )
{

	std::cout << message << std::endl;

	if ( argc > 1  && argc < 7)
	{
		switch(argc)
		{
			case 6: debugging = std::stoi(argv[5]);
			case 5: sensitivity = std::stoi(argv[4]);
			case 4: height = std::stoi(argv[3]);
			case 3: width = std::stoi(argv[2]);
			case 2: cameraIdx = std::stoi(argv[1]);
		}
	}

	cv::Mat originalImage;
	cv::Mat transformedImage;
	cv::Mat ImageToCompare;
	int key;

	// Get camera 
    cv::VideoCapture webCam( cameraIdx );

    if(!webCam.isOpened()) return -1;
 
 	// Set camera to 600x800 pixels
    webCam.set(CV_CAP_PROP_FRAME_WIDTH, width);
	webCam.set(CV_CAP_PROP_FRAME_HEIGHT, height);
	webCam.set(CV_CAP_PROP_CONTRAST, 0.5);
	webCam.set(CV_CAP_PROP_SATURATION, 0.5);

	// Get Camera Setting
    double Brightness = webCam.get(CV_CAP_PROP_BRIGHTNESS);
 	double Contrast   = webCam.get(CV_CAP_PROP_CONTRAST );
 	double Saturation = webCam.get(CV_CAP_PROP_SATURATION);
 	double Gain       = webCam.get(CV_CAP_PROP_GAIN);

 	// Print Camera Settings
 	std::cout << "Your Camera Settings: " << std::endl;
    std::cout << "Brightness: " + std::to_string(Brightness) << std::endl;
    std::cout << "Contrast: " 	+ std::to_string(Contrast) << std::endl;
    std::cout << "Saturation: " + std::to_string(Saturation) << std::endl;
    std::cout << "Gain: " 		+ std::to_string(Gain) << std::endl;
    
    std::cout << "\n\nPress any key to start" << std::endl;
    std::cin.ignore();

    // Create windows
    cv::namedWindow( "Camera", cv::WINDOW_AUTOSIZE );
 	if (debugging == 1) cv::namedWindow( "Debug", cv::WINDOW_AUTOSIZE );
 	cv::namedWindow( "Movement Detected", cv::WINDOW_NORMAL );
 	cv::resizeWindow( "Movement Detected", width, height);

	while(1)
	{
		// Get Image and Image to compare
		getWebCamFrame(originalImage, transformedImage, webCam);
		key = cv::waitKey( 30 );
		getWebCamFrame(ImageToCompare, ImageToCompare, webCam);

		// Get Diference Between both
		getDiference(transformedImage, ImageToCompare);

		// Get an original Image a mark it with movement found
		markAreas(ImageToCompare, originalImage);
		showImage(originalImage);

		// Press ESC to end program
		if( key == 27) break;
		// Press s or S to save screenshot
		if( key == 83 || key == 115 ) cv::imwrite("screenshot.jpg", originalImage);
	}

	// Destroy the windows
	cv::destroyAllWindows();

	return 0;
}
