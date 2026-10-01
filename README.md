# Xe dò line dùng ESP32-S3

Xe đọc 8 cảm biến phản xạ analog, ước lượng vị trí line và điều khiển hai động cơ DC qua TB6612FNG. Vòng điều khiển chạy ở 500 Hz. Nút **HỌC LINE** hiệu chuẩn cảm biến; nút **CHẠY/DỪNG** bắt đầu hoặc dừng xe. Bản hiệu chuẩn hợp lệ được lưu trong NVS để dùng lại sau khi tắt nguồn.

**Bản mã được biên dịch** nằm trong [`src/LINE_FOLLOWING_ROBOT`](src/LINE_FOLLOWING_ROBOT). [`platformio.ini`](platformio.ini) đặt `src_dir = src/LINE_FOLLOWING_ROBOT`. Các file nằm trực tiếp dưới `src/` là bản khác và không được PlatformIO biên dịch trong cấu hình hiện tại.

## 1. Phần cứng cần có

| Thành phần | Số lượng | Vai trò |
|---|---:|---|
| Bo ESP32-S3, cấu hình hiện tại là ESP32-S3 DevKitC-1 | 1 | Đọc ADC và điều khiển xe |
| Cảm biến phản xạ có **8 đầu ra analog** | 1 dãy | Phát hiện line từ trái sang phải |
| Driver động cơ TB6612FNG | 1 | Điều khiển hai động cơ DC |
| Động cơ DC, bánh xe và khung xe | 2 động cơ | Di chuyển và rẽ bằng chênh lệch tốc độ hai bánh |
| Nút nhấn HỌC LINE | 1 | Bắt đầu hiệu chuẩn; nút BOOT trên bo có thể dùng làm nút CHẠY/DỪNG |
| Nguồn cho bo và động cơ, dây nối | Theo xe | Cấp nguồn theo thông số thực tế của bo, driver và động cơ |

Nối **GND chung** cho ESP32-S3, dãy cảm biến, driver và nguồn động cơ. Đầu ra analog đưa vào ESP32-S3 phải nằm trong mức điện áp an toàn của bo (**không quá 3,3 V**). Không nối động cơ trực tiếp vào GPIO. Repo không quy định loại cảm biến hay điện áp động cơ cụ thể; chọn nguồn theo phần cứng thực tế.

### Bảng chân GPIO

Các chân được khai báo trong [`pin.h`](src/LINE_FOLLOWING_ROBOT/pin.h). `IR[0]` là mắt **trái nhất** khi nhìn theo hướng xe chạy.

| Chức năng | GPIO ESP32-S3 | Nối tới |
|---|---:|---|
| IR[0], IR[1], IR[2], IR[3] | 1, 2, 3, 4 | Bốn đầu ra analog từ trái vào giữa |
| IR[4], IR[5], IR[6], IR[7] | 5, 6, 7, 8 | Bốn đầu ra analog từ giữa sang phải |
| `PIN_MOTOR_L_PWM` | 15 | PWM kênh driver nối động cơ trái |
| `PIN_MOTOR_L_IN1` / `PIN_MOTOR_L_IN2` | 16 / 17 | Hai chân chọn chiều động cơ trái |
| `PIN_MOTOR_R_PWM` | 18 | PWM kênh driver nối động cơ phải |
| `PIN_MOTOR_R_IN1` / `PIN_MOTOR_R_IN2` | 21 / 47 | Hai chân chọn chiều động cơ phải |
| `PIN_MOTOR_STBY` | 14 | STBY của TB6612FNG |
| `PIN_BUTTON` | 0 | Nút CHẠY/DỪNG: nhấn để nối GPIO0 xuống GND |
| `PIN_LEARN_BUTTON` | 10 | Nút HỌC LINE: nhấn để nối GPIO10 xuống GND |

