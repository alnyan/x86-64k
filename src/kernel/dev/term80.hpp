#pragma once
#include <dev/chardev.hpp>
#include <stdint.h>

namespace devices::term80 {

    using PortType = uint16_t;

    class TextTerminal : public devices::CharDevice {
    private:
        typedef struct {
            char ch;
            uint8_t attrs;
        } term80_char_t;

        int m_x, m_y;
        term80_char_t *m_framebuffer;

        void clear();
        void scroll(int lines);
        void putAt(int x, int y, char ch);
        void ensureScrolled();
        void updateCursor();
    public:
        TextTerminal() : m_x(0), m_y(0), m_framebuffer(reinterpret_cast<term80_char_t*>(0xb8000)) {
            clear();
            updateCursor();
        }

        void putchar(char c) override;
        char getchar() override;
    };

}