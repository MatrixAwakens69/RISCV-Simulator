#include "MemoryManager.h"
#include "Debug.h"

#include <cstdio>

MemoryManager::MemoryManager()
{
    for (uint32_t i = 0; i < 1024; ++i)
    {
        this->memory[i] = nullptr;
    }
}

MemoryManager::~MemoryManager()
{
    for (uint32_t i = 0; i < 1024; ++i)
    {
        if (this->memory[i] != nullptr)
        {
            for (uint32_t j = 0; j < 1024; ++j)
            {
                if (this->memory[i][j] != nullptr)
                {
                    delete[] this->memory[i][j];
                    this->memory[i][j] = nullptr;
                }
            }
            delete[] this->memory[i];
            this->memory[i] = nullptr;
        }
    }
}

bool MemoryManager::addPage(uint32_t addr)
{
    uint32_t i = this->getFirstEntryId(addr);
    uint32_t j = this->getSecondEntryId(addr);
    if (this->memory[i] == nullptr)
    {
        this->memory[i] = new uint8_t *[1024];
        memset(this->memory[i], 0, 1024);
    }
    if (this->memory[i][j] == nullptr)
    {
        this->memory[i][j] = new uint8_t[4096];
        memset(this->memory[i][j], 0, 4096);
    }
    else
    {
        dbgprintf("Addr 0x%x already exists and do not need an addPage()!\n", addr);
        return false;
    }
    return true;
}

bool MemoryManager::isPageExist(uint32_t addr)
{
    return this->isAddrExist(addr);
}

bool MemoryManager::copyFrom(void *src, uint32_t dest, uint32_t len)
{
    for (uint32_t i = 0; i < len; ++i)
    {
        if (!this->isAddrExist(dest + i))
        {
            dbgprintf("Data copy to invalid addr 0x%x!\n", dest + i);
            return false;
        }
        this->setByte(dest + i, ((uint8_t *)src)[i]);
    }
}

bool MemoryManager::setByte(uint32_t addr, uint8_t val)
{
    if (!this->isAddrExist(addr))
    {
        dbgprintf("Byte write to invalid addr 0x%x!\n", addr);
        return false;
    }
    uint32_t i = this->getFirstEntryId(addr);
    uint32_t j = this->getSecondEntryId(addr);
    uint32_t k = this->getPageOffset(addr);
    this->memory[i][j][k] = val;
    return true;
}

uint8_t MemoryManager::getByte(uint32_t addr)
{
    if (!this->isAddrExist(addr))
    {
        dbgprintf("Byte read to invalid addr 0x%x!\n", addr);
        return false;
    }
    uint32_t i = this->getFirstEntryId(addr);
    uint32_t j = this->getSecondEntryId(addr);
    uint32_t k = this->getPageOffset(addr);
    return this->memory[i][j][k];
}

bool MemoryManager::setShort(uint32_t addr, uint16_t val)
{
    if (!this->isAddrExist(addr))
    {
        dbgprintf("Short write to invalid addr 0x%x!\n", addr);
        return false;
    }
    uint32_t i = this->getFirstEntryId(addr);
    uint32_t j = this->getSecondEntryId(addr);
    uint32_t k = this->getPageOffset(addr);
    memcpy((void *)this->memory[i][j][k], &val, 2);
    return true;
}

uint16_t MemoryManager::getShort(uint32_t addr)
{
    uint8_t b1 = this->getByte(addr);
    uint8_t b2 = this->getByte(addr + 1);
    return b1 + (b2 << 8);
}

bool MemoryManager::setInt(uint32_t addr, uint32_t val)
{
    if (!this->isAddrExist(addr))
    {
        dbgprintf("Int write to invalid addr 0x%x!\n", addr);
        return false;
    }
    uint32_t i = this->getFirstEntryId(addr);
    uint32_t j = this->getSecondEntryId(addr);
    uint32_t k = this->getPageOffset(addr);
    memcpy((void *)this->memory[i][j][k], &val, 4);
    return true;
}

uint32_t MemoryManager::getInt(uint32_t addr)
{
    uint8_t b1 = this->getByte(addr);
    uint8_t b2 = this->getByte(addr + 1);
    uint8_t b3 = this->getByte(addr + 2);
    uint8_t b4 = this->getByte(addr + 3);
    return b1 + (b2 << 8) + (b3 << 16) + (b4 << 24);
}

bool MemoryManager::setLong(uint32_t addr, uint64_t val)
{
    if (!this->isAddrExist(addr))
    {
        dbgprintf("Long write to invalid addr 0x%x!\n", addr);
        return false;
    }
    uint32_t i = this->getFirstEntryId(addr);
    uint32_t j = this->getSecondEntryId(addr);
    uint32_t k = this->getPageOffset(addr);
    memcpy((void *)this->memory[i][j][k], &val, 8);
    return true;
}

uint64_t MemoryManager::getLong(uint32_t addr)
{
    uint8_t b1 = this->getByte(addr);
    uint8_t b2 = this->getByte(addr + 1);
    uint8_t b3 = this->getByte(addr + 2);
    uint8_t b4 = this->getByte(addr + 3);
    uint8_t b5 = this->getByte(addr + 4);
    uint8_t b6 = this->getByte(addr + 5);
    uint8_t b7 = this->getByte(addr + 6);
    uint8_t b8 = this->getByte(addr + 7);
    return b1 + (b2 << 8) + (b3 << 16) + (b4 << 24) + (b5 << 32) + (b6 << 40) +
           (b7 << 48) + (b8 << 56);
}

void MemoryManager::printInfo()
{
    printf("Memory Pages: \n");
    for (uint32_t i = 0; i < 1024; ++i)
    {
        if (this->memory[i] == nullptr)
        {
            continue;
        }
        printf("0x%x-0x%x:\n", i << 22, (i + 1) << 22);
        for (uint32_t j = 0; j < 1024; ++j)
        {
            if (this->memory[j] == nullptr)
            {
                continue;
            }
            printf("  0x%x-0x%x\n", (i << 22) + (j << 12),
                   (i << 22) + ((j + 1) << 12));
        }
    }
}

uint32_t MemoryManager::getFirstEntryId(uint32_t addr)
{
    return (addr >> 22) & 0x3FF;
}

uint32_t MemoryManager::getSecondEntryId(uint32_t addr)
{
    return (addr >> 12) & 0x3FF;
}

uint32_t MemoryManager::getPageOffset(uint32_t addr) { return addr & 0xFFF; }

bool MemoryManager::isAddrExist(uint32_t addr)
{
    uint32_t i = this->getFirstEntryId(addr);
    uint32_t j = this->getSecondEntryId(addr);
    if (memory[i] && memory[i][j])
    {
        return true;
    }
    return false;
}
