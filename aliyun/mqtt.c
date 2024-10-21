#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <sqlite3.h>

#define MQTT_Uri        "tcp://8.138.13.104:1883"
#define ClientId        "client_receiver"
#define UserName        "interest"
#define PassWord        "0731666"
#define TOPIC_IMAGE     "image_topic"
#define TOPIC_TEXT      "data_topic"
#define QOS             1
#define IMAGE_DIR       "/root/project/images/"

static int image_count = 1;  // 接收到的图片计数器

static char* image_path = NULL;
static char* data = NULL;

// 保存图片的函数
void save_image(const char* filename, const char* data, int len) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return;
    }

    fwrite(data, 1, len, file);
    fclose(file);
    printf("Image saved to %s\n", filename);
}

// 将图片路径写入SQLite数据库
void write_to_db(void) {
    if (image_path == NULL || data == NULL) {
        printf("Image path or data is NULL, skipping database write.\n");
        return;
    }

    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *sql_insert = "INSERT INTO Images(Path, Humidity, Temperature, Signal) VALUES(?, ?, ?, ?);";
    char *err_msg = 0;

    if (sqlite3_open("/root/project/images.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char *sql_create = "CREATE TABLE IF NOT EXISTS Images(Id INTEGER PRIMARY KEY, Path TEXT, Humidity INTEGER, Temperature INTEGER, Signal INTEGER);";
    if (sqlite3_exec(db, sql_create, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL_create error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }

    if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, image_path, -1, SQLITE_STATIC);  // 绑定图片路径
    sqlite3_bind_int(stmt, 2, data[0]);                         // 绑定湿度
    sqlite3_bind_int(stmt, 3, data[1]);                         // 绑定温度
    sqlite3_bind_int(stmt, 4, data[2]);                         // 绑定信号强度

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Data saved to database: Path=%s, Humidity=%d, Temperature=%d, Signal=%d\n", image_path, data[0], data[1], data[2]);
    }

    sqlite3_finalize(stmt);  // 释放语句
    sqlite3_close(db);  // 关闭数据库

    // 释放 image_path 内存并重置全局变量
    free(image_path);
    free(data);
    image_path = NULL;
    data = NULL;
}

// 消息到达处理函数
int message_arrived(void* context, char* topicName, int topicLen, MQTTClient_message* message) {
    if (strcmp(topicName, TOPIC_IMAGE) == 0) {
        char filename[256];
        snprintf(filename, sizeof(filename), "%simage%d.png", IMAGE_DIR, image_count);  // 创建图片名称
        printf("Image received, saving...\n");

        save_image(filename, message->payload, message->payloadlen);  // 保存图片

        // 动态分配 image_path
        image_path = malloc(strlen(filename) + 1);
        if (image_path != NULL) {
            strcpy(image_path, filename);
        } else {
            printf("Failed to allocate memory for image_path\n");
        }

        image_count++;  // 更新计数器
    } else if (strcmp(topicName, TOPIC_TEXT) == 0) {
        data = malloc(message->payloadlen);  // 为数据分配内存
        if (data != NULL) {
            memcpy(data, message->payload, message->payloadlen);  // 复制文本消息内容
        } else {
            printf("Failed to allocate memory for data\n");
        }
    }

    // 同时收到图片和文本后，写入数据库
    if (image_path != NULL && data != NULL) {
        write_to_db();
    }

    // 清理 MQTT 消息
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);

    return 1;
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.username = UserName;
    conn_opts.password = PassWord;

    MQTTClient_create(&client, MQTT_Uri, ClientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(client, NULL, NULL, message_arrived, NULL);

    int rc = MQTTClient_connect(client, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        return -1;
    }
    printf("Connected successfully!\n");

    // 订阅图片和文本话题
    rc = MQTTClient_subscribe(client, TOPIC_IMAGE, QOS);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to subscribe to image topic, return code %d\n", rc);
    }

    rc = MQTTClient_subscribe(client, TOPIC_TEXT, QOS);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("Failed to subscribe to text topic, return code %d\n", rc);
    }

    printf("Press Q or q + <Enter> to quit\n\n");
    int ch;
    do {
        ch = getchar();
    } while (ch != 'Q' && ch != 'q');

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return 0;
}

