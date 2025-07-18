#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ESP32 C3推荐使用内置LED引脚或GPIO2
#define LED_PIN 2

Adafruit_MPU6050 mpu;

void setup(void) {
    Serial.begin(115200);
    
    // ESP32 C3的I2C引脚配置
    Wire.begin(4, 5); // SDA=GPIO4, SCL=GPIO5
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // 等待串口连接（用于调试）
    delay(2000);
    Serial.println("=== ESP32 C3 晃动检测程序启动 ===");
    
    // 尝试初始化MPU6050
    Serial.println("正在初始化MPU6050...");
    if (!mpu.begin()) {
        Serial.println("❌ 错误：无法找到MPU6050传感器！");
        Serial.println("请检查：");
        Serial.println("1. I2C连线：SDA->GPIO4, SCL->GPIO5");
        Serial.println("2. 电源连接：VCC->3.3V, GND->GND");
        Serial.println("3. MPU6050是否损坏");
        
        // LED快速闪烁表示错误
        while (1) { 
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
            delay(200);
        }
    }
    Serial.println("✅ MPU6050初始化成功！");

    // 配置传感器参数
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    Serial.print("加速计范围设置为: ");
    switch (mpu.getAccelerometerRange()) {
        case MPU6050_RANGE_2_G: Serial.println("±2G"); break;
        case MPU6050_RANGE_4_G: Serial.println("±4G"); break;
        case MPU6050_RANGE_8_G: Serial.println("±8G"); break;
        case MPU6050_RANGE_16_G: Serial.println("±16G"); break;
    }

    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    Serial.print("陀螺仪量程设置为: ");
    switch (mpu.getGyroRange()) {
        case MPU6050_RANGE_250_DEG: Serial.println("±250 deg/s"); break;
        case MPU6050_RANGE_500_DEG: Serial.println("±500 deg/s"); break;
        case MPU6050_RANGE_1000_DEG: Serial.println("±1000 deg/s"); break;
        case MPU6050_RANGE_2000_DEG: Serial.println("±2000 deg/s"); break;
    }

    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.print("过滤器带宽设置为: ");
    switch (mpu.getFilterBandwidth()) {
        case MPU6050_BAND_260_HZ: Serial.println("260 Hz"); break;
        case MPU6050_BAND_184_HZ: Serial.println("184 Hz"); break;
        case MPU6050_BAND_94_HZ: Serial.println("94 Hz"); break;
        case MPU6050_BAND_44_HZ: Serial.println("44 Hz"); break;
        case MPU6050_BAND_21_HZ: Serial.println("21 Hz"); break;
        case MPU6050_BAND_10_HZ: Serial.println("10 Hz"); break;
        case MPU6050_BAND_5_HZ: Serial.println("5 Hz"); break;
    }

    Serial.println("=== 开始晃动检测 ===");
    Serial.println("尝试晃动设备来测试LED是否点亮...");
    delay(1000);
}

void loop() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // 计算加速度模长
    float acc_norm = sqrt(
        a.acceleration.x * a.acceleration.x +
        a.acceleration.y * a.acceleration.y +
        a.acceleration.z * a.acceleration.z
    );
    
    // 静态变量保存历史数据
    static float last_acc_norm = acc_norm;
    static float baseline = acc_norm;
    static int stable_count = 0;
    
    // 计算变化量
    float delta = fabs(acc_norm - last_acc_norm);
    float deviation = fabs(acc_norm - baseline);
    
    // 更新基线（缓慢跟踪静止状态）
    if (delta < 0.5) {
        stable_count++;
        if (stable_count > 20) {
            baseline = baseline * 0.95 + acc_norm * 0.05;
        }
    } else {
        stable_count = 0;
    }
    
    last_acc_norm = acc_norm;

    // 改进的晃动检测：使用多重条件
    float threshold_delta = 1.5;      // 瞬时变化阈值
    float threshold_deviation = 2.0;  // 偏离基线阈值
    
    bool is_shaking = (delta > threshold_delta) || (deviation > threshold_deviation);
    
    // 控制LED
    if (is_shaking) {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("🔥 检测到晃动！LED点亮");
    } else {
        digitalWrite(LED_PIN, LOW);
    }

    // 详细的串口输出（每秒输出一次）
    static unsigned long last_print = 0;
    if (millis() - last_print > 500) {
        Serial.print("加速度 [X:");
        Serial.print(a.acceleration.x, 2);
        Serial.print(" Y:");
        Serial.print(a.acceleration.y, 2);
        Serial.print(" Z:");
        Serial.print(a.acceleration.z, 2);
        Serial.print("] 模长:");
        Serial.print(acc_norm, 2);
        Serial.print(" 基线:");
        Serial.print(baseline, 2);
        Serial.print(" Delta:");
        Serial.print(delta, 2);
        Serial.print(" 偏离:");
        Serial.print(deviation, 2);
        
        if (is_shaking) {
            Serial.println(" [晃动中]");
        } else {
            Serial.println(" [静止]");
        }
        
        last_print = millis();
    }
    
    delay(50); // 减少延迟提高响应速度
}