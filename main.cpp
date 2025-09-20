#include<iostream>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

using namespace cv;
using namespace std;

int main () {

    string imagePath = "../images/objects.png";
    Mat image = imread(imagePath);

    if (image.empty()) {
        cout << "Error: Could not load image from " << imagePath << endl;
        return -1;
    }

    while(true){
        imshow("Cutout tool", image);

        char key = waitKey(20);

        if(key == 'q'){
            cout << "Goodbye!";
            break;
        }

    }

    return 0;
}