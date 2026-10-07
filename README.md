# LINE FOLLOWING ROBOT — ESP32-S3-DevKitC-1

Xe dò line dùng **8 cảm biến analog**, **ESP32-S3-DevKitC-1** và driver **TB6612FNG**. Xe đọc cảm biến bằng **ADC continuous/DMA**, xử lý vị trí line và điều khiển hai bánh bằng **PD** ở nhịp **500 Hz**.

Cấu hình hiện tại:

- **Cảm biến:** GPIO `4, 5, 6, 7, 8, 3, 9, 10`, cùng hàng **J1**, theo thứ tự từ trái sang phải trên xe.
- **Motor và hai nút:** cùng hàng **J3**.
- **CHẠY/DỪNG:** nút ngoài ở **GPIO47**.
- **HỌC LINE:** nút ngoài ở **GPIO21**.
- **STBY:** giữ HIGH bằng phần cứng, không sử dụng GPIO để điều khiển.

“Học line” trong repo là **hiệu chuẩn min/max của từng cảm biến**. Xe chưa ghi nhớ lộ trình, chưa lập bản đồ và chưa có chức năng tránh vật cản.

## 1. Phần cứng cần có

| Thành phần | Số lượng | Yêu cầu / vai trò |
|---|---:|---|
| ESP32-S3-DevKitC-1 | 1 | Đọc cảm biến và điều khiển xe |
| Dãy cảm biến phản xạ analog | 8 mắt | Có 8 đầu ra analog riêng, đặt thành hàng ngang |
| TB6612FNG | 1 module | Điều khiển hai motor DC |
| Motor DC và bánh xe | 2 bộ | Một bên trái, một bên phải |
| Khung xe và bánh tự do | Theo xe | Giữ cảm biến và hai bánh đúng vị trí |
| Nút nhấn thường hở | 2 | Một nút CHẠY/DỪNG, một nút HỌC LINE |
| Nguồn, bộ ổn áp và dây nối | Theo phần cứng | Cấp nguồn phù hợp cho bo, cảm biến và motor |

Nối **GND chung** giữa ESP32, cảm biến, driver và nguồn motor. Cấp nguồn motor qua **VM của driver**; cấp nguồn logic TB6612FNG qua **VCC 3,3 V**. Đầu ra cảm biến đưa vào ESP32 không được vượt **3,3 V**. Không lấy GPIO làm nguồn cấp motor. Loại motor, cảm biến và điện áp VM cụ thể cần chọn theo linh kiện thực tế.

## 2. Sơ đồ GPIO và cách đấu dây

