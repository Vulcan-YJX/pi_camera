#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace std;

// 从 GStreamer 的 appsink 获取视频帧并处理
GstFlowReturn process_frame(GstAppSink *appsink) {
    // 从 appsink 中获取 sample
    GstSample *sample = gst_app_sink_pull_sample(appsink);
    if (!sample) {
        cerr << "Failed to pull sample from appsink!" << endl;
        return GST_FLOW_ERROR;
    }

    // 从 sample 获取缓冲区
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstCaps *caps = gst_sample_get_caps(sample);
    if (!buffer || !caps) {
        cerr << "Failed to get buffer or caps from sample!" << endl;
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    // 获取帧的宽度、高度和格式
    GstStructure *structure = gst_caps_get_structure(caps, 0);
    int width, height;
    gst_structure_get_int(structure, "width", &width);
    gst_structure_get_int(structure, "height", &height);

    // 获取帧数据
    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        cerr << "Failed to map buffer!" << endl;
        gst_sample_unref(sample);
        return GST_FLOW_ERROR;
    }

    // 将缓冲区数据转换为 OpenCV 格式
    // cv::Mat frame(height, width, CV_8UC3, (void *)map.data);
    // cv::Mat frame_bgr;
    // cv::cvtColor(frame, frame_bgr, cv::COLOR_RGB2BGR);

    // 获取时间戳生成文件名
    // auto now = chrono::system_clock::now();
    // auto now_time = chrono::system_clock::to_time_t(now);
    // auto now_ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // stringstream timestamp;
    // timestamp << put_time(localtime(&now_time), "%Y%m%d_%H%M%S") << "_" << setw(3) << setfill('0') << now_ms.count();
    // string filename = "frame_" + timestamp.str() + ".jpg";

    // 保存图片到本地
    // cv::imwrite(filename, frame_bgr);

    // 打印帧信息
    cout << "Frame width: " << width << ", height: " << height << std::endl;
        //  << ", size: " << map.size << " bytes, saved to: " << filename << endl;

    // 释放资源
    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);

    return GST_FLOW_OK;
}

// GStreamer 的帧接收回调
GstFlowReturn on_new_sample(GstAppSink *appsink, gpointer user_data) {
    return process_frame(appsink);
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    // 定义 GStreamer 管道
    string pipeline_desc = 
        "libcamerasrc ! "                 // 使用树莓派的 CSI 摄像头
        "videoconvert ! "                 // 转换为 RGB 格式
        "video/x-raw,format=RGB,width=640,height=480 ! "  // 指定分辨率和格式
        "appsink name=appsink emit-signals=true sync=false"; // 使用 appsink 提取帧

    GError *error = nullptr;
    GstElement *pipeline = gst_parse_launch(pipeline_desc.c_str(), &error);
    if (!pipeline) {
        cerr << "Failed to create GStreamer pipeline: " << error->message << endl;
        g_error_free(error);
        return -1;
    }

    // 获取 appsink 元素
    GstElement *appsink = gst_bin_get_by_name(GST_BIN(pipeline), "appsink");
    if (!appsink) {
        cerr << "Failed to get appsink from pipeline!" << endl;
        gst_object_unref(pipeline);
        return -1;
    }

    // 设置 appsink 的回调函数
    GstAppSinkCallbacks callbacks = {nullptr, nullptr, on_new_sample};
    gst_app_sink_set_callbacks(GST_APP_SINK(appsink), &callbacks, nullptr, nullptr);

    // 启动管道
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    cout << "Pipeline running. Press Ctrl+C to exit." << endl;

    // 运行主循环
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    // 停止管道
    gst_element_set_state(pipeline, GST_STATE_NULL);

    // 释放资源
    gst_object_unref(appsink);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}