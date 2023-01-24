#include <QApplication>
#include <QList>
#include <QStringList>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QtDebug>

#include "RCliMainWindowDialog.h"
#include "RBackgroundCommandWidget.h"
#include "RFrameCommandWidget.h"
#include "RFF2DCommandWidget.h"
#include "RJSCommandWidget.h"
#include "ROuterSpaceCommandWidget.h"
#include "RConstantDensityPlaneCommandWidget.h"
#include "RPerspectiveCommandWidget.h"
#include "RCameraCommandWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("render-client");
    QApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("render client");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("js-source", "Source folder for js files [default:current folder].");

    QCommandLineOption optionIPPort(QStringList() << "s" << "server", "server ip:port", "ipPort", "127.0.0.1:1999");
    parser.addOption(optionIPPort);

    QCommandLineOption optionRootDirectory(QStringList() << "d" << "directory", "root dir for js files", "rootDirectory", "./");
    parser.addOption(optionRootDirectory);

    parser.process(app);

    qDebug() << parser.value(optionIPPort);
    qDebug() << parser.value(optionRootDirectory);

    // Create command widgets and their names
    QList<RCommandWidget*> wList;
    QStringList sList;
    sList << "Background" << "Frame" << "FF2D" << "OuterSpace" << "JSON" << "ConstantDensityPlane" << "Perspective" << "Camera";
    wList.append(new RBackgroundCommandWidget());
    wList.append(new RFrameCommandWidget());
    wList.append(new RFF2DCommandWidget());
    wList.append(new ROuterSpaceCommandWidget());
    wList.append(new RJSCommandWidget(parser.value(optionRootDirectory)));
    wList.append(new RConstantDensityPlaneCommandWidget());
    wList.append(new RPerspectiveCommandWidget());
    wList.append(new RCameraCommandWidget());

    RCliMainWindowDialog dlg(sList, wList, parser.value(optionIPPort));
    dlg.show();
    return app.exec();
}
