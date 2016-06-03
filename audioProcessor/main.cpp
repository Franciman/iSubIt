#include "mainwindow.h"
#include <QtWidgets/QApplication>
#include "waveform.h"
#include <iostream>
#include <string>
#include <chrono>

#include "inputfile.h"
#include "ffmpegerror.h"
#include "mediafile.h"

extern "C"
{
#include <libavcodec/avcodec.h>
}

int main(int argc, char *argv[])
{
    int status = 0;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    status = a.exec();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    std::cout << time_elapsed.count() << std::endl;
    return status;
}
