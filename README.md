\# IoT Environmental Monitoring System (ESP32-C5)



\## 📌 Giới thiệu

Dự án \*\*IoT Environmental Monitoring System\*\* được xây dựng nhằm thu thập, xử lý và truyền dữ liệu quan trắc môi trường (nhiệt độ, độ ẩm, khí, áp suất, …) sử dụng vi điều khiển \*\*ESP32-C5\*\*.



Hệ thống được thiết kế theo hướng \*\*mở rộng, dễ bảo trì\*\*, áp dụng \*\*lập trình hướng đối tượng (OOP)\*\* và tách riêng từng chức năng thành các thư viện độc lập.



---



\## ⚙️ Môi trường phát triển



\### 🔹 Arduino IDE (BẮT BUỘC)

Hiện tại dự án \*\*yêu cầu sử dụng Arduino IDE\*\* vì:



\- \*\*ESP32-C5 chưa được PlatformIO hỗ trợ chính thức\*\*

\- PlatformIO \*\*chưa có board definition\*\* cho `esp32c5`

\- Arduino IDE đã được Espressif hỗ trợ \*\*ESP32-C5 đầy đủ và ổn định\*\*



👉 Do đó, \*\*PlatformIO KHÔNG được khuyến nghị\*\* cho dự án này ở thời điểm hiện tại.



---



\## 🧩 Phần cứng sử dụng

\- Vi điều khiển: \*\*ESP32-C5\*\*

\- Các cảm biến môi trường (tuỳ cấu hình dự án):

&nbsp; - Temp <-> Nhiệt độ

&nbsp; - PH <-> Độ axit – bazơ

&nbsp; - EC <-> Độ dẫn điện

&nbsp; - DO <-> Oxy hòa tan

\- Giao tiếp:

&nbsp; - I2C

&nbsp; - UART

&nbsp; - Analog to Digital Converter

&nbsp; - WiFi 5Ghz



---



\## 💻 Ngôn ngữ \& Kiến trúc phần mềm



\### 🔹 Ngôn ngữ lập trình

\- \*\*C++\*\*

\- Chuẩn Arduino framework



\### 🔹 Lập trình hướng đối tượng (OOP)

Dự án áp dụng \*\*OOP cho từng module\*\*, giúp:

\- Dễ mở rộng

\- Dễ bảo trì

\- Tái sử dụng code

\- Tách biệt phần cứng và logic xử lý



Mỗi cảm biến / chức năng được đóng gói trong \*\*class riêng\*\*.



---

