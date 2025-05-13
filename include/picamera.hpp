#include <memory>
#include <climits>
#include <mutex>
#include <queue>
#include "sys/mman.h"
#include <libcamera/libcamera.h>

using namespace libcamera;

typedef struct {
    uint8_t *imageData;
    uint32_t size;
    uint64_t request;
} LibcameraOutData;

class PiCamera
{
private:
    std::unique_ptr<CameraManager> cm;
    std::string cameraId;
    std::shared_ptr<Camera> camera_;
    bool camera_acquired_ = false;
    std::unique_ptr<CameraConfiguration> config_;
    std::unique_ptr<FrameBufferAllocator> allocator_;
    std::vector<std::unique_ptr<Request>> requests_;
    std::map<int, std::pair<void *, unsigned int>> mappedBuffers_;

    ControlList controls_;
    bool camera_started_ = false;
    Stream *viewfinder_stream_ = nullptr;
    std::queue<Request *> requestQueue;

    std::mutex control_mutex_;
    std::mutex camera_stop_mutex_;
    std::mutex free_requests_mutex_;

    void requestComplete(Request *request);
    int queueRequest(Request *request);
    void processRequest(Request *request);
    int startCapture();
    void StreamDimensions(Stream const *stream, uint32_t *w, uint32_t *h, uint32_t *stride) const;

public:
    PiCamera(/* args */){};
    ~PiCamera(){};
    int initCamera();
    char *getCameraId();
    void configureStill(int width, int height, PixelFormat format, int buffercount, int rotation);
    int startCamera();
    Stream *VideoStream(uint32_t *w, uint32_t *h, uint32_t *stride) const;
    bool readFrame(LibcameraOutData *frameData);
    void returnFrameBuffer(LibcameraOutData frameData);
    void stopCamera();
    void closeCamera();
    void set(ControlList controls);
    
};