Hai nút dùng `INPUT_PULLUP`; không cần điện trở kéo lên bên ngoài nếu đấu như bảng. **Thả nút GPIO0 khi cấp nguồn hoặc reset** vì đây là chân BOOT của ESP32-S3. Mã kéo STBY xuống LOW trong lúc khởi tạo motor, sau đó đưa lên HIGH khi PWM sẵn sàng. Khi chạy, dừng hoặc phanh, mã điều khiển bằng PWM và các chân IN; STBY vẫn HIGH.

## 2. Nạp mã và sử dụng xe

Mã dùng Arduino-ESP32 3.x với các chức năng ADC continuous, LEDC và `Preferences`/NVS. Không cần cài thư viện cảm biến hay driver motor riêng.

Với PlatformIO, chạy tại thư mục chứa README này:

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```

Cũng có thể mở [`LINE_FOLLOWING_ROBOT.ino`](src/LINE_FOLLOWING_ROBOT/LINE_FOLLOWING_ROBOT.ino) bằng Arduino IDE, chọn bo ESP32-S3 và Arduino-ESP32 3.x. Serial Monitor dùng **115200 baud**.

1. **Kiểm tra lần đầu:** kê bánh xe khỏi mặt đất, kiểm tra nguồn, GND chung, chiều quay hai bánh và thứ tự 8 mắt IR. Khi khởi động thành công, Serial in `INIT=OK ESP32-S3`.
2. **Học line:** đặt xe để lúc tự xoay, **mỗi cảm biến** lần lượt nhìn thấy cả line và nền. Nhấn nút HỌC LINE ở GPIO10. Xe xoay quét khoảng 3 giây. `CAL=SAVED_NVS` nghĩa là đã lưu hiệu chuẩn; `CAL=INVALID KEEP_PREVIOUS` nghĩa là ít nhất một mắt chưa quét đủ tương phản và bản cũ được giữ. Nhấn CHẠY trong lúc học để hủy lượt học.
3. **Chạy:** đặt dãy cảm biến lên line và nhấn nút CHẠY ở GPIO0. Xe chỉ vào `FOLLOW` khi đã có hiệu chuẩn hợp lệ và đang thấy line. Nếu không, Serial báo `RUN=BLOCKED LEARN_LINE_FIRST` hoặc `RUN=BLOCKED PLACE_ON_LINE`.
4. **Dừng hoặc chạy lại:** khi đang `FOLLOW` hoặc `LOST`, nhấn CHẠY để dừng. Ở `STOP`, có thể nhấn HỌC để hiệu chuẩn lại hoặc nhấn CHẠY để chạy tiếp khi đủ điều kiện.

Khi bật nguồn, `CAL=LOADED_NVS` cho biết đã nạp hiệu chuẩn đã lưu; `CAL=DEFAULT PRESS_LEARN` cho biết cần học line. Nếu mặt sân, độ cao cảm biến hoặc dãy cảm biến thay đổi, nên học lại. `CAL=RAM_ONLY NVS_WRITE_FAILED` nghĩa là lượt học dùng được trong RAM nhưng không lưu được vào flash.

### Cách xe hoạt động

```text
8 cảm biến IR → ADC → chuẩn hóa 0..1000 → tìm vùng/vị trí line
               → bộ điều khiển PD → PWM và chiều quay hai motor
