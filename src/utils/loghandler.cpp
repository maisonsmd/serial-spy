#include <QRegularExpression>
#include <QMutex>
#include <QThread>
#include <QDebug>
#include <QMap>
#include <QTime>
#include <stdio.h>
#include <cstring>

#include "loghandler.h"

const char *getFileName(const char *path) {
    if (!path) return "";

    const auto lastSlashUnix = strrchr(path, '/');
    const auto lastSlashWindows = strrchr(path, '\\');

    if (lastSlashUnix)
        return lastSlashUnix + 1;

    if (lastSlashWindows)
        return lastSlashWindows + 1;

    return path;
}

QString trimFunctionName(const QString &longName) {
    // keep class name and function name
    const auto matches = QRegularExpression("(\\w+::.*(?=\\()|(?<= )\\w+.*(?=\\())").match(longName);
    if (!matches.hasMatch())
        return longName;
    return matches.captured(0);
}

void logHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    const QByteArray localMsg = msg.toLocal8Bit();
    const char *file = getFileName(context.file);
    const QString qfunc = trimFunctionName(context.function ? context.function : "");
    const QByteArray bfunc = qfunc.toLocal8Bit();
    auto threadId = (uint64_t)QThread::currentThreadId();
    static int threadCount {0};
    static QMap<uint64_t, int> threadMap {};
    static QMutex mutex {};

    // map long threadIds to shorter ones, the short thread ID is the order of threads that log
    mutex.lock();
    if (threadMap.contains(threadId)) {
        threadId = threadMap[threadId];
    } else {
        threadCount++;
        threadMap.insert(threadId, threadCount);
        threadId = threadCount;
    }
    mutex.unlock();

    const auto time = QTime::currentTime().toString("HH:mm:ss.zzz");
    const auto timeStr = time.toStdString().c_str();

    switch (type) {
    case QtDebugMsg:
        fprintf(stdout, "%s [%llu] Debug: %s (%s, %s:%u)\n", timeStr, threadId, localMsg.constData(), bfunc.constData(), file, context.line);
        fflush(stdout);
        break;
    case QtInfoMsg:
        fprintf(stdout, "%s [%llu] Info: %s (%s, %s:%u)\n", timeStr, threadId, localMsg.constData(), bfunc.constData(), file, context.line);
        fflush(stdout);
        break;
    case QtWarningMsg:
        fprintf(stderr, "%s [%llu] Warning: %s (%s, %s:%u)\n", timeStr, threadId, localMsg.constData(), bfunc.constData(), file, context.line);
        fflush(stderr);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s [%llu] Critical: %s (%s, %s:%u)\n", timeStr, threadId, localMsg.constData(), bfunc.constData(), file, context.line);
        fflush(stderr);
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s [%llu] Fatal: %s (%s, %s:%u)\n", timeStr, threadId, localMsg.constData(), bfunc.constData(), file, context.line);
        fflush(stderr);
        break;
    }
}

void initLog() {
    qInstallMessageHandler(logHandler);
    qDebug("init");
}
