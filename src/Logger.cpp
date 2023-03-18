#pragma once

#include "Logger.h"
#include <stdexcept>
#include <ctime>


namespace test
{
namespace utility
{
    const char* Logger::m_Levels[Level::LEVEL_COUNT]{
        "DEBUG",
        "INFO",
        "WARN",
        "ERROR",
        "FATAL"
    };
    
    Logger* Logger::m_Instance = nullptr;

    bool Logger::Open(const std::string& fileName)
    {
        lock_guard lock(m_Mutex);
        if(m_Opened == false){
            m_FileName = fileName;
            m_Fout.open(fileName, std::ios::app);
            if(m_Fout.fail()){
                throw std::logic_error("Open log file failed: " + fileName);
                return false;
            }
            m_Fout.seekp(0, std::ios::end);
            m_CurLen = m_Fout.tellp();
            m_Opened = true;
        }
        return true;
    }

    void Logger::Close()
    {
        lock_guard lock(m_Mutex);
        if(m_Closed != true){
            m_Fout.close();
            m_Closed = true;
        }
    }

    // not safe
    Logger* Logger::Instance()
    {
        if(m_Instance == nullptr){
            m_Instance = new Logger();
        }
        return m_Instance;
    }

    Logger::Logger()
    :m_Level(Level::DEBUG), m_MaxLen(0), m_CurLen(0), m_Opened(false), m_Closed(false)
    {}

    Logger::~Logger()
    {
        Close();
    }

    std::string Logger::GetTime()
    {
        time_t ticks = time(nullptr);
        tm* ptm = localtime(&ticks);
        char timestamp[32]{0};
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", ptm);
        return std::string(timestamp);
    }

    std::string Logger::CreateHead(const char *fmt, const std::string& timestap, Level level, const std::string& file, int line)
    {
        std::string str;
        size_t len = snprintf(nullptr, 0, fmt, timestap.c_str(), m_Levels[level], file.c_str(), line);
        if(len > 0){
            char *buffer = reinterpret_cast<char*>(
                ::operator new(len + 1)
            );
            snprintf(buffer, len + 1, fmt, timestap.c_str(), m_Levels[level], file.c_str(), line);
            buffer[len] = 0;
            str.assign(buffer);
            ::operator delete(buffer);
        }
        return str;
    }

    void Logger::Rotate()
    {
        Close();
        std::string filename = m_FileName + " " + GetTime();
        if (rename(m_FileName.c_str(), filename.c_str()) != 0)
        {
            throw std::logic_error("rename log file failed: " + m_FileName + " to " + filename);
        }
        m_Closed = false;
        m_Opened = false;
        m_CurLen = 0;
        Open(m_FileName);
    }
}
}