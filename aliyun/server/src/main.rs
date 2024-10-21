use actix_web::{web, App, HttpResponse, HttpServer, Responder};
use rusqlite::{params, Connection, Result};
use rusqlite::OptionalExtension;
use actix_files::Files;
use std::process::Command;
use std::io::{self};

#[derive(Debug)]
struct Image {
    id: i32,
    path: String,
    humidity: i32,
    temperature: i32,
    signal: i32,
}

fn get_last_image_from_db() -> Result<Option<Image>> {
    let db_path = "/root/project/images.db";
    println!("Attempting to connect to database at: {}",db_path);
    let conn = Connection::open(db_path)?;
    println!("Successfully connected to the database.");

    let mut stmt = conn.prepare("SELECT Id, Path, Humidity, Temperature, Signal FROM Images ORDER BY Id DESC LIMIT 1")?;
    println!("Prepared statement to query the last image");

    let image = stmt.query_row(params![], |row| {
        Ok(Image {
            id: row.get(0)?,
            path: row.get(1)?,
            humidity: row.get(2)?,
            temperature: row.get(3)?,
            signal: row.get(4)?,
        })
    }).optional()?;

    if image.is_some(){
        println!("Retrieved image:{:?}",image);
    }else{
        println!("No image found in database.");
    }

    Ok(image)
}

async fn show_image() -> impl Responder {
    match get_last_image_from_db() {
        Ok(Some(image)) => {
            HttpResponse::Ok().content_type("text/html").body(format!(
               r#"<html lang="zh">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Image Details</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            background-color: #f4f4f4;
            color: #333;
            margin: 20px;
            text-align: center;
        }}

        h1 {{
            color: #4A90E2;
            /* 标题颜色 */
            font-size: 36px;
            /* 增加字体大小 */
            margin-bottom: 20px;
            /* 增加底部间距 */
        }}

        img {{
            max-width: 70%;
            height: auto;
            border: 2px solid #4A90E2;
            border-radius: 8px;
            margin: 15px 0;
        }}

        .data {{
            background-color: #fff;
            border: 1px solid #ccc;
            border-radius: 5px;
            padding: 20px;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
            display: inline-block;
            /* 使数据框体居中 */
            margin-top: 20px;
        }}

        .humidity {{
            color: #4CAF50;
            /* 湿度显示为绿色 */
            font-size: 24px;
            /* 增加字体大小 */
        }}

        .temperature {{
            color: #E94E77;
            /* 温度显示为红色 */
            font-size: 24px;
            /* 增加字体大小 */
            margin-top: 10px;
            /* 增加顶部间距 */
        }}

        .alert {{
            background-color: #ffcc00;
            color: #333;
            font-weight: bold;
            padding: 15px;
            border-radius: 5px;
            margin-top: 20px;
            display: none;
            /* 默认隐藏 */
        }}

        .alert-critical {{
            background-color: #ff4d4d;
            /* 紧急警告的背景色 */
            color: #fff;
        }}
    </style>
</head>

<body>
    <h1>Image ID: {}</h1>
    <img src='{}' alt='Image not found' />
    <div class="data">
        <p class="humidity">Humidity: {:.2} %</p>
        <p class="temperature">Temperature: {:.2} &deg;C</p> <!-- 使用 &deg; 显示摄氏度符号 -->
    </div>
    <div id="alert" class="alert"></div>

    <script>
        const flameDetected = {}; // 这里填写 Rust 传来的布尔值，1 表示检测到火苗，0 表示未检测到火苗

        // 定义消息常量
        const FLAME_DETECTED_MESSAGE = "🔥 检测到火焰！请立即采取行动！";
        const FLAME_NOT_DETECTED_MESSAGE = "未检测到火焰。";

        // 获取 alertBox 元素
        const alertBox = document.getElementById('alert');

        // 判断火焰检测值并设置警告信息
        if (flameDetected === null) {{
            alertBox.textContent = FLAME_DETECTED_MESSAGE;
            alertBox.classList.add('alert-critical'); // 添加紧急警告样式
            alertBox.style.display = "block"; // 显示弹窗
            alert(FLAME_DETECTED_MESSAGE); // 弹出警告
        }} else {{
            alertBox.textContent = FLAME_NOT_DETECTED_MESSAGE;
            alertBox.classList.remove('alert-critical'); // 移除紧急警告样式（如果有的话）
            alertBox.style.display = "block"; // 显示信息
            alert(FLAME_NOT_DETECTED_MESSAGE); // 弹出警告
        }}
    </script>
</body>
</html>"#,
    image.id,
    image.path.strip_prefix("/root/project").unwrap_or(&image.path),
    image.humidity,
    image.temperature,
    image.signal,
            ))
        }  
        Ok(None) => HttpResponse::Ok().body("No images found in database."),
        Err(_e) => {  // 使用 _e 以消除警告
            println!("Error retrieving image from database: {:?}",_e);
            HttpResponse::InternalServerError().body("Error retrieving image from database")
        },
    }
}

fn run_mqtt_client() {
    let mut child = Command::new("/root/project/mqtt")
        .spawn()
        .expect("Failed to start mqtt client");

    if let Some(mut output) = child.stdout.take() {
        io::copy(&mut output, &mut io::stdout()).expect("Failed to copy stdout");
    }

    if let Some(mut error) = child.stderr.take() {
        io::copy(&mut error, &mut io::stderr()).expect("Failed to copy stderr");
    }
}

#[actix_web::main]
async fn main() -> std::io::Result<()> {
    // 在新的线程中运行 MQTT 客户端
    std::thread::spawn(|| {
        run_mqtt_client();
    });

    // 启动 HTTP 服务器
    HttpServer::new(|| {
        App::new()
            .service(Files::new("/images", "/root/project/images")) // 设置静态文件服务
            .route("/", web::get().to(show_image))  // 显示图片
    })
    .bind(("172.23.73.243", 8080))?
    .run()
    .await
}

