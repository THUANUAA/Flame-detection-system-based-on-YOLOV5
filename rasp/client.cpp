#include <MQTTClient.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

#define LABELS_DIR "image_detect/labels/"
#define SPI_DEVICE "/dev/spidev0.1"
#define SPI_MODE SPI_MODE_3
#define SPI_BITS 8
#define SPI_SPEED 500000
#define SPI_DELAY 0

#define MQTT_Uri "tcp://8.138.13.104:1883"
#define ClientId "client_sender"
#define UserName "interest"
#define PassWord "0731666"
#define TOPIC_IMAGE "image_topic"
#define TOPIC_DATA "data_topic"
#define QOS 1
#define IMAGE_FILE "image.txt"
#define IMAGE_DETECT_FILE "image_detect/image_detect.txt"
#define DATA_ADDR "/dev/mydht11"

int spi_fd;
int signal_flag;

// 初始化 SPI
void spi_init()
{
    int ret;
    uint8_t mode = SPI_MODE;
    uint8_t bits = SPI_BITS;
    uint32_t speed = SPI_SPEED;

    spi_fd = open(SPI_DEVICE, O_RDWR);
    if (spi_fd < 0) {
        perror("无法打开 SPI 设备");
        exit(1);
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1) {
        perror("无法设置 SPI 模式");
        exit(1);
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) {
        perror("无法设置 SPI 位数");
        exit(1);
    }

    ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1) {
        perror("无法设置 SPI 速度");
        exit(1);
    }
}

// SPI 发送并接收数据
uint8_t spi_transfer(uint8_t tx_byte)
{
    int ret;
    uint8_t rx_byte;
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&tx_byte,
        .rx_buf = (unsigned long)&rx_byte,
        .len = 1,
        .speed_hz = SPI_SPEED, // speed_hz 应该在 delay_usecs 之前
        .delay_usecs = SPI_DELAY, // delay_usecs 在 speed_hz 之后
        .bits_per_word = SPI_BITS
    };

    ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        perror("无法发送 SPI 消息\r\n");
    }
    return rx_byte;
}

// 关闭 SPI
void spi_close() { close(spi_fd); }

// 检测 LABELS 目录中文件的数量
int get_labels_file_count()
{
    int file_count = 0;
    DIR* dir = opendir(LABELS_DIR);
    if (dir == nullptr) {
        perror("无法打开标签目录");
        return -1;
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_REG) {
            file_count++;
        }
    }
    closedir(dir);
    return file_count;
}

// 监控 labels 文件夹并发送 SPI 消息
void monitor_labels_and_send_spi()
{
    int last_file_count = get_labels_file_count();

    while (true) {
        int current_file_count = get_labels_file_count();
        if (current_file_count > last_file_count) {
            std::cout << "检测到火焰，发送报警信号..." << std::endl;
            spi_transfer(0xAA); // 发送 0xAA 代表检测到火焰
            signal_flag = 1;
        } else {
            std::cout << "未检测到火焰，发送正常信号..." << std::endl;
            spi_transfer(0xBB); // 发送 0xBB 代表未检测到火焰
            signal_flag = 0;
        }
        last_file_count = current_file_count;
        std::this_thread::sleep_for(std::chrono::seconds(15)); // 每10秒检测一次
    }
}

void send_image_and_data(MQTTClient client, const char* file_path,
    const char* data_path)
{
    // 发送图片
    if (file_path != nullptr) {
        FILE* file = fopen(file_path, "rb");
        if (!file) {
            std::cerr << "Failed to open image file: " << file_path << std::endl;
        } else {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            char* buffer = new char[file_size];
            fread(buffer, 1, file_size, file);
            fclose(file);

            MQTTClient_message pubmsg = MQTTClient_message_initializer;
            pubmsg.payload = buffer;
            pubmsg.payloadlen = file_size;
            pubmsg.qos = QOS;
            pubmsg.retained = 0;

            MQTTClient_deliveryToken token;
            int rc = MQTTClient_publishMessage(client, TOPIC_IMAGE, &pubmsg, &token);
            if (rc != MQTTCLIENT_SUCCESS) {
                std::cerr << "Failed to publish image, return code " << rc << std::endl;
            } else {
                std::cout << "Image published successfully." << std::endl;
            }

            delete[] buffer;
        }
    }
    // 发送数据
    if (data_path != nullptr) {
        int fd = open(data_path, O_RDWR|O_NONBLOCK);
        if (fd == -1) {
            std::cerr << "Cannot open data file: " << data_path << std::endl;
            return;
        }
        char buf[2];
        ssize_t bytes_read = read(fd, buf, sizeof(buf)); // 使用 ssize_t 以便捕捉负值
        if (bytes_read < 0) {
            std::cerr << "Failed to read data, error: " << strerror(errno)
                      << std::endl;
        } else if(bytes_read==0){
			std::cerr<<"Reached end of file: "<< data_path << std::endl;
		} else if (bytes_read < static_cast<ssize_t>(sizeof(buf))) {
            std::cerr << "Read incomplete data: expected " << sizeof(buf) << " bytes but got " << bytes_read << " bytes." << std::endl;
        }else if (bytes_read == sizeof(buf)) {
			char data[3];
			data[0]=buf[0];
			data[1]=buf[1];
			data[2]=static_cast<char>(signal_flag);;
            
			MQTTClient_message pubmsg = MQTTClient_message_initializer;
            pubmsg.payload = data;
            pubmsg.payloadlen = sizeof(data);
            pubmsg.qos = QOS;
            pubmsg.retained = 0;
            MQTTClient_deliveryToken token;

            int rc = MQTTClient_publishMessage(client, TOPIC_DATA, &pubmsg, &token);
            if (rc != MQTTCLIENT_SUCCESS) {
                std::cerr << "Failed to publish data, return code " << rc << std::endl;
            } else {
                std::cout << "Data published successfully. Humidity= " << static_cast<int>(data[0])
                          << "   Temperature= " << static_cast<int>(data[1]) << "   signal= " << static_cast<int>(data[2])
                          << std::endl;
            }
        } else {
            std::cerr << "Read incomplete data: expected " << sizeof(buf)
                      << " bytes but got " << bytes_read << " bytes." << std::endl;
        }
        close(fd);
    }
}

