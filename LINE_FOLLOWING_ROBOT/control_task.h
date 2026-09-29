#pragma once
// Vai trò: nối nút bấm, cảm biến, xử lý line và máy trạng thái vào nhịp điều khiển 500 Hz.
#include "types.h"

bool control_task_init();
void control_task_run();                          // gọi liên tục trong loop(), tự chờ tick
const RobotContext *control_task_context();
