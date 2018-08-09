/*
ShapeCV
Copyright (C) 2018 Simone Dassi

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "stdafx.h"
#include <iostream>
#include <stack>
#include <opencv2\core.hpp>
#include <opencv2\imgproc.hpp>
#include <opencv2\imgcodecs.hpp>
#include <opencv2\highgui.hpp>

using namespace std;

vector<vector<cv::Point> > contoursVect, approxVect;
vector<cv::Vec4i> hierarchy;

cv::RNG rng(12345);

cv::Mat filled;

string type2str(int type) {
	string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + (type >> CV_CN_SHIFT);

	switch (depth) {
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += (chans + '0');

	return r;
}

cv::Vec3b hsv_to_rgb(cv::Vec3b src)
{
	cv::Mat srcMat(1, 1, CV_8UC3);
	*srcMat.ptr< cv::Vec3b >(0) = src;

	cv::Mat resMat;
	cv::cvtColor(srcMat, resMat, CV_HSV2BGR);

	return *resMat.ptr< cv::Vec3b >(0);
}

bool isLineSimple(vector<cv::Point> line) {		//Edges of objects cannot self-intersect
	for (int i = 0; i < line.size(); i++)
		for (int j = 0; j < line.size(); j++)
			if (i != j && i!=j+1 && i!=j-1 && cv::norm(line[i] - line[j]) < 2)
				return false;
	return true;
}

bool isCloneRec(vector<cv::Point> l1, vector<cv::Point> l2) {
	if (!l1.size() && !l2.size())
		return true;

	for (int i = 0; i < l1.size(); i++)
		if (cv::norm(l1[0] - l2[i]) < 8) {
			l1.erase(l1.begin());
			l2.erase(l2.begin()+i);
			return isCloneRec(l1, l2);
		}

	return false;
}

bool isClone(vector<cv::Point> l1, vector<cv::Point> l2) {

	if (l1.size() != l2.size())
		return false;


	//only works if points are in the same order

	//for (int i = 0; i < l1.size(); i++) {
	//	if (cv::norm(l1[i] - l2[i]) > 4)
	//		return false;
	//}

	//return true;

	vector<cv::Point> _l1 = l1, _l2 = l2;

	return isCloneRec(_l1, _l2);	//modifies the copy
}

bool cloneExists(vector<cv::Point> line, vector<vector<cv::Point> > list) {
	for (int i = 0; i < list.size(); i++)
		if (isClone(line, list[i]))
			return true;

	return false;
}

void floodFillRec(int i, int j, cv::Vec3b color, int tot) {
	//cout << i << " " << j << " " << tot << endl;

	if (i<0 || j < 0 || i > filled.rows - 1 || j > filled.cols - 1)
		return;
	if (filled.at<cv::Vec3b>(i, j) != cv::Vec3b(0, 0, 0))
		return;

	filled.at<cv::Vec3b>(i, j) = color;

	cv::imshow("Filled", filled);
	cv::waitKey(1);

	floodFillRec(i - 1, j, color, tot+1);
	floodFillRec(i, j - 1, color, tot+1);
	floodFillRec(i, j + 1, color, tot+1);
	floodFillRec(i + 1, j, color, tot+1);
}

void floodFillIter(cv::Mat image, cv::Point p0, cv::Vec3b color) {
	int cont = 0;
	stack<cv::Point> s;
	s.push(p0);

	do {
		cv::Point p(s.top().x, s.top().y);	//deep copy of the top element, so i can access x and y after popping the stack
		//cout << p.x << " " << p.y << " " << endl;

		if (p.x < 0 || p.y < 0 || p.x > image.rows - 1 || p.y > image.cols - 1) {		//Point.x is the row (see floodFill())
			s.pop();
			continue;
		}
		if (image.at<cv::Vec3b>(p.x, p.y) != cv::Vec3b(0, 0, 0)) {	//deja-vu, I've just been in this place before..
			s.pop();
			continue;
		}

		image.at<cv::Vec3b>(p.x, p.y) = color;
		s.pop();	//this pixel has been computed, remove it from the stack

		if (cont == 10000) {
			cont = 0;
			cv::imshow("Filled", image);	//update live image
			cv::waitKey(1);
		}
		cont++;

		//push the neighbors to propagate the color
		s.push(cv::Point(p.x - 1, p.y));
		s.push(cv::Point(p.x, p.y - 1));
		s.push(cv::Point(p.x, p.y + 1));
		s.push(cv::Point(p.x + 1, p.y));

	} while (!s.empty());
}

void floodFill(cv::Mat image) {
	cv::namedWindow("Filled");

	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			if (image.at<cv::Vec3b>(i, j) == cv::Vec3b(0, 0, 0)) {
				//cv::Vec3b color = cv::Vec3b(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

				cv::Vec3b color = hsv_to_rgb(cv::Vec3b(rng.uniform(0, 180), 255, 255));

				floodFillIter(image, cv::Point(i, j), color);
				cout << "zone complete" << endl;
			}
		}
	}
}

void detectGrayscaleEdges(cv::Mat src, cv::Mat* edges) {
	cv::Canny(src, *edges, 50, 150);	//60,240

	cv::namedWindow("Edges");
	cv::imshow("Edges", *edges);
	cv::waitKey(0);
}

void detectRGBEdges(cv::Mat src, cv::Mat* edges) {	//Detects edges inside the three channels instad of working on the grayscale image (which contains 1/3 of the original data) 
	vector<cv::Mat> channels;

	cv::split(src, channels);

	static const int lThresh = 60, hThresh = 240;
	cv::Canny(channels[0], channels[0], lThresh, hThresh);	//Computes gradient module and binarizes it with thresholds
	cv::Canny(channels[1], channels[1], lThresh, hThresh);
	cv::Canny(channels[2], channels[2], lThresh, hThresh);

	*edges = channels[0] + channels[1] + channels[2];

	cv::namedWindow("Edges");
	cv::imshow("Edges", *edges);
	cv::namedWindow("B");
	cv::imshow("B", channels[0]);
	cv::namedWindow("G");
	cv::imshow("G", channels[1]);
	cv::namedWindow("R");
	cv::imshow("R", channels[2]);
	cv::waitKey(0);
}

int main(){

	cv::Mat src = cv::imread("src.jpg", CV_LOAD_IMAGE_COLOR);
	if (!src.data) {
		cout << "Source image not found";
		return 0;
	}
	cout << "Image type: " << type2str(src.type()) << endl;
 
	//DAMPEN NOISE
	cv::GaussianBlur(src, src, cv::Size(7,7), 0, 0);

	//DETECT EDGES
	cv::Mat edges;
	//cv::cvtColor(src, src, CV_BGR2GRAY);	//GRAYSCALE DETECTION
	//detectGrayscaleEdges(src, &edges);
	detectRGBEdges(src, &edges);
	cv::cvtColor(src, src, CV_BGR2GRAY);
	
	//CLOSE THE EDGES
	cv::Mat element3Cross = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
	cv::Mat element3Rect = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::Mat element2Cross = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(2, 2));
	cv::morphologyEx(edges, edges, cv::MORPH_CLOSE, element3Cross);
	cv::morphologyEx(edges, edges, cv::MORPH_CLOSE, element3Rect);

	//SORT EDGE LINES INTO VECTORS OF PIXELS
	cv::findContours(edges, contoursVect, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	cv::Mat contours = cv::Mat::zeros(edges.rows, edges.cols, CV_8UC1);
	cv::Mat filled(contours.rows, contours.cols, CV_8UC3, cv::Scalar(0, 0, 0));

	//DRAW RELEVANT CONTOURS
	for (int i = 0; i < contoursVect.size(); i++) {
		vector<cv::Point> hull;
		cv::convexHull(contoursVect[i], hull);
		if (cv::contourArea(hull) > 0.001 * contours.rows * contours.cols) {	//objects must have a meaningful area > 0.1% total surface

			cv::drawContours(contours, contoursVect, i, cv::Scalar(255));
			cv::drawContours(filled, contoursVect, i, cv::Scalar(255, 255, 255));

			//APPROX EDGES WITH POLY LINES
			vector<cv::Point> approx;	//array of vertices
			cv::approxPolyDP(contoursVect[i], approx, 0.01 * cv::arcLength(contoursVect[i], false), true);
			cout << approx.size() << endl;

			//FILTER BAD COUNTOURS AND DUPLICATES, DRAW POLYGONS AROUND SOURCE OBJECTS
			if (isLineSimple(approx) && !cloneExists(approx, approxVect)) {
				approxVect.push_back(approx);
			}
		}
	}
	cv::drawContours(src, approxVect, -1, cv::Scalar(255, 0, 0));

	//FLOOD FILL
	//cv::dilate(filled, filled, element2Cross);	//more robust against small 'holes'
	filled = filled(cv::Rect(2, 2, contours.cols - 3, contours.rows - 3));	//pretty
	
	floodFill(filled);

	//BLEND ORIGINAL AND FILLED
	cv::Mat blend;
	cv::cvtColor(src, blend, CV_GRAY2BGR);
	blend = blend(cv::Rect(2, 2, contours.cols - 3, contours.rows - 3));	//same as filled

	cv::addWeighted(blend, 0.5, filled, 0.5, 0, blend);

	//DISPLAY EVERYTHING
	cv::namedWindow("Source");
	cv::namedWindow("Edges");
	cv::namedWindow("Contours");
	cv::namedWindow("Blend");
	cv::imshow("Source", src);
	cv::imshow("Edges", edges);
	cv::imshow("Contours", contours);
	cv::imshow("Filled", filled);
	cv::imshow("Blend", blend);
	cv::waitKey(0);

    return 0;
}

