// ===== 腳位設定(告訴 Arduino 每個元件接在哪個腳位) =====
const int lightSensorPin = A0;   // 光敏電阻的 AO(類比輸出)接在 A0 腳位
const int buttonPin = 2;         // 按鈕的訊號腳(S)接在數位 2 號腳
const int redPin = 9;            // RGB燈的紅色(R)接在數位 9 號腳(要用 ~ 符號的PWM腳)
const int greenPin = 10;         // RGB燈的綠色(G)接在數位 10 號腳
const int bluePin = 11;          // RGB燈的藍色(B)接在數位 11 號腳

// ===== 全域變數(這些變數在整個程式執行期間都會記住數值,不會重置) =====
bool isOn = false;               // 記錄目前燈是「開啟」還是「關閉」,一開始設為關閉(false)
int lastStableState = -1;        // 記錄上一次「確定穩定」的按鈕狀態,-1代表還沒有任何紀錄
int lastReading = -1;            // 記錄「上一輪迴圈」讀到的按鈕原始數值,用來偵測有沒有改變
unsigned long lastDebounceTime = 0;  // 記錄「上一次按鈕數值改變」的時間點(用來做防彈跳判斷)
const int debounceDelay = 80;    // 防彈跳的等待時間,單位是毫秒(數值穩定超過80毫秒才算數)

// ===== setup():這個區塊只會在開機/重置時執行一次 =====
void setup() {
  Serial.begin(9600);            // 啟動序列通訊,速度設為9600,才能在電腦上看到 Serial Monitor 的文字
  pinMode(buttonPin, INPUT);     // 設定按鈕腳位為「輸入」模式,因為要讀取按鈕的狀態
  pinMode(redPin, OUTPUT);       // 設定紅色腳位為「輸出」模式,因為要控制LED發光
  pinMode(greenPin, OUTPUT);     // 設定綠色腳位為「輸出」模式
  pinMode(bluePin, OUTPUT);      // 設定藍色腳位為「輸出」模式
}

// ===== loop():這個區塊會不斷重複執行,是程式的主要邏輯 =====
void loop() {
  int reading = digitalRead(buttonPin);  // 讀取按鈕「這一瞬間」的數值(只會是0或1)

  // --- 防彈跳邏輯:避免按鈕接觸不良造成的雜訊被誤判成「按很多次」 ---
  if (reading != lastReading) {          // 如果這次讀到的值,跟上一輪不一樣(代表數值剛剛有變動)
    lastDebounceTime = millis();         // 就記錄「現在的時間」,millis()是從開機到現在經過的毫秒數
  }

  // 只有當數值「變動後,已經維持超過80毫秒沒有再變」,才認定這是真的按下,不是雜訊
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != lastStableState) {    // 如果這個「穩定值」跟上一次記錄的穩定值不一樣
      lastStableState = reading;         // 更新「上一次穩定值」為現在這個值
      if (reading == LOW) {              // 如果穩定值變成 LOW(代表按鈕被按下,依你的模組特性判斷)
        isOn = !isOn;                    // 就把開關狀態反過來(原本開就變關,原本關就變開)
      }
    }
  }
  lastReading = reading;                 // 把這一輪讀到的值,存起來給下一輪比對用

  // --- 讀取光敏電阻,計算LED該多亮 ---
  int lightValue = analogRead(lightSensorPin);  // 讀取光敏電阻的類比數值,範圍是0~1023

  int brightness;                        // 宣告一個變數,準備存放「算出來的LED亮度」
  if (lightValue > 600) {                // 如果光線數值大於600(代表環境很亮)
    brightness = 0;                      // 亮度設為0,LED不發光
  } else {                               // 否則(光線數值小於等於600,代表偏暗)
    brightness = map(lightValue, 100, 600, 255, 0);  
    // 把光線數值(100~600這個範圍)換算成亮度(255~0),數值越暗換算出來的亮度越高
    brightness = constrain(brightness, 0, 255);  
    // 確保算出來的亮度不會超出0~255的合法範圍(避免數值跑到範圍外造成錯誤)
  }

  // --- 根據開關狀態,決定要不要真的輸出亮度給LED ---
  if (isOn) {                            // 如果目前是「開啟」狀態
    analogWrite(redPin, brightness);     // 紅色LED輸出剛剛算出來的亮度
    analogWrite(greenPin, brightness);   // 綠色LED輸出同樣的亮度
    analogWrite(bluePin, brightness);    // 藍色LED輸出同樣的亮度(三色一樣=白光效果)
  } else {                               // 如果目前是「關閉」狀態
    analogWrite(redPin, 0);              // 紅色LED強制不亮
    analogWrite(greenPin, 0);            // 綠色LED強制不亮
    analogWrite(bluePin, 0);             // 藍色LED強制不亮
  }

  // --- 把目前狀態印到電腦螢幕上,方便你邊測試邊觀察數值 ---
  Serial.print("狀態: ");
  Serial.print(isOn ? "開啟中" : "關閉中");  // 如果isOn是true印「開啟中」,否則印「關閉中」
  Serial.print("　光線: ");
  Serial.print(lightValue);              // 印出目前讀到的光線數值
  Serial.print("　亮度: ");
  Serial.println(brightness);            // 印出目前算出來的LED亮度,並換行

  delay(150);                            // 暫停150毫秒,讓迴圈不要跑太快、畫面看得清楚
}