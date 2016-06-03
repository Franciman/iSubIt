HEADERS += audioProcessor/audio_frame_traits.h \
    audioProcessor/ffmpegerror.h \
    audioProcessor/inputfile.h \
    audioProcessor/mediafile.h \
    audioProcessor/packetiterator.h \
    audioProcessor/peaks.h \
    audioProcessor/sampleextractor.h \
    audioProcessor/sample_format_traits.h

SOURCES += audioProcessor/ffmpegerror.cpp \
    audioProcessor/inputfile.cpp \
    audioProcessor/mediafile.cpp \
    audioProcessor/packetiterator.cpp

PKGCONFIG += libavcodec libavformat libavutil
