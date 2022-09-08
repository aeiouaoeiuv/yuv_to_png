/**
 * Converting yuv format to png by this command below
 * v4l2-ctl -d /dev/video0 --set-fmt-video=width=1280,height=720,pixelformat=NV12 --stream-mmap=3 --stream-skip=3 --stream-to=/tmp/out.yuv --stream-count=10 --stream-poll
 */
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <string>

struct Arguments {
    std::string              input_file;
    std::string              output_dir;
    uint32_t                 width;
    uint32_t                 height;
    cv::ColorConversionCodes cvt_code;
};

void PrintUsage(int argc, char *argv[]) {
    std::string version = "0.0.1";
    std::cout << "Version " << version << std::endl;

    std::cout << "Usage: " << argv[0] << " [OPTION] [VALUE]" << std::endl
              << std::endl
              << "options:" << std::endl
              << "    --help                    this help" << std::endl
              << "    --input <yuv_file>        input a yuv file" << std::endl
              << "    --output <dir>            output image to dir" << std::endl
              << "    --width <int>             width in number. default: 1280" << std::endl
              << "    --height <int>            height in number. default: 720" << std::endl
              << "    --format <nv12|nv21>      default: nv12" << std::endl
              << std::endl
              << "e.g.:" << std::endl
              << "    " << argv[0] << " --input test.yuv --output dir" << std::endl;
}

void CheckAndCreateDir(const std::string &dir) {
    struct stat buf;
    int         ret = stat(dir.c_str(), &buf);
    if (0 != ret) {
        mkdir(dir.c_str(), 0664);
    }
}

std::string GetBasename(const std::string &pathname) {
    size_t pos = pathname.rfind("/");
    if (pos != std::string::npos) {
        return pathname.substr(pos + 1, pathname.length());
    }

    return "";
}

std::string GetBasenamePrefix(const std::string &basename) {
    size_t pos = basename.rfind(".");
    if (pos != std::string::npos) {
        return basename.substr(0, pos);
    }

    return "";
}

int Convert(const Arguments &args) {
    std::ifstream input_file(args.input_file);
    if (false == input_file.is_open()) {
        return 1;
    }

    std::string file_content;
    file_content = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

    uint32_t frame_size = args.width * args.height * 1.5;
    if (frame_size > file_content.size()) {
        return 2;
    }

    // create dir if not exist
    CheckAndCreateDir(args.output_dir);

    auto basename        = GetBasename(args.input_file);
    auto basename_prefix = GetBasenamePrefix(basename);

    int      frame_count = file_content.size() / frame_size;
    int      data_type   = CV_8UC1;
    cv::Mat  img;
    cv::Size raw_size;
    raw_size.width  = args.width;
    raw_size.height = args.height * 1.5;  // height*1.5

    for (int i = 0; i < frame_count; ++i) {
        uint32_t offset = i * frame_size;
        uint8_t *data   = const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(file_content.data()));

        std::string output_file = args.output_dir + "/" + basename_prefix + std::to_string(i + 1) + ".png";

        cv::Mat raw_img = cv::Mat(raw_size, data_type, data + offset);
        cv::cvtColor(raw_img, img, args.cvt_code);
        cv::imwrite(output_file, img);

        std::cout << output_file << std::endl;
    }

    return 0;
}

Arguments ParseArguments(int argc, char *argv[]) {
    struct option long_opts[] = {
        {"help",   no_argument,       0, 'h'},
        {"input",  required_argument, 0, 'i'},
        {"width",  required_argument, 0, 'W'},
        {"height", required_argument, 0, 'H'},
        {"output", required_argument, 0, 'o'},
        {"format", required_argument, 0, 'f'},
        {0,        0,                 0, 0  }
    };

    int       opt;
    int       option_idx;
    Arguments args = {"", "", 1280, 720, cv::COLOR_YUV2RGB_NV12};
    while (true) {
        opt = getopt_long(argc, argv, "hi:o:W:H:f:", long_opts, &option_idx);
        if (-1 == opt) {
            break;
        }

        switch (opt) {
        case 'h':
            PrintUsage(argc, argv);
            exit(0);

        case 'i':
            args.input_file = optarg;
            break;

        case 'o':
            args.output_dir = optarg;
            break;

        case 'W':
            args.width = std::stoi(optarg);
            break;

        case 'H':
            args.height = std::stoi(optarg);
            break;

        case 'f': {
            std::string format = optarg;
            if (format == "nv21") {
                args.cvt_code = cv::COLOR_YUV2RGB_NV21;
            }
            else if (format == "nv12") {
                args.cvt_code = cv::COLOR_YUV2RGB_NV12;
            }
            break;
        }

        default:
            PrintUsage(argc, argv);
            exit(1);
        }
    }

    if (args.output_dir.empty()) {
        args.output_dir = ".";
    }

    return args;
}

int main(int argc, char *argv[]) {
    auto args = ParseArguments(argc, argv);

    return Convert(args);
}
