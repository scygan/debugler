/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "gtest/gtest.h"

#include <QApplication>
#include <string>

#ifdef _WIN32
#include <windows.h> 
#endif

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
#ifdef _WIN32
    if (argc) {
        char drive[255];
        char folder[255];
        _splitpath_s(argv[0], drive, 255, folder, 255, NULL, 0, NULL, 0);
        std::string newpath = std::string(drive) + folder + "..";
        SetCurrentDirectory(newpath.c_str());
    }
#endif
    return RUN_ALL_TESTS();
}

