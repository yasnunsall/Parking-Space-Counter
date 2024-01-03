#include <opencv2/opencv.hpp>
#include <fstream>
#include <vector>

int width = 100;
int height = 42;

std::string projectPath = "C:\\Users\\ben\\Desktop\\cpp\\CV Projects\\parking_space_counter\\";
std::string dataPath = projectPath + "data\\";
std::string outputPath = projectPath + "output\\";

std::vector<cv::Point> getPosList(std::string path) {
    std::fstream file(path);

    if (!file.is_open()) {
        std::cerr << "Unable to open the file:  " << path << std::endl;
        return std::vector<cv::Point>();
    }

    std::vector<cv::Point> posList;
    int x, y;

    while (file >> x >> y) {
        posList.push_back(cv::Point(x, y));
    }

    return posList;
}

void checkParkScape(cv::Mat& orgImg, cv::Mat processedImg, std::vector<cv::Point> posList) {
    int spaceCounter = 0;

    for (cv::Point pos : posList) {
        int x = pos.x, y = pos.y;

        cv::Mat imgCrop = processedImg(cv::Rect(x, y, width, height));
        int count = cv::countNonZero(imgCrop);

        int thresh = static_cast<int>(width * height * 0.3);

        cv::Scalar color;
        if (count < thresh) {
            color = cv::Scalar(0, 255, 0);
            spaceCounter += 1;
        } else {
            color = cv::Scalar(0, 0, 255);
        }

        cv::rectangle(orgImg, pos, cv::Point(pos.x + width, pos.y + height), color, 1);
        cv::putText(orgImg, std::to_string(count), cv::Point(x, y + height - 2),
                    cv::FONT_HERSHEY_PLAIN, 1, color, 1);
    }

    cv::putText(orgImg, "Free: " + std::to_string(spaceCounter) + "/" + std::to_string(posList.size()), cv::Point(15, 24),
                cv::FONT_HERSHEY_COMPLEX_SMALL, 1.5, cv::Scalar(0, 0, 0), 2);
}

cv::Mat processImg(cv::Mat img) {
   cv::Mat grayImg;
   cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);

   cv::Mat blurredImg;
   cv::GaussianBlur(grayImg, blurredImg, cv::Size(3, 3), 1);

   cv::Mat threshImg;
   cv::adaptiveThreshold(blurredImg, threshImg, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 25, 16);

   cv::Mat medianImg;
   cv::medianBlur(threshImg, medianImg, 3);

   cv::Mat dilateImg;
   cv::dilate(medianImg, dilateImg, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)),
              cv::Point(-1, -1), 2);

   cv::Mat erodeImg;
   cv::erode(dilateImg, erodeImg, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)),
             cv::Point(-1, -1), 1);

   return erodeImg;
}

int main() {
    std::vector<cv::Point> posList = getPosList(dataPath + "CarParkPos.txt");
    cv::VideoCapture cap(dataPath + "carPark.mp4");

    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open the video." << std::endl;
        return -1;
    }

    int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    cv::VideoWriter videoWriter(outputPath + "output.mp4", cv::VideoWriter::fourcc('m', 'p', '4', 'v'), 25,
                    cv::Size(frame_width, frame_height));

    while (true) {
        cv::Mat frame;
        bool success = cap.read(frame);

        if (!success) {
            std::cerr << "Error: Unable to read the frame." << std::endl;
            break;
        }

        cv::Mat resizedImg;
        cv::resize(frame, resizedImg, cv::Size(frame.cols, frame.rows));

        cv::Mat processedImg = processImg(resizedImg);
        checkParkScape(resizedImg, processedImg, posList);

        cv::imshow("Video", resizedImg);
        videoWriter.write(resizedImg);

        char c = (char)cv::waitKey(1);
        if (c == 'q')
            break;
    }

    videoWriter.release();
    cap.release();
    cv::destroyAllWindows();

    return 0;
}