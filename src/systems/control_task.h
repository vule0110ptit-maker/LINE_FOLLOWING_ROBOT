#pragma once
#include "utils/types.h"

bool control_task_init();
void control_task_run();                          // gọi liên tục trong loop(), tự chờ tick
const RobotContext *control_task_context();