#include "TRTModule.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <opencv2/opencv.hpp>

std::string getTypeName(int tag_id) {
    // 根据 tag_id 返回相应的类型名称
    // 这里只是一个示例，您需要根据实际情况进行调整
    switch (tag_id) {
        case 0: return "类型1";
        case 1: return "类型2";
        // 其他类型...
        default: return "未知";
    }
}
int main() {
    std::string directory = "/home/zyi/Desktop/jidegitclone (copy)/models/"; // 模型文件所在的目录
    std::string videoPath = "/home/zyi/Desktop/jidegitclone/data/test.avi";  // 视频文件路径

    // 列出目录中所有 .onnx 和 .cache 文件
    std::set<std::string> model_files;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        std::string path = entry.path().string();
        if (path.ends_with(".onnx") || path.ends_with(".cache")) {
            // 移除文件后缀
            std::string base_name = path.substr(0, path.find_last_of("."));
            model_files.insert(base_name);
        }
    }

    // 显示模型文件给用户并询问选择哪个模型
    std::vector<std::string> choices;
    int idx = 0;
    for (const auto& file : model_files) {
        std::cout << ++idx << ". " << file << std::endl;
        choices.push_back(file);
    }

    int choice;
    std::cout << "请选择一个模型（输入数字）: ";
    std::cin >> choice;
    if (choice < 1 || choice > choices.size()) {
        std::cerr << "无效选择" << std::endl;
        return -1;
    }

    // 获取用户选择的模型基础路径
    std::string selected_model_base_path = choices[choice - 1];
    std::string onnx_path = selected_model_base_path + ".onnx";
    std::string cache_path = selected_model_base_path + ".cache";
    
    // 创建 TRTModule 对象
    TRTModule trtModule(onnx_path, cache_path);

    // 打开视频文件
    cv::VideoCapture cap(videoPath);
    if (!cap.isOpened()) {
        std::cerr << "无法打开视频文件: " << videoPath << std::endl;
        return -1;
    }

    // 打开视频文件
        cv::Mat frame;
    while (cap.read(frame)) {
        std::vector<bbox_t> boxes = trtModule(frame);

        float total_confidence = 0;
        for (const auto& box : boxes) {
            // 绘制边界框
            for (int i = 0; i < 4; i++) {
                cv::line(frame, box.pts[i], box.pts[(i+1)%4], cv::Scalar(0, 255, 0), 2);
            }

            // 获取类型名称
            std::string type_name = getTypeName(box.tag_id); // 假设 getTypeName 函数根据 tag_id 返回类型名称

            // 绘制类型和置信度
            std::string label = type_name + " - " + std::to_string(box.confidence);
            cv::putText(frame, label, box.pts[0], cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 255), 1);

            total_confidence += box.confidence;
        }

        // 计算并显示平均置信度
        if (!boxes.empty()) {
            float avg_confidence = total_confidence / boxes.size();
            std::string avg_conf_label = "Average Confidence: " + std::to_string(avg_confidence);
            cv::putText(frame, avg_conf_label, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2);
        }

        // 显示处理后的视频帧
        cv::imshow("Detected Objects", frame);
        if (cv::waitKey(30) == 27) { // 按 'ESC' 键退出
            break;
        }
    }

    return 0;
}

// getTypeName 函数的一个简单实现
