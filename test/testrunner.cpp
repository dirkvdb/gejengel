#include <iostream>
#include <string>
#include <unistd.h>
#include <locale.h>

#include <sys/stat.h>
#include <gtest/gtest.h>

#include "Utils/fileoperations.h"

using namespace std;

int main(int argc, char **argv)
{
    if (!setlocale(LC_CTYPE, ""))
    {
        cerr << "Locale not specified. Check LANG, LC_CTYPE, LC_ALL" << endl;
        return 1;
    }
    
    std::cout << "Running gejengel tests" << std::endl;

    testing::InitGoogleTest(&argc, argv);
    umask(0);
    return RUN_ALL_TESTS();
}
