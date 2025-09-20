#include<iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

using namespace cv;
using namespace std;

bool selectRectangleMode = false, firstClickDone = false;
Point firstClick, secondClick;      // Variables to store the coordinates of the two clicks
Mat image;  

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
            
            // Draw a rectangle on the image with 2-pixel thickness
            rectangle(image, topLeft, bottomRight, Scalar(0, 255, 0), 2);
            
            cout << "Rectangle selected: Top-left (" << topLeft.x << ", " << topLeft.y 
                 << "), Bottom-right (" << bottomRight.x << ", " << bottomRight.y << ")" << endl;
        }
    }
}

int main () {

    string imagePath = "../images/objects.png";
    image = imread(imagePath);

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

                // Enter rectangle selection mode
                selectRectangleMode = true;

                // Reset the first click flag
                firstClickDone = false;

                cout << "Rectangle selection mode activated. Click for top-left corner." << endl;
            } else {
                cout << "Already in rectangle selection mode. Complete the current selection first." << endl;
            }
        }

    }

    return 0;
}