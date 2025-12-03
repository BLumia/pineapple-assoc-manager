// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "mainwindow.h"
#include "association_manager.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QTranslator>
#include <QDir>
#include <QFileDialog>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QTranslator translator;
    if (translator.load(QLocale(), "pineapple-assoc-manager", "_", ":/i18n/")) {
        a.installTranslator(&translator);
    }

    QApplication::setApplicationName("Pineapple Assoc Manager");
    QApplication::setApplicationDisplayName(
        QCoreApplication::translate("main", "Pineapple Assoc Manager"));

    QCommandLineParser parser;
    parser.setApplicationDescription("File Association Manager");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption configOption(QStringList() << "c" << "config",
            QCoreApplication::translate("main", "Path to configuration file."),
            QCoreApplication::translate("main", "file"));
    parser.addOption(configOption);
    QCommandLineOption demoOption(QStringList() << "d" << "demo",
                                  QCoreApplication::translate("main", "Path to .pademo/.patest file."),
                                  QCoreApplication::translate("main", "file"));
    parser.addOption(demoOption);
    // an option to enable manual select exectuable as fallback, default off
    QCommandLineOption manualFallbackOption(QStringList() << "m" << "manual-fallback",
                                            QCoreApplication::translate("main", "Manual select exectuable when target app not found."));
    parser.addOption(manualFallbackOption);
    parser.process(a);

    if (parser.isSet(demoOption)) {
        QCommandLineParser::showMessageAndExit(QCommandLineParser::MessageType::Information,
                                               QString("Demo: The passed file name is: %1").arg(parser.value(demoOption)));
    }

    QString configPath = parser.value(configOption);
    if (configPath.isEmpty()) {
        // Check if program location contains default-assoc.pacfg first, if so, use that.
        // Otherwise use default-assoc.pacfg from qrc
        configPath = QCoreApplication::applicationDirPath() + QDir::separator() + "default-assoc.pacfg";
        if (!QFile::exists(configPath)) {
            qDebug() << "External default config not found at" << configPath;
            configPath = ":/default-assoc.pacfg";
        }
    }

    AssociationManager manager;
    if (!manager.loadConfig(configPath)) {
        QMessageBox::critical(nullptr, "Error", "Failed to load configuration.");
        return 1;
    }

    QString targetAppFullPath(manager.getTargetAppFullPath());

    // allow user to locate target app if it doesn't exist
    while (!QFile::exists(targetAppFullPath) && parser.isSet(manualFallbackOption)) {
        QMessageBox::StandardButton reply = QMessageBox::question(nullptr, 
            "Target Application Not Found",
            QString("Target application '%1' not found at:\n%2\n\nDo you want to locate it manually?")
            .arg(manager.targetApp())
            .arg(targetAppFullPath),
            QMessageBox::Yes | QMessageBox::No);
            
        if (reply == QMessageBox::Yes) {
            QString fileName = QFileDialog::getOpenFileName(nullptr, 
                "Locate Target Application",
                QFileInfo(targetAppFullPath).absolutePath(),
                QString("Target App (%1);;Executables (*.exe)").arg(manager.targetApp()));
                
            if (!fileName.isEmpty()) {
                // Try to reload config with the new target app
                if (manager.loadConfig(configPath, fileName)) {
                    targetAppFullPath = manager.getTargetAppFullPath();
                }
            }
        } else {
            break;
        }
    }

    // Check if target app exists
    if (!QFile::exists(targetAppFullPath)) {
        QMessageBox::critical(nullptr, "Error", 
            QString("Target application '%1' not found at:\n%2")
            .arg(manager.targetApp())
            .arg(targetAppFullPath));
        return 1;
    }

    manager.checkStatus();

    MainWindow w(&manager);
    w.show();

    return a.exec();
}
