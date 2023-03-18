#pragma once

#include <string>
#include <fstream>
#include <stddef.h>
#include <mutex>
#include <atomic>
namespace test
{
namespace utility
{

class Logger;

#define debug(format, ...) \
    test::utility::Logger::Instance() -> Log(test::utility::Logger::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define info(format, ...) \
     test::utility::Logger::Instance() -> Log(test::utility::Logger::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define warn(format, ...) \
     test::utility::Logger::Instance() -> Log(test::utility::Logger::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define error(format, ...) \
     test::utility::Logger::Instance() -> Log(test::utility::Logger::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__)

#define fatal(format, ...) \
     test::utility::Logger::Instance() -> Log(test::utility::Logger::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__)


class Logger
{
public:
    enum Level
    {
        DEBUG = 0,
        INFO,
        WARN,
        ERROR,
        FATAL,
        LEVEL_COUNT
    };
private:
    std::string m_FileName;
    std::ofstream m_Fout;
    Level m_Level;
    size_t m_MaxLen;
    size_t m_CurLen;
    static const char* m_Levels[Level::LEVEL_COUNT];
    std::recursive_mutex m_Mutex;
    bool m_Opened;
    bool m_Closed;
    static Logger* m_Instance;

    typedef std::lock_guard<std::recursive_mutex> lock_guard;

private:
    Logger();
    ~Logger();
public:
    static Logger* Instance();

    bool Open(const std::string& fileName);

    void Close();

    void SetLevel(Level level)
    {
        {
            lock_guard lock(m_Mutex);
            m_Level = level;
        }
    }

    Level getLevel()
    {
        lock_guard lock(m_Mutex);
        return m_Level;
    }

    void setMaxLen(size_t maxLen)
    {
        lock_guard lock(m_Mutex);
        m_MaxLen = maxLen;
    }

    template<typename... Args>
    void Log(Level level, const std::string file, int line, const char* format, Args... args);
private:
    std::string GetTime();
    std::string CreateHead(const char *fmt, const std::string& timestap, Level level, const std::string& file, int line);
    void Rotate();
};
    template<typename... Args>
    void Logger::Log(Level level, const std::string file, int line, const char* format, Args... args)
    {
        {
            lock_guard lock(m_Mutex);
            if(m_Level > level){
                return;
            }
            if(m_Fout.fail())
            {
                throw std::logic_error("Open log file failed: " + m_FileName);
            }
        }

        std::string headerStr = CreateHead("%s %s %s:%d ", GetTime(), level, file, line);
        size_t len = snprintf(nullptr, 0, format, std::forward<Args>(args)...);
        if(len > 0){
            char *buffer = reinterpret_cast<char*>(
                ::operator new(len + 1)
            );
            snprintf(buffer, len + 1, format, std::forward<Args>(args)...);
            buffer[len] = 0;
            headerStr += buffer;
            ::operator delete(buffer);
        }

        {
            lock_guard lock(m_Mutex);
            m_CurLen += headerStr.length();
            m_Fout << headerStr;
            m_Fout << "\n";
            m_Fout.flush();
            if(m_MaxLen > 0 && m_CurLen >= m_MaxLen){
                Rotate();
            }
        }

    }

}
}