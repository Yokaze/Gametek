//
// Created by gmardon on 13/01/17.
//

#include <cstring>
#include "Memory.hh"

Memory::Memory(Gametek *gametek) {
    this->m_gametek = gametek;
}

void Memory::init() {
    this->m_cartridge = m_gametek->getCartridge();
    this->m_processor = m_gametek->getProcessor();
    m_RAM = (uint8_t *) malloc(65536 * sizeof(uint8_t));
}

Memory::~Memory() {
    delete m_RAM;
}

void Memory::fillFromCartridge(Cartridge *cartridge) {
    for (int i = 0; i < 65536; i++)
        this->m_RAM[i] = cartridge->getROM()[i];
}

uint8_t Memory::get(uint16_t address) {
    if (address < 65536)
        return (m_RAM[address]);
    else
        return (NULL);
}

uint8_t *Memory::getRAM() {
    return m_RAM;
}

void Memory::printRAM() {
    for (int i = 0; i < 65536; i++) {
        if (i % 10 == 0)
            printf("\n%#-6i\t", i);
        if (m_RAM[i] == NULL)
            printf("     ");
        else
            printf("0x%02X ", m_RAM[i]);
    }
}

uint8_t Memory::read(uint16_t address) {
    switch (address & 0xE000) {
        case 0x0000: {
            if (m_gametek->getState() == BOOT) {
                if (m_cartridge->isGBC()) {
                    if (address < 0x0100)
                        return BootRomCGB[address];
                    if (address < 0x0900 && address > 0x01FF)
                        return BootRomCGB[address - 0x100];
                } else {
                    if (address < 0x0200) // might be 0x0100 but it works...
                        return BootRomDMG[address];
                }
            }
        }
        case 0x2000:
        case 0x4000:
        case 0x6000:
            return this->m_baseHandler->performRead(address);
        case 0x8000:
            return this->m_commonHandler->performRead(address);
        case 0xA000:
            return m_baseHandler->performRead(address);
        case 0xC000:
        case 0xE000: {
            if (address < 0xFF00)
                return m_commonHandler->performRead(address);
            else
                return m_ioHandler->performRead(address);
        }
        default:
            return this->get(address);
    }
}

void Memory::write(uint16_t address, uint8_t value) {
    printf("try to write at : %02X\n value %02X", address, value);
    switch (address & 0xE000) {
        case 0x0000:
        case 0x2000:
        case 0x4000:
        case 0x6000: {
            m_baseHandler->performWrite(address, value);
            break;
        }
        case 0x8000: {
            m_commonHandler->performWrite(address, value);
            break;
        }
        case 0xA000: {
            m_baseHandler->performWrite(address, value);
            break;
        }
        case 0xC000:
        case 0xE000: {
            if (address < 0xFF00)
                m_commonHandler->performWrite(address, value);
            else
                m_ioHandler->performWrite(address, value);
            break;
        }
        default: {
            this->load(address, value);
            break;
        }
    }
}

void Memory::setIOHandler(MemoryHandler *handler) {
    this->m_ioHandler = handler;
}

void Memory::setCommonHandler(MemoryHandler *handler) {
    this->m_commonHandler = handler;
}

void Memory::setBaseHandler(MemoryHandler *handler) {
    this->m_baseHandler = handler;
}

void Memory::load(uint16_t address, uint8_t value) {
    this->m_RAM[address] = value;
}