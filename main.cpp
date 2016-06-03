#include "mainwindow.h"
#include <QApplication>
#include <locale.h>
//#include "srtParser/srtparser.h"
#include "mediaProcessor/mediafile.h"
#include "mediaProcessor/mediaprocessor.h"
#include "mediaProcessor/peaks.h"
#include "mediaProcessor/ffmpegerror.h"

#include <iostream>
#include <fstream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Needed for libmpv
    setlocale(LC_NUMERIC, "C");

    MainWindow w;
    w.show();

    /*std::ofstream stream("/home/francesco/Desktop/PeakFile");

    try
    {
        MediaFile File("/home/francesco/Desktop/vid.mp4");
        QVector<int> Res;
        Peaks PeakList = ExtractPeaksAndSceneChanges(File, *File.best_stream_of_type(AVMEDIA_TYPE_AUDIO), *File.best_stream_of_type(AVMEDIA_TYPE_VIDEO), Res);
        stream << PeakList.sampleRate() << "\n";
        stream << PeakList.samplesPerPeak() << "\n";
        for(auto p = PeakList.peaks_begin(); p != PeakList.peaks_end(); ++p)
        {
            stream << p->min() << "\n";
            stream << p->max() << "\n";
        }
    }
    catch(FFmpegError &err)
    {
        std::cout << err.what() << std::endl;
    }
    stream.close();
    return 0;*/
    return a.exec();
}
