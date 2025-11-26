// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "mainwindow.h"
#include "association_manager.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QMessageBox>
#include <QDir>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

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
    parser.process(a);

    QString configPath = parser.value(configOption);
    if (configPath.isEmpty()) {
        configPath = ":/default-assoc.pacfg";
    }

    AssociationManager manager;
    if (!manager.loadConfig(configPath)) {
        QMessageBox::critical(nullptr, "Error", "Failed to load configuration.");
        return 1;
    }

    // Check if target app exists
    QDir dir = QCoreApplication::applicationDirPath();
    if (!dir.exists(manager.targetApp())) {
        QMessageBox::critical(nullptr, "Error", 
            QString("Target application '%1' not found in: %2")
            .arg(manager.targetApp())
            .arg(dir.absolutePath()));
        return 1;
    }

    manager.checkStatus();

    MainWindow w(&manager);
    w.show();

    return a.exec();
}
