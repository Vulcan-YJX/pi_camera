#include <iomanip>
#include <iostream>
#include <memory>
#include <thread>
#include "picamera.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

using namespace cv;

int main()
{

    uint32_t width = 640;
    uint32_t height = 480;
    uint32_t stride;
    int frame_count = 0;
    time_t start_time = time(0);

    PiCamera cam;
    ControlList controls_;
    int64_t frame_time = 1000000 / 10;

    // Set frame rate
	controls_.set(controls::FrameDurationLimits, libcamera::Span<const int64_t, 2>({ frame_time, frame_time }));
    // Adjust the brightness of the output images, in the range -1.0 to 1.0
    controls_.set(controls::Brightness, 0.5);
    // Adjust the contrast of the output image, where 1.0 = normal contrast
    controls_.set(controls::Contrast, 1.5);
    // Set the exposure time
    controls_.set(controls::ExposureTime, 20000);

    int ret = cam.initCamera();
    cam.getCameraId();
    cam.configureStill(width,height,formats::RGB888,1,1);
    cam.startCamera();

    // cam.set(controls_);
    if (!ret) {
        bool flag;
        LibcameraOutData frameData;
        cam.startCamera();
        cam.VideoStream(&width, &height, &stride);
        while (frame_count < 500) {
            flag = cam.readFrame(&frameData);
            if (!flag)
                continue;
            // Mat im(height, width, CV_8UC3, frameData.imageData, stride);
            frame_count++;
            if ((time(0) - start_time) >= 1){
                printf("fps: %d\n", frame_count);
                std::cout << frameData.size << std::endl;
                frame_count = 0;
                start_time = time(0);
                // imwrite("test.jpg",im);
            }
            cam.returnFrameBuffer(frameData);
        }
        // destroyAllWindows();
        cam.stopCamera();
    }
    cam.closeCamera();

    return 0;
}