// 监视 image_detect.txt 文件并发送检测后的图片
void monitor_detected_images(MQTTClient client)
{
    std::string last_detected_image;
    off_t last_file_size = 0;

    while (true) {
        std::ifstream image_file(IMAGE_DETECT_FILE,
            std::ios::ate); // 以读取文件末尾方式打开
        if (image_file.is_open()) {
            off_t current_file_size = image_file.tellg(); // 获取当前文件大小
            if (current_file_size > last_file_size) {
                // 文件有新内容，回到上次记录的大小位置
                image_file.seekg(last_file_size);
                std::string new_line;
                while (getline(image_file, new_line)) {
                    if (!new_line.empty() && new_line != last_detected_image) {
                        last_detected_image = new_line;
                        std::cout << "Detected new image for MQTT: " << new_line
                                  << std::endl;
                        // 发送图片和数据
                        send_image_and_data(client, new_line.c_str(), DATA_ADDR);
                    }
                }
                last_file_size = current_file_size; // 更新文件大小
            }
            image_file.close();
        } else {
            std::cerr << "Failed to open image_detect.txt" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(15)); // 每15秒检测一次
    }
}

// 运行 Python 脚本
void run_python_script(const std::string& script_name)
{
    std::cout << "Starting " << script_name << "..." << std::endl;
    std::string command = "python " + script_name + " &"; // 后台运行
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Failed to start " << script_name << std::endl;
    }
}

// 运行 detect.py 读取 image.txt 并检测图片
void run_detect()
{
    std::string last_image;
    off_t last_file_size = 0;

    while (true) {
        std::ifstream image_file(IMAGE_FILE,
            std::ios::ate); // 以读取文件末尾方式打开
        if (image_file.is_open()) {
            off_t current_file_size = image_file.tellg(); // 获取当前文件大小
            if (current_file_size > last_file_size) {
                // 文件有新内容，回到上次记录的大小位置
                image_file.seekg(last_file_size);
                std::string received_image;
                while (getline(image_file, received_image)) {
                    if (!received_image.empty() && received_image != last_image) {
                        last_image = received_image;
                        std::cout << "Running detect.py for image: " << received_image
                                  << std::endl;
                        std::string detect_command = "python ../share/project/yolov5/detect.py "
                                                     "--weights best.pt "
                                                     "--img 640 --conf 0.25 --project image_detect/ "
                                                     "--save-txt "
                                                     "--source "
                            + received_image;
                        int result = system(detect_command.c_str());
                        if (result != 0) {
                            std::cerr << "Failed to run detect.py for image: "
                                      << received_image << std::endl;
                        }
                    }
                }
                last_file_size = current_file_size; // 更新文件大小
            }
            image_file.close();
        } else {
            std::cerr << "Failed to open image.txt" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(15)); // 每30秒检测一次
    }
}

int main()
{
    // 初始化 MQTT 客户端
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.username = UserName;
    conn_opts.password = PassWord;

    spi_init();

    if (MQTTClient_create(&client, MQTT_Uri, ClientId,
            MQTTCLIENT_PERSISTENCE_NONE,
            NULL)
        != MQTTCLIENT_SUCCESS) {
        std::cerr << "Failed to create MQTT client" << std::endl;
        return -1;
    }

    if (MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
        std::cerr << "Failed to connect to MQTT broker" << std::endl;
        MQTTClient_destroy(&client);
        return -1;
    }

    // 启动 imagesget.py 线程
    std::thread imagesget_thread(run_python_script, "imagesget.py");

    // 启动 detect.py 线程
    std::thread detect_thread(run_detect);

    // 启动监视 image_detect.txt 的线程
    std::thread monitor_thread(monitor_detected_images, client);

    // 启动 labels 目录监控线程
    std::thread labels_monitor_thread(monitor_labels_and_send_spi);

    // 主线程循环可以处理其他逻辑（如果需要）
    while (true) {
        sleep(1); // 保持主线程运行
    }

    // 等待线程结束（通常不会到达这里）
    imagesget_thread.join();
    detect_thread.join();
    monitor_thread.join();
    labels_monitor_thread.join();
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    // 关闭 SPI 设备
    spi_close();

    return 0;
}
