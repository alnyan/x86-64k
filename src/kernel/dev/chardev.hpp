#pragma once

namespace devices {

    class CharDevice {
    public:
        virtual void putchar(char ch) = 0;
        virtual char getchar() = 0;
    };
    
}