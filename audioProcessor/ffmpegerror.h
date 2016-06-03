#ifndef FFMPEG_ERROR_H
#define FFMPEG_ERROR_H

#include <string>

class FFmpegError
{
public:
    FFmpegError(int error);
    FFmpegError(const std::string &errorMessage) :
        ErrorMessage(errorMessage)
    { }

    const char *what() const noexcept;

private:
    std::string ErrorMessage;
};

#endif // FFMPEG_ERROR_H
