#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Defines.h"
#include "EdgeSegment.h"
#include "Point.h"
#include "PreProcessing.hpp"


using namespace std;

//Mat src; Mat src_gray;
int thresh = 100;
int max_thresh = 255;
//RNG rng(12345);

///* Function header
//vo*/id thresh_callback(int, void*);

/** @function main */
int main(int argc, char** argv) {
	/*/// Load source image and convert it to gray
	src = imread("D:\\Users\\Boschki\\Documents\\Visual Studio 2013\\Projects\\Test\\strassenschilder.jpg");

	 Convert image to gray and blur it
	cvtColor(src, src_gray, CV_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3));

	 Create Window
	char* source_window = "Source";
	namedWindow(source_window, CV_WINDOW_AUTOSIZE);
	imshow(source_window, src);

	createTrackbar(" Canny thresh:", "Source", &thresh, max_thresh, thresh_callback);
	thresh_callback(0, 0);

	waitKey(0);*/

	 //Load source image and convert it to gray
	
	//cv::Mat src = cv::imread("..\\strassenschilder2.png", CV_LOAD_IMAGE_GRAYSCALE);
	//cv::Mat src = cv::imread("..\\Test.png", CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat src = cv::imread("..\\mehrEllipsen.png", CV_LOAD_IMAGE_GRAYSCALE);
	 //cv::Mat src = cv::imread("..\\bloederFall1.png", CV_LOAD_IMAGE_GRAYSCALE);
	//cv::Mat src = cv::imread("..\\TestBild2.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	//cv::Mat src = cv::imread("..\\strassenschilder.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	//cv::Mat test = cv::Mat::zeros(20, 20, CV_8UC1);
	cv::RNG rng(12353);
	clock_t start, end,time;
		
	/*test.at<uchar>(0, 0) = 255;
	test.at<uchar>(5, 6) = 255;
	test.at<uchar>(5, 7) = 255;*/
	//src = test;
	cv::Mat edgeImage = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
	//cv::Mat edgeImage2(src);
	cv::Mat segImage = cv::Mat::zeros(src.rows, src.cols, CV_8UC3);
	cv::Mat lineSegmentedEdges = cv::Mat::zeros(src.rows, src.cols, CV_8UC3);
	cv::Mat endPointImage = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
	cv::Mat curveSegImage = cv::Mat::zeros(src.rows, src.cols, CV_8UC3);

	list<EdgeSegment*> edgeSegments;
	list<Point*> endPoints;
	list<EdgeSegment*> curveSegments;
	set<EllipticalArc*> ellipticalArcs;
	int edgeCnt=0;


	// Create Window
	printf("Hallo Startrows: %d, cols: %d\n",src.rows,src.cols);
	char* source_window = "Source";
	cv::namedWindow(source_window, CV_WINDOW_AUTOSIZE);
	cv::imshow(source_window, src);

	start = clock();
	//detect edges
	edgeDetection(src, edgeImage, 100, 150, 3);
	end = clock();
	time = (end - start);
	cout << "Laufzeit edgeDetection: " << time << " ms" << endl;

#ifdef DEBUG_SHOW_EDGES
	char* edge_window = "Edges";
	cv::namedWindow(edge_window, CV_WINDOW_AUTOSIZE);
	cv::imshow(edge_window, edgeImage);
	cv::imwrite("..\\edgeImage.jpg", edgeImage);
	
#endif
	cv::Mat edgeImage2(edgeImage);


	//find ends
	start = clock();
	int nEnds  = findEnds(&endPoints, &edgeImage2,&edgeCnt);
	end = clock();
	time = (end - start);
	cout << "Laufzeit findEnds: " << time << " ms" << endl;

#ifdef DEBUG_SHOW_EDGES
	char* edge2_window = "Edges2";
	cv::namedWindow(edge2_window, CV_WINDOW_AUTOSIZE);
	cv::imshow(edge2_window, edgeImage2);
	cv::imwrite("..\\edgeImage_NachFindEnds.jpg", edgeImage2);
#endif


	//link edges
	int nSegs = 0;
	start = clock();
	nSegs = edgeLinking(&edgeImage2, &endPoints, &edgeSegments,&edgeCnt);
	end = clock();
	time = (end - start);
	cout << "Laufzeit edgeLinking: " << time << " ms" << endl;
	printf("Ends: %d\nSegments: %d\n", nEnds,nSegs);

#ifdef DEBUG_SHOW_EDGESEGS
	//show segments
	char* segment_window = "Segments";
	cv::namedWindow(segment_window, CV_WINDOW_AUTOSIZE);

	for (list<EdgeSegment*>::iterator it = edgeSegments.begin(); it != edgeSegments.end(); it++){
		cv::Vec3b color(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		if ((*it)->getLength() > MIN_LENGTH) {
			(*it)->drawToImage(&segImage, color);
			/*cv::imshow(segment_window, segImage);
			cout << "ID: " << (*it)->ID<<endl;
			cv::waitKey();*/
		}
	}
	cv::imshow(segment_window, segImage);
	cv::imwrite("..\\Segments.jpg", segImage);
#endif 

	//line segmentation for each Edge segment
	list<EdgeSegment*>::iterator it;
	start = clock();
	for (it=edgeSegments.begin(); it!=edgeSegments.end();)	{
		if ((*it)->getLength() > MIN_LENGTH) {
			(*it)->lineSegmentation(D_TOL);
			it++;
		}else{
			it=edgeSegments.erase(it);
		}
	}
	end = clock();
	time = (end - start);
	cout << "Laufzeit lineSegmentation: " << time << " ms" << endl;

#ifdef DEBUG_SHOW_LINESEGMENTEDEDGES
	std::cout << "G�ltige Segmente: " << edgeSegments.size() << std::endl;
	//draw segments
	for (list<EdgeSegment*>::iterator it = edgeSegments.begin(); it != edgeSegments.end(); it++){
		cv::Vec3b color(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		(*it)->drawToImage(&lineSegmentedEdges, color);
	}
	char* lineSegmentedEdges_window = "LineSegmentedEdges";
	cv::namedWindow(lineSegmentedEdges_window, CV_WINDOW_AUTOSIZE);
	cv::imshow(lineSegmentedEdges_window, lineSegmentedEdges);
	cv::imwrite("..\\lineSegmentedEdges.jpg", lineSegmentedEdges);
#endif

	//curve segmentation
	start = clock();
	int t=curveSegmentation(&edgeSegments,&curveSegments);
	end = clock();
	time = (end - start);
	cout << "Laufzeit curveSegmentation: " << time << " ms" << endl;

	for (list<EdgeSegment*>::iterator it = curveSegments.begin(); it != curveSegments.end();)	{
		if ((*it)->getLength() < NP /*|| !(*it)->evaluateCurvature()*/) {
			it = curveSegments.erase(it);
		}
		else{
			it++;
		}
	}

#ifdef DEBUB_CURVE_SEG
	std::cout << "Curve Segments: " << t <<"Verwendete Segmente: "<<curveSegments.size()<< std::endl;
	for (list<EdgeSegment*>::iterator it = curveSegments.begin(); it != curveSegments.end(); it++){
		cv::Vec3b color(rng.uniform(10, 255), rng.uniform(10, 255), rng.uniform(10, 255));
		(*it)->drawToImage(&curveSegImage, color);
		char* curveSegments_window = "Curve Segments";
		cv::namedWindow(curveSegments_window, CV_WINDOW_AUTOSIZE);
		cv::imshow(curveSegments_window, curveSegImage);
		cout << (*it)->ID << endl;
		cv::waitKey(0);
	}
	char* curveSegments_window = "Curve Segments";
	cv::namedWindow(curveSegments_window, CV_WINDOW_AUTOSIZE);
	cv::imshow(curveSegments_window, curveSegImage);
	//cv::waitKey(0);
	cv::imwrite("..\\curveSegments.jpg", curveSegImage);
#endif

	//curve grouping
	start = clock();
	int nArcs=curveGrouping(&curveSegments, &ellipticalArcs);
	end = clock();
	time = (end - start);
	cout << "Laufzeit curveGrouping: " << time << " ms" << endl;

#ifdef DEBUB_CURVE_GRP
	cout << "Elliptical Arcs: " << nArcs << endl;
	cv::Mat arcImage = cv::Mat::zeros(src.rows, src.cols, CV_8UC3);
	for (set<EllipticalArc*>::iterator it = ellipticalArcs.begin(); it != ellipticalArcs.end(); it++){
		cv::Vec3b color(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		(*it)->drawToImage(&arcImage, color);
		char* arcs_window = "Elliptical Arcs";
		cv::namedWindow(arcs_window, CV_WINDOW_AUTOSIZE);
		cv::imshow(arcs_window, arcImage);
		//cv::imwrite("..\\ellipticalArcs.jpg", arcImage);
		cv::waitKey(0);
	}
	char* arcs_window = "Elliptical Arcs";
	cv::namedWindow(arcs_window, CV_WINDOW_AUTOSIZE);
	cv::imshow(arcs_window, arcImage);
	cv::imwrite("..\\ellipticalArcs.jpg", arcImage);
#endif
	cv::waitKey(0);
	return(0);
}
