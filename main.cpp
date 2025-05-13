#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/camera.h>
#include <libcamera/request.h>
#include <libcamera/framebuffer_allocator.h>
#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

using namespace libcamera;

int main() {
    // 创建 CameraManager
    std::shared_ptr<CameraManager> cameraManager = std::make_shared<CameraManager>();
    int ret = cameraManager->start(); // 启动 CameraManager
    if (ret) {
        std::cerr << "Failed to start CameraManager!" << std::endl;
        return -1;
    }

    // 获取可用摄像头
    if (cameraManager->cameras().empty()) {
        std::cerr << "No cameras found!" << std::endl;
        return -1;
    }

    std::shared_ptr<Camera> camera = cameraManager->cameras()[0]; // 获取第一个摄像头
    std::cout << "Using camera: " << camera->id() << std::endl;

    // 打开摄像头
    if (camera->acquire()) {
        std::cerr << "Failed to acquire camera!" << std::endl;
        return -1;
    }

    // 配置摄像头
    std::unique_ptr<CameraConfiguration> config = camera->generateConfiguration({ StreamRole::VideoRecording });
    if (!config || config->size() == 0) {
        std::cerr << "Failed to generate camera configuration!" << std::endl;
        return -1;
    }

    // 设置分辨率和帧率
    StreamConfiguration &streamConfig = config->at(0);
    streamConfig.size.width = 640;
    streamConfig.size.height = 480;
    streamConfig.pixelFormat = formats::NV12; // NV12 格式
    streamConfig.bufferCount = 4;

    if (camera->configure(config.get())) {
        std::cerr << "Failed to configure camera!" << std::endl;
        return -1;
    }

    // 为帧缓冲分配内存
    FrameBufferAllocator allocator(camera);
    for (StreamConfiguration &cfg : *config) {
        Stream *stream = cfg.stream();
        if (allocator.allocate(stream) < 0) {
            std::cerr << "Failed to allocate buffers!" << std::endl;
            return -1;
        }
    }

    // 获取缓冲区
    Stream *stream = config->at(0).stream();
    const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator.buffers(stream);

    // 注册帧完成事件的回调函数
    camera->requestCompleted.connect([](Request *request) {
        if (request->status() == Request::RequestComplete) {
            std::cout << "Frame captured successfully!" << std::endl;
        } else {
            std::cerr << "Frame capture failed!" << std::endl;
        }
    });

    // 启动摄像头
    if (camera->start()) {
        std::cerr << "Failed to start camera!" << std::endl;
        return -1;
    }

    for (int i = 0; i < 100; i++) { // 捕获 100 帧
        // 创建 Request 对象
        std::unique_ptr<Request> request = camera->createRequest();
        if (!request) {
            std::cerr << "Failed to create request!" << std::endl;
            break;
        }

        // 将缓冲区绑定到 Request
        for (const std::unique_ptr<FrameBuffer> &buffer : buffers) {
            if (request->addBuffer(stream, buffer.get()) < 0) {
                std::cerr << "Failed to add buffer to request!" << std::endl;
                return -1;
            }
        }

        // 将 Request 加入队列
        if (camera->queueRequest(request.get()) < 0) {
            std::cerr << "Failed to queue request!" << std::endl;
            break;
        }

        // 等待帧完成的回调处理
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟延时
    }

    // 停止摄像头
    camera->stop();
    camera->release();
    cameraManager->stop();

    std::cout << "Camera capture finished!" << std::endl;
    return 0;
}
