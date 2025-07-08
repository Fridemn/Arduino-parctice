#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED显示屏参数
#define SCREEN_WIDTH 128 // OLED显示宽度，以像素为单位
#define SCREEN_HEIGHT 64 // OLED显示高度，以像素为单位

// 声明一个SSD1306显示对象连接到I2C (SDA, SCL引脚)
#define OLED_RESET     -1 // 重置引脚 # (如果共享Arduino重置引脚，则为-1)
#define SCREEN_ADDRESS 0x3C // 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = 生成内部显示电压
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306分配失败"));
    for(;;); // 不继续，无限循环
  }

  // 显示初始的Adafruit启动画面
  display.display();
  delay(2000); // 暂停2秒

  // 清除缓冲区
  display.clearDisplay();

  // 显示自定义文本
  testdrawtext();
  display.display();
  delay(2000);
}

void loop() {
  // 循环显示不同的眼睛表情
  static int eyeState = 0;
  static unsigned long lastChange = 0;
  
  // 每2秒换一种表情
  if (millis() - lastChange > 2000) {
    eyeState = (eyeState + 1) % 5; // 5种不同表情
    lastChange = millis();
  }
  
  display.clearDisplay();
  
  switch(eyeState) {
    case 0:
      drawNormalEyes();
      break;
    case 1:
      drawHappyEyes();
      break;
    case 2:
      drawSleepyEyes();
      break;
    case 3:
      drawSurprisedEyes();
      break;
    case 4:
      drawBlinkEyes();
      break;
  }
  
  display.display();
  delay(100);
}

void testdrawtext(void) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(35, 28);
  display.println(F("Cozmo Eyes"));
  display.display();
}

// 普通眼睛表情
void drawNormalEyes() {
  // 左眼 - 椭圆形效果（用多个圆圈模拟）
  display.fillCircle(32, 32, 15, SSD1306_WHITE);
  display.fillCircle(32, 32, 11, SSD1306_BLACK);
  
  // 右眼 - 椭圆形效果
  display.fillCircle(96, 32, 15, SSD1306_WHITE);
  display.fillCircle(96, 32, 11, SSD1306_BLACK);
}

// 开心眼睛表情
void drawHappyEyes() {
  // 左眼 - 简单的弧形笑眼
  display.drawLine(15, 32, 25, 28, SSD1306_WHITE);
  display.drawLine(25, 28, 35, 28, SSD1306_WHITE);
  display.drawLine(35, 28, 45, 32, SSD1306_WHITE);
  display.drawLine(15, 33, 25, 29, SSD1306_WHITE);
  display.drawLine(25, 29, 35, 29, SSD1306_WHITE);
  display.drawLine(35, 29, 45, 33, SSD1306_WHITE);
  
  // 右眼 - 简单的弧形笑眼
  display.drawLine(83, 32, 93, 28, SSD1306_WHITE);
  display.drawLine(93, 28, 103, 28, SSD1306_WHITE);
  display.drawLine(103, 28, 113, 32, SSD1306_WHITE);
  display.drawLine(83, 33, 93, 29, SSD1306_WHITE);
  display.drawLine(93, 29, 103, 29, SSD1306_WHITE);
  display.drawLine(103, 29, 113, 33, SSD1306_WHITE);
}

// 困倦眼睛表情
void drawSleepyEyes() {
  // 左眼 - 半闭的眼睛（用矩形模拟）
  display.fillRect(15, 30, 34, 6, SSD1306_WHITE);
  display.fillRect(17, 32, 30, 2, SSD1306_BLACK);
  
  // 右眼 - 半闭的眼睛
  display.fillRect(79, 30, 34, 6, SSD1306_WHITE);
  display.fillRect(81, 32, 30, 2, SSD1306_BLACK);
}

// 惊讶眼睛表情
void drawSurprisedEyes() {
  // 左眼 - 大圆眼睛
  display.fillCircle(32, 32, 18, SSD1306_WHITE);
  display.fillCircle(32, 32, 14, SSD1306_BLACK);
  
  // 右眼 - 大圆眼睛
  display.fillCircle(96, 32, 18, SSD1306_WHITE);
  display.fillCircle(96, 32, 14, SSD1306_BLACK);
}

// 眨眼表情
void drawBlinkEyes() {
  // 左眼 - 一条线（闭眼）
  display.drawLine(12, 32, 52, 32, SSD1306_WHITE);
  display.drawLine(12, 33, 52, 33, SSD1306_WHITE);
  
  // 右眼 - 一条线（闭眼）
  display.drawLine(76, 32, 116, 32, SSD1306_WHITE);
  display.drawLine(76, 33, 116, 33, SSD1306_WHITE);
}