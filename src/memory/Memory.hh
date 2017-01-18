//
// Created by gmardon on 13/01/17.
//

#ifndef GAMETEK_MEMORY_H
#define GAMETEK_MEMORY_H
#include <cstdint>
#include "../cartridge/Cartridge.hh"
#include "MemoryHandler.hh"
#include "../processor/Processor.hh"
#include "../Gametek.hh"

class Gametek;
class Cartridge;

class Memory {
public:
    Memory(Gametek *gametek);
    ~Memory();
    void fillFromCartridge(Cartridge *cartridge);
    uint8_t get(uint16_t address);
    uint8_t read(uint16_t address);
    void write(uint16_t address, uint8_t value);
    void setIOHandler(MemoryHandler *handler);
    void setCommonHandler(MemoryHandler *handler);
    void setBaseHandler(MemoryHandler *handler);
    void load(uint16_t address, uint8_t value);
    void init();
    void printRAM();
private:
    Gametek *m_gametek;
    Cartridge *m_cartridge;
    Processor *m_processor;
    uint8_t* m_RAM;
    MemoryHandler *m_ioHandler;
    MemoryHandler *m_commonHandler;
    MemoryHandler *m_baseHandler;
};


#endif //GAMETEK_MEMORY_H
