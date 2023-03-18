#include <iostream>
#include "Logger.h"

int main()
{
    test::utility::Logger::Instance() -> Open("./TestLog.txt");
    test::utility::Logger::Instance() -> setMaxLen(512);
    debug("name=%s age=%d", "jack", 18);
    test::utility::Logger::Instance() -> Close();
}