import gi
import numpy as np
import cv2
from datetime import datetime

gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

def process_frame(sample):
    """
    从 GStreamer 的 appsink 提取视频帧，并打印宽度、高度和数据尺寸。
    """
    # 从 sample 中获取数据缓冲区
    buffer = sample.get_buffer()

    # 获取帧数据大小
    size = buffer.get_size()

    # 提取帧元数据（宽度、高度等信息）
    caps = sample.get_caps()
    structure = caps.get_structure(0)
    width = structure.get_value('width')
    height = structure.get_value('height')

    # 将缓冲区数据解析为 numpy 数组
    data = np.ndarray(
        (height, width, 3),  # 假设为 RGB 图像（3 通道）
        buffer=buffer.extract_dup(0, size),
        dtype=np.uint8
    )

    frame_bgr = cv2.cvtColor(data, cv2.COLOR_RGB2BGR)

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
    filename = f"frame_{timestamp}.jpg"

    # 保存图片到本地
    #cv2.imwrite(filename, frame_bgr)

    # 打印帧信息
    print(f"Frame width: {width}, height: {height}, size: {size} bytes")

    # 返回是否继续处理
    return True

def main():
    # 初始化 GStreamer
    Gst.init(None)

    # 定义 GStreamer 管道
    pipeline_str = (
        "libcamerasrc ! "                   # 使用树莓派的 CSI 摄像头
        "videoconvert ! "                   # 转换为 RGB 格式
        "video/x-raw,format=RGB,width=640,height=480 ! "  # 指定分辨率和格式
        "appsink name=appsink emit-signals=true sync=false"  # 使用 appsink 提取帧
    )

    # 创建 GStreamer 管道
    pipeline = Gst.parse_launch(pipeline_str)

    # 获取 appsink 元素
    appsink = pipeline.get_by_name("appsink")

    # 设置 appsink 属性
    appsink.set_property("emit-signals", True)
    appsink.set_property("sync", False)

    # 连接信号，处理帧数据
    appsink.connect("new-sample", lambda sink: process_frame(sink.emit("pull-sample")) or Gst.FlowReturn.OK)

    # 启动管道
    pipeline.set_state(Gst.State.PLAYING)

    try:
        # 运行主循环
        loop = GLib.MainLoop()
        print("Pipeline running. Press Ctrl+C to exit.")
        loop.run()
    except KeyboardInterrupt:
        print("Exiting...")

    # 停止管道
    pipeline.set_state(Gst.State.NULL)

if __name__ == "__main__":
    main()
