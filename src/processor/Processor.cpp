//
// Created by gmardon on 13/01/17.
//

#include <zconf.h>
#include "Processor.hh"
#include "../Gametek.hh"
#include "../opcode.hh"

Processor::Processor(Gametek *gametek) {
    m_memory = gametek->getMemory();
    m_cartridge = gametek->getCartridge();
    m_gametek = gametek;
    if (m_gametek->getState() == BOOT)
        m_PC.setValue(0x0);
    else // set this after Boot ROM
        m_PC.setValue(0x100);
    m_SP.setValue(0xFFFE);
    if (m_cartridge->isGBC())
        m_AF.setValue(0x11B0);
    else
        m_AF.setValue(0x01B0);
    m_BC.setValue(0x0013);
    m_DE.setValue(0x00D8);
    m_HL.setValue(0x014D);

    m_operators = (Operator *) malloc(256 * (sizeof(Operator_t)));
    m_operators[0x00] = (Operator) {0x00, "NOP", &Processor::NOP, 1};
    m_operators[0x31] = (Operator) {0x31, "LD_SP NN", &Processor::LD_SP_NN, 1};
    m_operators[0xAF] = (Operator) {0xAF, "CP N", &Processor::CP_N, 1};
    m_operators[0xFF] = (Operator) {0xFF, "RST 38H", &Processor::RST_38H, 1};
}

uint8_t Processor::tick() {
    if (m_gametek->getState() != HALT) {
        if (m_gametek->getState() == BOOT) {
            uint16_t pc_before = m_memory->read(m_PC.getValue());
            executeOPCode(retrieveOPCode());
            uint16_t pc_after = m_memory->read(m_PC.getValue());
            if ((pc_before == 0xFE) && (pc_after == 0x100))
                m_gametek->setState(IN_GAME);
        } else
            executeOPCode(retrieveOPCode());
    } else {
        printf("Halt requested..");
    }
    usleep(100000);

}

uint8_t Processor::retrieveOPCode() {
    uint8_t opcode = m_memory->read(m_PC.getValue());
    m_PC.increment();

    /*if (m_bSkipPCBug)
    {
        m_bSkipPCBug = false;
        PC.Decrement();
    }*/

    return opcode;
}


void Processor::executeOPCode(uint8_t opcode) {
    bool isCB = (opcode == 0xCB);


    /*if (isCB)
    {
       // opcodeTable = m_OPCodesCB;
        //opcode = m_memory->read(m_head++);
    }
    else
        //opcodeTable = m_OPCodes;
        m_operatorsZ
     */
    printf("[PC: %i][Found: 0x%02X] %s\n", m_PC.getValue(), opcode, this->m_operators[opcode].name.c_str());
    auto ptrRef = this->m_operators[opcode].function;
    (this->*ptrRef)();
}

void Processor::updateRealtimeClock() {
    time(&m_RTC);
}

time_t Processor::getRealtimeClock() {
    return m_RTC;
}

void Processor::clearAllFlags() {
    setFlag((uint8_t) FLAG_NONE);
}

void Processor::toggleZeroFlagFromResult(uint8_t result) {
    if (result == 0)
        toggleFlag((uint8_t) FLAG_ZERO);
}

void Processor::setFlag(uint8_t flag) {
    m_AF.setLow(flag);
}

void Processor::flipFlag(uint8_t flag) {
    m_AF.setLow(m_AF.getLow() ^ flag);
}

void Processor::toggleFlag(uint8_t flag) {
    m_AF.setLow(m_AF.getLow() | flag);
}

void Processor::untoggleFlag(uint8_t flag) {
    m_AF.setLow(m_AF.getLow() & (~flag));
}

bool Processor::isSetFlag(uint8_t flag) {
    return (m_AF.getLow() & flag) != 0;
}

void Processor::NOP() {

}

void Processor::LD_SP_NN() {
    m_SP.setLow(m_memory->read(m_PC.getValue()));
    m_PC.increment();
    m_SP.setHigh(m_memory->read(m_PC.getValue()));
    m_PC.increment();
}

void Processor::CP_N()
{
    OP_CP(m_memory->read(m_PC.getValue()));
    m_PC.increment();
}

void Processor::RST_38H()
{
    stackPush(&m_PC);
    m_PC.setValue(0x0038);
}

void Processor::ADD_HL(uint8_t number) {
    int result = m_HL.getValue() + number;
    isSetFlag(FLAG_ZERO) ? setFlag(FLAG_ZERO) : clearAllFlags();
    if (result & 0x10000) {
        toggleFlag(FLAG_CARRY);
    }
    if ((m_HL.getValue() ^ number ^ (result & 0xFFFF)) & 0x1000) {
        toggleFlag(FLAG_HALF);
    }
    m_HL.setValue(static_cast<uint16_t > (result));
}

void Processor::OP_CP(uint8_t number)
{
    setFlag(FLAG_SUB);
    if (m_AF.getHigh() < number)
    {
        toggleFlag(FLAG_CARRY);
    }
    if (m_AF.getHigh() == number)
    {
        toggleFlag(FLAG_ZERO);
    }
    if (((m_AF.getHigh() - number) & 0xF) > (m_AF.getHigh() & 0xF))
    {
        toggleFlag(FLAG_HALF);
    }
}

void Processor::stackPush(SixteenBitRegister* reg)
{
    m_SP.decrement();
    m_memory->write(m_SP.getValue(), reg->getHigh());
    m_SP.decrement();
    m_memory->write(m_SP.getValue(), reg->getLow());
}

void Processor::stackPop(SixteenBitRegister* reg)
{
    reg->setLow(m_memory->read(m_SP.getValue()));
    m_SP.increment();
    reg->setHigh(m_memory->read(m_SP.getValue()));
    m_SP.increment();
}