```

- **Bám line:** xe dùng trọng tâm của vùng line được chọn để tăng/giảm lệnh hai bánh. Line càng lệch tâm, tốc độ nền càng giảm.
- **Hai nhánh rời nhau:** `BRANCH_CHOICE` chọn vùng trái, phải hoặc vùng gần vị trí line trước đó. Đây là lựa chọn tại chỗ, **không phải học thuộc lộ trình của map**.
- **Khoảng line đứt:** nếu line vừa ở gần giữa dãy cảm biến, xe đi thẳng chậm tối đa 80 ms để thử vượt khoảng trống. Nếu line mất lâu hơn hoặc mất khi đang lệch nhiều, xe vào `LOST`, quay tìm line tối đa 1,5 giây rồi dừng nếu vẫn không thấy.
- **Vạch ngang và số vòng:** mặc định tính năng nhận mốc đang **tắt** (`LAP_MARKER_PATTERN = Disabled`), nên vùng line rộng không tự làm xe dừng. Có thể cấu hình mốc một hoặc hai vạch và số vòng cần chạy trong `parameters.h`. Vùng rộng, giao lộ và vạch đích có thể cho cùng mẫu cảm biến; mốc đích cần đủ đặc trưng trên sân thực tế.
- **ADC quá hạn:** nếu không có khung ADC mới quá 100 ms, xe phanh và chuyển `STOP`; nút bấm chỉ được nhận lại khi dữ liệu ADC đã cập nhật.

Serial in mỗi 100 ms một dòng như sau:

```text
IR=00011000 state=FOLLOW move=FORWARD L=+0.45 R=+0.45 sg=1
```

`IR` gồm 8 bit trái → phải; `1` là cảm biến được đánh dấu đang trên line. `sg` là số vùng line rời nhau. `L/R` là **lệnh motor chuẩn hóa**, không phải tốc độ bánh đo được. `GAP` chỉ là nhãn hiển thị khi xe tạm mất line trong `FOLLOW`, không phải trạng thái riêng. Mỗi giây có thêm dòng `HEALTH adc=OK cal=OK ovr_1s=0 lap=0`. `ovr_1s` là số nhịp điều khiển bị lỡ trong giây vừa qua.

## 3. Các thông số có thể thay đổi

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

[`robot_setup.h`](src/LINE_FOLLOWING_ROBOT/robot_setup.h) chứa `SENSOR_COUNT = 8`, cực tính `LINE_ADC_HIGH = false` (ADC **thấp** được hiểu là line), cấu hình ADC `40000 Hz` tổng/`10` chuyển đổi mỗi chân/đệm `16`/trung bình `8` mẫu, PWM `20000 Hz` ở `10 bit` và vòng điều khiển `500 Hz`. Chỉ đổi `LINE_ADC_HIGH` sau khi kiểm tra tín hiệu cảm biến trên line và nền. Nếu đổi cực tính hoặc mặt sân, học line lại.

`ADC_RING_SIZE` phải là lũy thừa của 2. `PWM_MAX_DUTY` và `CONTROL_PERIOD_US` được tính từ độ phân giải PWM và tần số điều khiển, nên không cần sửa riêng. `SENSOR_COUNT` và thứ tự `IR_PINS` phải khớp phần cứng; mã nhận diện hiện dùng mặt nạ 8 bit. Các chân GPIO trong [`pin.h`](src/LINE_FOLLOWING_ROBOT/pin.h) chỉ nên đổi khi đấu dây thực tế khác. STBY GPIO14 là chân bật driver lúc khởi tạo, **không phải tham số tăng tốc xe**.

## 4. Thứ tự chỉnh xe trên sân

1. Xác nhận dây, GND chung, chiều quay motor và thứ tự 8 cảm biến; kê bánh khỏi mặt đất khi thử lần đầu.
2. Học line và kiểm tra `IR=xxxxxxxx`: đưa từng mắt lên line/nền để xác nhận đúng cực tính. Nếu bị đảo, sửa `LINE_ADC_HIGH` rồi học lại.
3. Chạy chậm bằng cách giảm `BASE_SPEED`; sau đó chỉnh `PD_KP`, `PD_KD` và `SPEED_DROP`, mỗi lần chỉ đổi một nhóm.
4. Khi bám line liên tục ổn, mới chỉnh thông số vượt khoảng đứt, tìm line và chọn nhánh theo sân thật.
5. Chỉ bật đếm vòng sau khi xác định được mốc đích có mẫu riêng, không trùng giao lộ hoặc ô màu rộng.

Xe chưa có cảm biến tránh vật cản và chưa ghi nhớ lộ trình của map. Việc “học line” ở đây là **hiệu chuẩn min/max phản xạ của 8 cảm biến**, không phải học đường đi.
