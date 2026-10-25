# IoT Environmental Monitoring System (ESP32-C5)

## 📌 Giới thiệu

Dự án **IoT Environmental Monitoring System** được xây dựng nhằm thu thập, xử lý và truyền dữ liệu quan trắc môi trường sử dụng vi điều khiển **ESP32-C5**.

Hệ thống được thiết kế theo hướng **mở rộng, dễ bảo trì**, áp dụng **lập trình hướng đối tượng (OOP)** và tách riêng từng chức năng thành các thư viện độc lập.

---

## ⚙️ Môi trường phát triển

### 🔹 Arduino IDE (BẮT BUỘC)

Hiện tại dự án **yêu cầu sử dụng Arduino IDE** vì:

- **ESP32-C5 chưa được PlatformIO hỗ trợ chính thức**
- PlatformIO **chưa có board definition** cho `esp32c5`
- Arduino IDE đã được Espressif hỗ trợ **ESP32-C5 đầy đủ và ổn định**

👉 Do đó, **PlatformIO KHÔNG được khuyến nghị** cho dự án này ở thời điểm hiện tại.

---

## 🧩 Phần cứng sử dụng

- Vi điều khiển: **ESP32-C5**
- Các cảm biến môi trường (tuỳ cấu hình dự án):
  - **Temperature (Temp)** – Nhiệt độ
  - **pH** – Độ axit / bazơ
  - **EC** – Độ dẫn điện
  - **DO** – Oxy hoà tan
- Giao tiếp:
  - I2C
  - UART
  - Analog to Digital Converter (ADC)
  - WiFi 5 GHz

---

## 💻 Ngôn ngữ & Kiến trúc phần mềm

### 🔹 Ngôn ngữ lập trình
- **C++**
- Arduino Framework

### 🔹 Lập trình hướng đối tượng (OOP)

Dự án áp dụng **OOP cho từng module**, giúp:
- Dễ mở rộng
- Dễ bảo trì
- Tái sử dụng code
- Tách biệt phần cứng và logic xử lý

Mỗi cảm biến hoặc chức năng được đóng gói trong **class riêng**, tách thành các file `.h / .cpp` độc lập.

## Liên hệ
- Project Manager: Nhan Hoang Vo
- Email: vhnhan@vnkgu.edu.vn
- DEV: Thanh Hau
- Email: thanhhau.itdev@gmail.com
- GitHub: [thanhhau-itdev](https://github.com/thanhhau-itdev)

## Giấy phép
Dự án được phát hành dưới [MIT License](LICENSE).
