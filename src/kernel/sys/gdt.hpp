#pragma once

namespace gdt {
    void init();

    void setTaskswitchStack(void *stackPtr);
    void *taskswitchStack();

    // TODO IST1-IST7 installation
}