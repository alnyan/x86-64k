#include <dev/term80.hpp>
#include <dev/io.hpp>
#include <algo/memory.hpp>

const int TERM_SCREEN_WIDTH = 80;
const int TERM_SCREEN_HEIGHT = 25;
const int TERM_TAB_WIDTH = 4;

const uint8_t TERM80_FG_RED   = 1 << 0;
const uint8_t TERM80_FG_GREEN = 1 << 1;
const uint8_t TERM80_FG_BLUE  = 1 << 2;
const uint8_t TERM80_FG_BLINK = 1 << 3;
const uint8_t TERM80_BG_RED   = 1 << 4;
const uint8_t TERM80_BG_GREEN = 1 << 5;
const uint8_t TERM80_BG_BLUE  = 1 << 6;
const uint8_t TERM80_BG_BLINK = 1 << 7;

const uint8_t TERM80_FG_WHITE = 0x07;

using namespace devices::term80;
using namespace io;

void TextTerminal::clear() {
    for (int i = 0; i < TERM_SCREEN_WIDTH * TERM_SCREEN_HEIGHT; i++) {
        term80_char_t ch = {0, TERM80_FG_WHITE};
        m_framebuffer[i] = ch;
    }
}

void TextTerminal::scroll(int lines) {
    if (lines < 1) return;
    
    memmove(m_framebuffer, &m_framebuffer[lines * TERM_SCREEN_WIDTH], 
        (TERM_SCREEN_HEIGHT - lines) * TERM_SCREEN_WIDTH * sizeof(term80_char_t));

    for (int i = (TERM_SCREEN_HEIGHT - lines) * TERM_SCREEN_WIDTH; 
             i < TERM_SCREEN_WIDTH * TERM_SCREEN_HEIGHT; i++) {
        term80_char_t ch = {0, TERM80_FG_WHITE };
        m_framebuffer[i] = ch;
    }
}

void TextTerminal::putAt(int x, int y, char ch) {
    m_framebuffer[y * TERM_SCREEN_WIDTH + x].ch = ch;
}

void TextTerminal::ensureScrolled() {
    if (m_x < 0) {
        if (m_y < 0) {
            m_x = 0;
        }
        else {
            m_y--;
            m_x = TERM_SCREEN_WIDTH - 1;
        }
        return;
    }
    
    if (m_x >= TERM_SCREEN_WIDTH) {
        m_y += m_x / TERM_SCREEN_WIDTH;
        m_x = m_x % TERM_SCREEN_WIDTH;
    }	
    if (m_y >= TERM_SCREEN_HEIGHT) { 
        scroll(m_y - TERM_SCREEN_HEIGHT + 1);
        m_y = TERM_SCREEN_HEIGHT - 1;
    }
}

void TextTerminal::updateCursor() {
    uint16_t pos = m_y * TERM_SCREEN_WIDTH + m_x;
 
    out<uint8_t>(0x3d4, 0x0f);
    out<uint8_t>(0x3d5, pos);
    out<uint8_t>(0x3d4, 0x0e);
    out<uint8_t>(0x3d5, (pos >> 8));
}


void TextTerminal::putchar(char ch) {    
    switch (ch) {
        case '\e': /* todo escape processing */
            break;
        case '\b': /* todo remove from here */
            if (m_x == 0 && m_y == 0) return;
            
            m_x--;
            ensureScrolled();
            putAt(m_x, m_y, ' ');
            break;
        case '\n':
            m_x = 0;
            m_y++;
            ensureScrolled();
            break;
        case '\f':
            m_x = 0;
            m_y = 0;
            clear();
            break;
        case '\t':
            m_x = (m_x / TERM_TAB_WIDTH + 1) * TERM_TAB_WIDTH;
            ensureScrolled();
            break;
        default:
            putAt(m_x, m_y, ch);
            m_x++;
            ensureScrolled();
            break;
    }
    updateCursor();
}

char TextTerminal::getchar() { return '\0'; }