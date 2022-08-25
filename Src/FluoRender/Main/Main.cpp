#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QSurfaceFormat>

#include "QvisMainWindow.h"

#include <iostream>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(FluoRender);

    QCoreApplication::setOrganizationName("SCI");
    QCoreApplication::setOrganizationDomain("sci.utah.edu");
    QCoreApplication::setApplicationName(FLUORENDER_TITLE);
    QCoreApplication::setApplicationVersion(FLUORENDER_VERSION_STRING);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":FluoRender/FluoRender/UI/icons/FluoRenderIcon.gif"));

    // Parse any command line arguments.
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();

    // Optional arguments at the end is a file name.
    parser.addPositionalArgument("[file|project]", QCoreApplication::translate("main", "Image files or project to load."));

    QCommandLineOption fullscreenkCLO(QStringList() << "fs" << "fullscreen",
                                       QCoreApplication::translate("main", "Start FluoRender in full screen."));
    parser.addOption(fullscreenkCLO);

    // QCommandLineOption benchmarkCLO(QStringList() << "b" << "benchmark",
    //                                    QCoreApplication::translate("main", "Run FluoRender in the benchmark mode."));
    // parser.addOption(benchmarkCLO);

    // QCommandLineOption imagejCLO(QStringList() << "j" << "imagej",
    //                                    QCoreApplication::translate("main", "Use imagej to load volume files."));
    // parser.addOption(imagejCLO);

    // Movie options
    // QCommandLineOption movieCLO(QStringList() << "m" << "movie",
    //                                    QCoreApplication::translate("main", "Export a movie."),
    //                                    QCoreApplication::translate("main", "filename"), "fluorender");
    // parser.addOption(movieCLO);

    // QCommandLineOption alphaCLO(QStringList() << "a" << "alpha",
    //                                    QCoreApplication::translate("main", "Save the alpha channel of the exported movie."));
    // parser.addOption(alphaCLO);

    // QCommandLineOption floatCLO(QStringList() << "f" << "float",
    //                                    QCoreApplication::translate("main", "Save the float channel of the exported movie."));
    // parser.addOption(floatCLO);

    // QCommandLineOption bitrateCLO(QStringList() << "br" << "bitrate",
    //                                    QCoreApplication::translate("main", "Bit rate (Mbps) of the exported movie."),
    //                                    QCoreApplication::translate("main", "bitrate"), "10");
    // parser.addOption(bitrateCLO);

    // QCommandLineOption lzwCLO(QStringList() << "c" << "compress",
    //                                    QCoreApplication::translate("main", "Compress the exported movie using LZW."));
    // parser.addOption(lzwCLO);

    // Do the actual parsing.
    parser.process(app);

    // bool benchmark = parser.isSet(benchmarkCLO);
    // bool imagej    = parser.isSet(imagejCLO);

    // QString filename  = parser.value(movieCLO);
    // bool alphaChannel = parser.isSet(alphaCLO);
    // bool floatChannel = parser.isSet(floatCLO);
    // double bitrate    = parser.value(bitrateCLO).toDouble();
    // bool compress     = parser.isSet(lzwCLO);

    // Create the main window.
    QvisMainWindow mainWin;

    // Optional argument is a file name.
    if (!parser.positionalArguments().isEmpty())
    {
        const QStringList args = parser.positionalArguments();

        for(const auto & filename : args)
        {
          if(!filename.isEmpty())
                mainWin.loadFile(filename);
        }
    }

    // Raise the window.
    if(parser.isSet(fullscreenkCLO))
        mainWin.showFullScreen();
    else
        mainWin.showNormal();

    return app.exec();
}
