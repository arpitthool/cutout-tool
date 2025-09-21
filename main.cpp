#include<iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

using namespace cv;
using namespace std;

bool selectRectangleMode = false, firstClickDone = false;
Point firstClick, secondClick;      // Variables to store the coordinates of the two clicks
Mat ogImage, image;  
Rect selectedRect;                  // Rectangle selected by user

// Mouse callback function for rectangle selection - this function is called whenever the mouse is clicked
// Where x and y are the coordinates of the mouse click and event is the type of event that occurred
void mouseCallback(int event, int x, int y, int flags, void* userdata) {

    // Check if the left mouse button was pressed down AND we're in rectangle selection mode
    if (event == EVENT_LBUTTONDOWN && selectRectangleMode) {

        if (!firstClickDone) {
            // store first click coordinates
            firstClick = Point(x, y);

            firstClickDone = true;
            
            cout << "First click at: (" << x << ", " << y << "). Click again for bottom-right corner." << endl;
        } else {
            // store second click coordinates
            secondClick = Point(x, y);

            firstClickDone = false;
            
            // Exit rectangle selection mode
            selectRectangleMode = false;
            
            // Make sure the first click is actually the top-left corner and second click is bottom-right
            // This handles cases where user clicks bottom-right first, then top-left
            Point topLeft = Point(min(firstClick.x, secondClick.x), min(firstClick.y, secondClick.y));
            Point bottomRight = Point(max(firstClick.x, secondClick.x), max(firstClick.y, secondClick.y));

            selectedRect = Rect(topLeft, bottomRight);
            
            // Draw a rectangle on the image with 2-pixel thickness
            rectangle(image, topLeft, bottomRight, Scalar(0, 255, 0), 2);
            
            cout << "Rectangle selected: Top-left (" << topLeft.x << ", " << topLeft.y 
                 << "), Bottom-right (" << bottomRight.x << ", " << bottomRight.y << ")" << endl;            
        }
    }
}

// createMask() function takes in an image and a rectangle area, then finds the foreground objects in this specified
// by creating a mask. It refines this mask using Guassian Blur and finally returns the mask.
Mat createMask (Mat image, Rect selectedRect) {
    Mat bgModel, fgModel; 

    Mat mask = Mat(image.size(), CV_8UC1, GC_BGD);
    
    // Run GrabCut on the selected rectangle
    grabCut(image, mask, selectedRect, bgModel, fgModel, 5, GC_INIT_WITH_RECT);
    
    // refine the mask
    /*
    grabCut fills mask with labels 0..3 (GC_BGD=0, GC_FGD=1, GC_PR_BGD=2, GC_PR_FGD=3).

    When you do image.copyTo(result, mask), any non-zero value is treated as “copy,” so the entire rectangle (values 1–3) gets copied → looks like a plain crop.

    When you imshow(mask), it looks black because pixel values are tiny (0–3), not 0/255.
    
    The solution is to use a binary foreground mask that keeps only FG or PR_FG, and scale it to 0/255 for display.

    mask == GC_FGD creates a temporary binary mask where pixels that are definite foreground (1) become 255 (and everything else is 0), type CV_8UC1.

    mask == GC_PR_FGD does the same for probable foreground (3).

    The | (bitwise OR) combines them.
    So the result is a single-channel image of type CV_8UC1 where foreground = 255, background = 0.
    */

    // Keep only definite FG (1) or probable FG (3)
    mask = (mask == GC_FGD) | (mask == GC_PR_FGD);

    // apply guassian filter to smoothen the edges
    /*
    pure black/white pixels near borders become shades of gray (e.g. 50, 128, 200).

    This is often used to feather or antialias the edge so the cutout blends more smoothly with new backgrounds.
    */
    GaussianBlur(mask, mask, cv::Size(3,3), 0);

    // After blurring, you typically re-threshold if you need a hard mask again:
    threshold(mask, mask, 128, 255, cv::THRESH_BINARY);
    
    return mask;
}

int main () {

    string imagePath = "../images/objects.png";
    ogImage = imread(imagePath);
    image = ogImage.clone();
    
    if (image.empty()) {
        cout << "Error: Could not load image from " << imagePath << endl;
        return -1;
    }

    // Create a window to display the image
    namedWindow("Cutout tool", WINDOW_AUTOSIZE);

    // VIP : Tell the program to call our mouseCallback function whenever the mouse is clicked in this window
    setMouseCallback("Cutout tool", mouseCallback, NULL);

    while(true){
        imshow("Cutout tool", image);

        char key = waitKey(20);

        if(key == 'q'){
            cout << "Goodbye!\n";
            break;
        } else if( key == 'r') {
            // Check for rectangle selection mode
            if (!selectRectangleMode) {

                // reset the image
                image = ogImage.clone();

                // Enter rectangle selection mode
                selectRectangleMode = true;

                // Reset the first click flag
                firstClickDone = false;

                cout << "Rectangle selection mode activated. Click for top-left corner." << endl;
            } else {
                cout << "Already in rectangle selection mode. Complete the current selection first." << endl;
            }
        } else if( key == 'g') {
            // Check if we have a valid rectangle and mask from GrabCut
            if(selectedRect.width > 0 && selectedRect.height > 0) {

                Mat mask = createMask(image, selectedRect);
               
                // Create a result image with only the foreground
                Mat result;
                image.copyTo(result, mask);
                
                // Show the cutout result in a new window
                imshow("Cutout Mask", mask);
                imshow("GrabCut Result", result);

                cout << "GrabCut result displayed! Close the result window to continue." << endl;
            } else {
                cout << "No rectangle selected or GrabCut not run yet. Press 'r' to select a rectangle first." << endl;
            }
        }

    }

    return 0;
}