Chân được khai báo trong [`src/LINE_FOLLOWING_ROBOT/pin.h`](src/LINE_FOLLOWING_ROBOT/pin.h). Hàng J1/J3 và số vị trí dưới đây theo [sơ đồ ESP32-S3-DevKitC-1 của Espressif](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/user_guide_v1.0.html#header-block).

**GPIO và số vị trí trên header là hai số khác nhau.** Đấu dây theo nhãn GPIO in trên bo và đối chiếu sơ đồ đúng chiều.

### 2.1. Tám cảm biến — hàng J1

| Mắt cảm biến trên xe | Tên trong code | GPIO | Hàng / vị trí |
|---|---|---:|---|
| Trái nhất | `IR[0]` | 4 | J1 / 4 |
| Thứ 2 từ trái | `IR[1]` | 5 | J1 / 5 |
| Thứ 3 từ trái | `IR[2]` | 6 | J1 / 6 |
| Thứ 4 từ trái | `IR[3]` | 7 | J1 / 7 |
| Thứ 5 từ trái | `IR[4]` | 8 | J1 / 12 |
| Thứ 6 từ trái | `IR[5]` | 3 | J1 / 13 |
| Thứ 7 từ trái | `IR[6]` | 9 | J1 / 15 |
| Phải nhất | `IR[7]` | 10 | J1 / 16 |

```cpp
constexpr uint8_t IR_PINS[SENSOR_COUNT] = {4, 5, 6, 7, 8, 3, 9, 10};
```

Cả 8 chân đều thuộc **ADC1**. Arduino-ESP32 `analogContinuous()` chỉ hỗ trợ ADC1, theo [tài liệu ADC chính thức](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html#example-applications). Vì vậy, giữ bộ chân này khi dùng cách đọc hiện tại. GPIO15–18 thuộc ADC2; bộ `4, 5, 6, 7, 15, 16, 17, 18` không phù hợp nếu chỉ thay mảng chân mà giữ nguyên module ADC.

`IR[0]` phải nối đúng mắt trái nhất; thứ tự trong mảng không phải thứ tự tăng dần của số GPIO. GPIO3 là chân strapping liên quan lựa chọn JTAG.

### 2.2. Hai motor — hàng J3

Ví dụ nối motor trái vào kênh A, motor phải vào kênh B của TB6612FNG:

| Chức năng | Hằng số trong code | GPIO | Hàng / vị trí | Chân TB6612FNG |
|---|---|---:|---|---|
| PWM motor trái | `PIN_MOTOR_L_PWM` | 1 | J3 / 4 | PWMA |
| Chiều motor trái 1 | `PIN_MOTOR_L_IN1` | 2 | J3 / 5 | AIN1 |
| Chiều motor trái 2 | `PIN_MOTOR_L_IN2` | 42 | J3 / 6 | AIN2 |
| PWM motor phải | `PIN_MOTOR_R_PWM` | 41 | J3 / 7 | PWMB |
| Chiều motor phải 1 | `PIN_MOTOR_R_IN1` | 40 | J3 / 8 | BIN1 |
| Chiều motor phải 2 | `PIN_MOTOR_R_IN2` | 39 | J3 / 9 | BIN2 |

Motor trái nối hai đầu ra kênh A; motor phải nối hai đầu ra kênh B. Kiểm tra trên xe để lệnh dương làm cả hai bánh đẩy xe tiến. Nếu một bánh quay ngược, đổi hai dây của motor đó.

GPIO39–42 đang dùng cho motor nên không dùng JTAG ngoài trên các chân này. UART GPIO43/44 và USB GPIO19/20 không được dùng cho cảm biến, motor hoặc nút trong cấu hình hiện tại.

### 2.3. Hai nút — hàng J3

| Chức năng | Hằng số | GPIO | Hàng / vị trí | Cách đấu |
|---|---|---:|---|---|
| CHẠY/DỪNG | `PIN_BUTTON` | 47 | J3 / 17 | Nút nối GPIO47 xuống GND khi nhấn |
| HỌC LINE | `PIN_LEARN_BUTTON` | 21 | J3 / 18 | Nút nối GPIO21 xuống GND khi nhấn |

Hai nút dùng `INPUT_PULLUP`: thả nút = HIGH, nhấn = LOW. Không cần điện trở kéo lên ngoài. **Nút BOOT GPIO0 trên bo không dùng để chạy/dừng xe trong cấu hình này.**

Trên biến thể ESP32-S3-WROOM-2, GPIO47 làm việc ở mức 1,8 V, theo [datasheet Espressif](https://documentation.espressif.com/esp32-s3-wroom-2_datasheet_en.html). Giữ cách đấu nút xuống GND và kéo lên nội; không kéo GPIO47 lên 3,3 V bên ngoài.

### 2.4. STBY của TB6612FNG

Nối **STBY lên VCC logic 3,3 V** nếu module chưa có kéo lên sẵn. Nếu dùng dây cũ, tháo kết nối STBY với GPIO14. Không nối STBY vào nguồn motor VM.

Code điều khiển chạy, dừng và phanh bằng PWM cùng các chân IN. GPIO14 hiện không được dùng.

## 3. Mã nguồn và môi trường nạp

**Bản đang được PlatformIO chọn nằm trong [`src/LINE_FOLLOWING_ROBOT`](src/LINE_FOLLOWING_ROBOT).** `platformio.ini` đặt:

```ini
[platformio]
src_dir = src/LINE_FOLLOWING_ROBOT
```

Các file nằm trực tiếp dưới `src/` là bản khác; chúng không được biên dịch với cấu hình này. Khi chỉnh thông số xe, sửa các file trong thư mục đang được chọn.

| File / nhóm file | Vai trò |
|---|---|
| `LINE_FOLLOWING_ROBOT.ino` | Khởi động xe, chạy tác vụ điều khiển và in Serial |
| `pin.h` | Danh sách GPIO cảm biến, motor và nút |
| `robot_setup.h` | Số mắt, cực tính cảm biến, ADC, PWM, tần số điều khiển |
| `parameters.h` | Tốc độ, PD, ngưỡng nhận line, hiệu chuẩn, tìm line và mốc vòng |
| `adc_dma.cpp`, `sensor_ir.cpp` | Thu ADC, lọc mẫu, chuẩn hóa và học min/max |
| `calibration_store.cpp` | Đọc/ghi hiệu chuẩn trong NVS bằng `Preferences` |
| `line_process.cpp` | Tạo mặt nạ line, tách vùng và tính vị trí |
| `pid.cpp`, `lowpassfilter.cpp` | Bộ điều khiển PD và bộ lọc |
| `control_task.cpp`, `timer.cpp` | Nhịp 500 Hz, đọc nút, xử lý dữ liệu và điều phối |
| `fsm.cpp`, `states.cpp`, `marker_tracker.cpp` | Trạng thái xe và nhận mốc đếm vòng |
| `motor.cpp` | Xuất PWM, chọn chiều và phanh hai motor |

Mã dùng API **Arduino-ESP32 3.x**: ADC continuous, `ledcAttach()` và timer. `Preferences` có trong bộ Arduino-ESP32; không cần thêm thư viện driver motor hoặc cảm biến riêng. Nếu dùng PlatformIO, môi trường phải cung cấp core tương thích; dòng `platform = espressif32` hiện không khóa phiên bản core.

### Nạp bằng PlatformIO

Chạy tại thư mục gốc của repo:

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```

### Nạp bằng Arduino IDE

Mở [`LINE_FOLLOWING_ROBOT.ino`](src/LINE_FOLLOWING_ROBOT/LINE_FOLLOWING_ROBOT.ino), dùng Arduino-ESP32 3.x, chọn bo ESP32-S3 tương ứng và cổng nạp. Cấu hình USB CDC phù hợp khi dùng cổng USB trực tiếp của ESP32; cấu hình PlatformIO hiện bật `ARDUINO_USB_CDC_ON_BOOT=1`. Serial Monitor dùng **115200 baud**.

## 4. Cách sử dụng xe

### 4.1. Kiểm tra và bật nguồn

1. Kiểm tra nguồn, GND chung, STBY và toàn bộ dây theo bảng GPIO.
2. Kê bánh xe khỏi mặt đất khi kiểm tra chiều quay lần đầu.
3. Bật nguồn và mở Serial Monitor. Khởi động thành công hiển thị `INIT=OK ESP32-S3`.
4. `CAL=LOADED_NVS` nghĩa là có hiệu chuẩn đã lưu; `CAL=DEFAULT PRESS_LEARN` nghĩa là cần học line.

Xe khởi động ở `IDLE`, chờ nút; không tự chạy khi bật nguồn. Nút bị giữ lúc khởi động không được tính là một lần nhấn mới.

### 4.2. Học line — GPIO21

1. Đặt xe trên vùng line/nền sao cho khi xoay, **từng mắt trong cả 8 mắt** đều có thể thấy cả hai bề mặt.
2. Nhấn rồi thả nút HỌC LINE. Xe xoay quét hai phía trong khoảng **3 giây**.
3. Chờ xe dừng và xem kết quả Serial:

| Thông báo | Ý nghĩa |
|---|---|
| `CAL=SAVED_NVS` | Hiệu chuẩn hợp lệ, đã lưu vào flash |
| `CAL=RAM_ONLY NVS_WRITE_FAILED` | Hiệu chuẩn dùng được trong lần bật nguồn này, nhưng chưa lưu thành công |
| `CAL=INVALID KEEP_PREVIOUS` | Ít nhất một mắt thiếu tương phản; giữ hiệu chuẩn trước đó nếu có |
| `CAL=CANCELLED` | Đã hủy lượt học bằng nút CHẠY/DỪNG |

Cả 8 mắt phải có chênh lệch min/max ít nhất `CALIB_MIN_RANGE`, mặc định **300 đơn vị ADC**. Nếu muốn quét bằng tay, đặt `CALIB_SPIN_SPEED = 0`, nạp lại rồi đưa từng mắt qua line và nền trong thời gian học.

**Cần học lại** khi đổi danh sách/thứ tự GPIO cảm biến, thay cảm biến hoặc thay điều kiện mặt sân/độ cao lắp đặt. Bản NVS khác danh sách GPIO sẽ bị từ chối. **Chỉ đổi GPIO của nút hoặc motor không làm bản hiệu chuẩn bị từ chối.**

### 4.3. Chạy và dừng — GPIO47

1. Đặt dãy cảm biến lên line.
2. Nhấn rồi thả nút CHẠY/DỪNG để bắt đầu bám line.
3. Khi xe đang chạy hoặc tìm line, nhấn lại để dừng.
4. Sau khi dừng, có thể nhấn CHẠY để chạy tiếp khi đủ điều kiện hoặc nhấn HỌC để hiệu chuẩn lại.

Xe chỉ bắt đầu chạy khi có **hiệu chuẩn hợp lệ** và **đang thấy line**. Nếu chưa đủ điều kiện, Serial báo `RUN=BLOCKED LEARN_LINE_FIRST` hoặc `RUN=BLOCKED PLACE_ON_LINE`.

Mỗi lần bắt đầu chạy bằng nút, bộ đếm mốc/vòng được đặt lại.

## 5. Cách xe hoạt động

```text
8 đầu ra analog
    → ADC continuous/DMA trên ADC1
    → trung bình mẫu và chuẩn hóa từng mắt về 0..1000
    → nhận line bằng hai ngưỡng bật/tắt
    → tách các vùng line liên tiếp, chọn một vùng
    → tính trọng tâm vị trí -1..+1 và lọc
    → bộ điều khiển PD
    → PWM và chiều quay hai motor
```

ADC thu mẫu ở nền; callback báo có khung mới. Timer đặt nhịp điều khiển **2 ms**, còn xử lý cảm biến, PD và motor diễn ra trong tác vụ điều khiển. Log giám sát được bỏ qua khi bộ đệm Serial thiếu chỗ.

Giá trị chuẩn hóa **1000** biểu thị tín hiệu line mạnh; **0** biểu thị nền. Mặc định `LINE_ADC_HIGH = false`, nghĩa là ADC thô thấp được hiểu là line. Vị trí âm nằm bên trái, dương nằm bên phải.

Khi thấy line, lệnh motor được tính như sau:

```text
base  = BASE_SPEED - SPEED_DROP × |vị trí đã lọc|
steer = PD(vị trí đã lọc)
left  = base + steer
right = base - steer
```

Mỗi lệnh motor được giới hạn trong `[-1, +1]`: dấu chọn chiều, độ lớn chọn duty PWM. Đây là lệnh điều khiển, không phải tốc độ bánh đo bằng encoder.

### Trạng thái xe

| Trạng thái | Hoạt động |
|---|---|
| `IDLE` | Chờ nút sau khi bật nguồn |
| `CALIBRATE` / Serial `LEARN` | Xoay quét và lấy min/max cảm biến |
| `READY` | Dừng, chờ chạy hoặc học lại |
| `FOLLOW` | Bám line bằng PD |
| `LOST` | Xoay tìm lại line |
| `STOP` | Phanh điện mặc định 300 ms, sau đó đưa lệnh motor về 0 |

`GAP` là nhãn Serial khi đang `FOLLOW` nhưng tạm không thấy line, không phải một trạng thái riêng. `ADC_FAULT` là nhãn báo dữ liệu ADC không còn mới.

### Các tình huống trên map

| Tình huống | Cách xử lý hiện tại |
|---|---|
| Line thẳng hoặc cong | Tính vị trí line, điều chỉnh chênh lệnh hai bánh; giảm tốc nền theo độ lệch |
| Hai hoặc nhiều vùng line rời nhau | `Keep` chọn vùng gần vị trí trước đó; bằng nhau thì chọn trái. Có thể cấu hình `Left` / `Right` |
| Line đứt ngắn khi vừa ở gần giữa | Đi thẳng với `GAP_SPEED = 0.20` trong khoảng tối đa 80 ms |
| Mất line khi lệch nhiều hoặc hết thời gian vượt khoảng đứt | Chuyển `LOST`, tìm về phía cuối thấy line trước rồi đổi hướng quét |
| Tìm lại được line | Trở về `FOLLOW`, đặt lại bộ điều khiển/lọc |
| Tìm không được line sau 1,5 giây trong `LOST` | Chuyển `STOP` |
| Vùng line rộng, giao lộ hoặc vạch ngang | Báo `crossing` khi ít nhất 7 mắt thấy line; mặc định chưa bật nhận mốc nên không tự dừng vì vùng rộng |
| Mốc vòng hợp lệ khi đã bật nhận mốc | Tăng số vòng và dừng khi đạt `TARGET_LAPS` |
| Không có ADC mới quá 100 ms | Dừng/phanh theo trạng thái; nút chỉ được xử lý lại khi dữ liệu ADC mới trở lại |

Xe quyết định dựa trên tín hiệu ngay dưới dãy cảm biến. Giao lộ, vạch đích và vùng màu rộng có thể cho mẫu giống nhau; cần thiết kế mốc phù hợp nếu dùng đếm vòng. Bảng mô tả xử lý của code; khả năng vượt cua hoặc khoảng đứt thực tế còn phụ thuộc tốc độ và cơ khí.

### Đọc Serial

Mỗi khoảng 100 ms có bản tin giám sát:

```text
IR=00011000 state=FOLLOW move=FORWARD L=+0.45 R=+0.45 sg=1
```

| Trường | Ý nghĩa |
|---|---|
| `IR` | 8 bit theo thứ tự mắt trái → phải; `1` là mắt được nhận đang trên line |
| `state` | Trạng thái hoặc nhãn giám sát |
| `move` | Suy ra từ lệnh motor: tiến, lùi, rẽ, xoay, phanh hoặc đứng yên |
| `L`, `R` | Lệnh motor trái/phải chuẩn hóa |
| `sg` | Số vùng line rời nhau |

Mỗi giây có thêm bản tin:

```text
HEALTH adc=OK cal=OK ovr_1s=0 lap=0
```

`adc` báo độ mới dữ liệu, `cal` báo hiệu chuẩn hợp lệ, `ovr_1s` là số nhịp điều khiển bị lỡ trong giây vừa qua và `lap` là số vòng đã đếm. Log có thể bị bỏ qua khi Serial đầy.

## 6. Các thông số có thể thay đổi

Thông số chạy xe nằm trong [`parameters.h`](src/LINE_FOLLOWING_ROBOT/parameters.h). Chúng là hằng số lúc biên dịch: **sửa file, biên dịch và nạp lại** mới có tác dụng. Giá trị tốc độ từ 0 đến 1 là mức lệnh PWM tương đối; tốc độ thực còn tùy nguồn, động cơ, bánh và mặt sân.

### Tốc độ, PD và lọc

| Thông số | Mặc định | Tác dụng |
|---|---:|---|
| `BASE_SPEED` | `0.45` | Tốc độ nền khi line ở giữa; giảm để thử xe chậm hơn. |
| `SPEED_DROP` | `0.15` | Mức giảm tốc khi line lệch tâm; tăng để vào cua chậm hơn. |
| `PD_KP` | `0.55` | Độ mạnh rẽ theo độ lệch; quá cao có thể làm xe lắc. |
| `PD_KD` | `0.035` | Phản ứng với tốc độ thay đổi độ lệch; quá cao có thể nhạy nhiễu. |
| `POS_FILTER_HZ` | `60 Hz` | Lọc vị trí line; cao hơn phản ứng nhanh hơn nhưng ít lọc nhiễu hơn. |
| `PD_D_FILTER_HZ` | `40 Hz` | Lọc thành phần D; cao hơn phản ứng nhanh hơn nhưng nhạy nhiễu hơn. |
| `PD_OUT_LIMIT` | `1.0` | Giới hạn độ lớn lệnh rẽ. |

### Nhận line và chọn nhánh

| Thông số | Mặc định | Tác dụng |
|---|---:|---|
| `LINE_ON_THRESHOLD` | `500` | Tín hiệu chuẩn hóa tối thiểu để bắt đầu đánh dấu một mắt thấy line. |
| `LINE_OFF_THRESHOLD` | `350` | Ngưỡng tắt mắt đã thấy line; phải **nhỏ hơn** ngưỡng bật. |
| `SENSOR_NOISE_FLOOR` | `80` | Bỏ tín hiệu rất nhỏ khỏi tổng kiểm tra line. |
| `LINE_FOUND_SUM_MIN` | `250` | Tổng tín hiệu tối thiểu để báo tìm thấy line. |
| `CROSS_MIN_ACTIVE` | `7 mắt` | Số mắt cùng thấy line để coi là vùng rộng; chủ yếu dùng khi bật đếm mốc. |
| `LAST_SIDE_MIN` | `0.3` | Độ lệch cần có để cập nhật phía cuối cùng thấy line khi tìm lại. |
| `BRANCH_CHOICE` | `Keep` | `Keep` chọn vùng gần line trước đó; có thể chọn `Left` hoặc `Right` nếu có các vùng line rời nhau. |

**Lưu ý về hai ngưỡng tổng:** với `LINE_ON_THRESHOLD = 500`, `LINE_OFF_THRESHOLD = 350` và `LINE_FOUND_SUM_MIN = 250` hiện tại, chỉ cần một mắt đã bật thì tín hiệu của mắt đó đủ vượt tổng 250. Vì vậy, đổi nhỏ riêng `SENSOR_NOISE_FLOOR` hoặc `LINE_FOUND_SUM_MIN` thường không làm kết quả nhận line thay đổi. Khi chỉnh độ nhạy, nên kiểm tra hai ngưỡng bật/tắt trước.

### Học line, khoảng đứt và tìm lại line

| Thông số | Mặc định | Tác dụng |
|---|---:|---|
| `CALIB_TIME_MS` | `3000 ms` | Thời gian một lượt học. |
| `CALIB_SWEEP_MS` | `300 ms` | Nhịp đổi hướng xoay khi học; phải lớn hơn 0. |
| `CALIB_SPIN_SPEED` | `0.30` | Tốc độ xoay khi học; đặt 0 nếu muốn tự quét cảm biến bằng tay. |
| `CALIB_MIN_RANGE` | `300 ADC` | Mức chênh min/max tối thiểu của **từng mắt** để lượt học hợp lệ. |
| `LOST_GRACE_MS` | `80 ms` | Thời gian tối đa thử vượt khoảng đứt ngắn. |
| `GAP_SPEED` | `0.20` | Tốc độ đi thẳng trong khoảng đứt ngắn. |
| `GAP_MAX_ENTRY_POS` | `0.35` | Chỉ vượt khoảng đứt nếu line trước đó đủ gần giữa. |
| `LOST_TIMEOUT_MS` | `1500 ms` | Thời gian tìm line tối đa trước khi dừng. |
| `LOST_SPIN_SPEED` | `0.35` | Tốc độ quay tìm line. |
| `LOST_INITIAL_SWEEP_MS` | `500 ms` | Thời gian quét đầu tiên về phía cuối thấy line. |
| `LOST_SWEEP_MS` | `350 ms` | Nhịp đổi hướng quét sau giai đoạn đầu; phải lớn hơn 0. |
| `SENSOR_STALE_TIMEOUT_MS` | `100 ms` | Quá thời gian này không có ADC mới thì dừng xe. |

### Mốc đếm vòng, phanh và nút

| Thông số | Mặc định | Tác dụng |
|---|---:|---|
| `LAP_MARKER_PATTERN` | `Disabled` | `Disabled` tắt đếm vòng; `SingleBar` nhận một vạch; `DoubleBar` nhận hai vạch. |
| `TARGET_LAPS` | `1` | Số vòng hợp lệ cần đạt trước khi dừng; chỉ có tác dụng khi bật nhận mốc. |
| `MIN_LAP_MS` | `3000 ms` | Khoảng thời gian tối thiểu từ lúc xuất phát và giữa các vòng được đếm. |
| `DOUBLE_BAR_MIN_MS` / `DOUBLE_BAR_MAX_MS` | `100 / 800 ms` | Khoảng thời gian hợp lệ giữa hai vạch khi dùng `DoubleBar`. |
| `FINISH_HOLD_MS` | `40 ms` | Vùng rộng phải tồn tại đủ lâu mới được nhận là một vạch mốc. |
| `MARKER_CLEAR_MS` | `60 ms` | Phải rời vùng rộng đủ lâu trước khi nhận vạch tiếp theo. |
| `STOP_BRAKE_MS` | `300 ms` | Thời gian phanh điện trước khi thả motor. |
| `BTN_DEBOUNCE_MS` | `30 ms` | Thời gian lọc dội hai nút bấm. |

### Cấu hình phần cứng ít khi cần đổi

Các hằng số sau nằm trong [`robot_setup.h`](src/LINE_FOLLOWING_ROBOT/robot_setup.h):

| Thông số | Mặc định | Vai trò |
|---|---:|---|
| `SENSOR_COUNT` | `8` | Số mắt cảm biến; thuật toán hiện dùng mặt nạ 8 bit |
| `LINE_ADC_HIGH` | `false` | ADC thô thấp được hiểu là line; `true` khi ADC cao trên line |
| `ADC_SAMPLE_FREQ_HZ` | `40000 Hz` | Tần số chuyển đổi tổng của 8 kênh, tương đương 5000 chuyển đổi/giây/kênh |
| `ADC_CONV_PER_PIN` | `10` | Số chuyển đổi mỗi chân được driver lấy trung bình trong một khung |
| `ADC_RING_SIZE` | `16` | Số mẫu trung bình gần nhất lưu cho mỗi mắt; phải là lũy thừa của 2 |
| `ADC_AVG_SAMPLES` | `8` | Số mẫu trong bộ đệm dùng để lấy trung bình khi đọc |
| `ADC_MAX_GPIO` | `48` | Giới hạn chỉ số bảng tra GPIO, không phải cho phép mọi GPIO đọc ADC |
| `PWM_FREQ_HZ` | `20000 Hz` | Tần số PWM motor |
| `PWM_RES_BITS` | `10 bit` | Độ phân giải PWM |
| `PWM_MAX_DUTY` | `1023` | Tính từ độ phân giải PWM, không cần sửa riêng |
| `CONTROL_FREQ_HZ` | `500 Hz` | Tần số tác vụ điều khiển |
| `CONTROL_PERIOD_US` | `2000 µs` | Tính từ tần số điều khiển, không cần sửa riêng |

Chỉ đổi `LINE_ADC_HIGH` sau khi kiểm tra tín hiệu cảm biến trên line và nền. Nếu đổi cực tính hoặc mặt sân, học line lại. Đổi tần số ADC, PWM hoặc điều khiển cần kiểm tra lại giới hạn API, thời gian xử lý và phản ứng của xe.

`ADC_RING_SIZE` phải là lũy thừa của 2. `PWM_MAX_DUTY` và `CONTROL_PERIOD_US` được tính từ độ phân giải PWM và tần số điều khiển, nên không cần sửa riêng. `SENSOR_COUNT` và thứ tự `IR_PINS` phải khớp phần cứng; mã nhận diện hiện dùng mặt nạ 8 bit. Các chân GPIO trong [`pin.h`](src/LINE_FOLLOWING_ROBOT/pin.h) chỉ nên đổi khi đấu dây thực tế khác. GPIO14 đã được giải phóng vì STBY được giữ HIGH bằng phần cứng.

## 7. Trình tự chỉnh xe trên sân

1. Kiểm tra dây, chiều quay motor và thứ tự mắt IR.
2. Học line; đưa từng mắt qua line/nền để kiểm tra bit `IR` tương ứng. Nếu cực tính bị đảo, sửa `LINE_ADC_HIGH`, nạp lại và học lại.
3. Giảm `BASE_SPEED` để thử chậm. Chỉnh `PD_KP`, `PD_KD` và `SPEED_DROP`, mỗi lần thay ít thông số để quan sát nguyên nhân.
4. Sau khi bám line liên tục ổn, chỉnh thời gian vượt khoảng đứt, tốc độ tìm line và chọn nhánh theo sân thực tế.
5. Bật đếm vòng sau khi đã có mốc phù hợp và kiểm tra được mốc không trùng giao lộ.

Các tùy chọn enum phải viết đúng kiểu trong code, ví dụ:

```cpp
constexpr BranchChoice BRANCH_CHOICE = BranchChoice::Left;
constexpr MarkerPattern LAP_MARKER_PATTERN = MarkerPattern::DoubleBar;
```

## 8. Xử lý lỗi thường gặp

| Hiện tượng / thông báo | Kiểm tra |
|---|---|
| `INIT=FAILED MOTOR_PWM_OR_ADC` | Core Arduino tương thích API, danh sách GPIO ADC1, khởi tạo PWM/ADC; không tiếp tục chạy xe |
| `RUN=BLOCKED LEARN_LINE_FIRST` | Thực hiện lượt học hợp lệ cho cả 8 mắt |
| `RUN=BLOCKED PLACE_ON_LINE` | Đặt cảm biến lên line, kiểm tra cực tính và thứ tự mắt |
| `CAL=INVALID KEEP_PREVIOUS` | Từng mắt phải quét đủ line/nền; kiểm tra chiều cao, đầu ra analog và tương phản |
| `CAL=RAM_ONLY NVS_WRITE_FAILED` | Kiểm tra khả năng ghi NVS; hiệu chuẩn chưa được bảo đảm lưu qua lần tắt nguồn |
| `IR=-------- state=ADC_FAULT move=STILL` hoặc `adc=STALE` | Kiểm tra dữ liệu ADC, khởi tạo và bộ chân cảm biến |
| Nhấn nút không có phản ứng | GPIO47/21 phải nối xuống GND khi nhấn; kiểm tra ADC có đang `STALE` hay không |
| Motor không quay | Nguồn VM, GND chung, STBY HIGH, dây motor và bản tin lệnh `L/R` |
| Xe đánh lái ngược | Kiểm tra thứ tự mắt trái → phải và chiều quay từng motor |
| Xe lắc khi bám line | Giảm tốc, kiểm tra hiệu chuẩn rồi chỉnh PD/lọc |
| `ovr_1s` tăng thường xuyên | Tác vụ điều khiển đang lỡ nhịp; kiểm tra phần xử lý hoặc log được thêm vào |
| PlatformIO báo thiếu `resultcallback` | Lỗi tương thích PlatformIO/Click trong môi trường Python; cần sửa bộ công cụ trước khi build |

README mô tả cấu hình và hành vi của mã nguồn hiện tại. Thử nghiệm trên xe thực tế vẫn cần để xác nhận dây, chiều motor và thông số chạy phù hợp.
