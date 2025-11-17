//
// Created by jadjer on 16.11.25.
//

#ifndef AVC_AVC_LINE_HPP
#define AVC_AVC_LINE_HPP

#include <cstdint>
#include <pico/types.h>

class AVCLine {
public:
    AVCLine();
    ~AVCLine();

public:
    [[nodiscard]] bool isInputSet() const;
    [[nodiscard]] bool isInputClear() const;

public:
    void outputSet();
    void outputReset();

private:
    static AVCLine* m_instance;
    static void receiveCallback(uint gpio, std::uint32_t events);

private:
    bool m_inputLevel = false;
    std::uint64_t m_inputDelay = 0;
};


#endif //AVC_AVC_LINE_HPP
