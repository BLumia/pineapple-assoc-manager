#include "association_manager.h"
#include <QSettings>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QLocale>
#include <QDebug>
#include <QTemporaryFile>
#include <shobjidl.h>
#include <shlobj.h>
#include <comdef.h>
#include <windows.h>

AssociationManager::AssociationManager(QObject *parent) : QObject(parent) {}

bool AssociationManager::loadConfig(const QString &configPath) {
    m_configPath = configPath;
    if (!QFileInfo::exists(configPath)) {
        qWarning() << "Config file not found:" << configPath;
        return false;
    }
    QString actualConfigPath = configPath;
    QTemporaryFile tempFile;
    if (configPath.startsWith(":") || configPath.startsWith("qrc:")) {
        QFile resourceFile(configPath);
        if (resourceFile.open(QIODevice::ReadOnly) && tempFile.open()) {
            tempFile.write(resourceFile.readAll());
            tempFile.close();
            actualConfigPath = tempFile.fileName();
        }
    }
    QSettings settings(actualConfigPath, QSettings::IniFormat);
    m_targetApp = settings.value("targetApp").toString();
    m_openCommand = settings.value("openCommand").toString();
    if (m_targetApp.isEmpty()) {
        qWarning() << "targetApp not specified in config";
        return false;
    }
    // Friendly name handling
    QString lang = QLocale::system().name();
    QString langShort = lang.left(2);
    if (settings.contains("friendlyAppName[" + lang + "]"))
        m_friendlyAppName = settings.value("friendlyAppName[" + lang + "]").toString();
    else if (settings.contains("friendlyAppName[" + langShort + "]"))
        m_friendlyAppName = settings.value("friendlyAppName[" + langShort + "]").toString();
    else
        m_friendlyAppName = settings.value("friendlyAppName").toString();
    if (m_friendlyAppName.isEmpty())
        m_friendlyAppName = m_targetApp;
    // Load ProgIDs
    m_progIds.clear();
    if (settings.childGroups().contains("ProgId")) {
        settings.beginGroup("ProgId");
        for (const QString &id : settings.childGroups()) {
            settings.beginGroup(id);
            ProgIdInfo info;
            QString baseName = QFileInfo(m_targetApp).completeBaseName();
            info.id = baseName + "." + id;
            // name localization
            if (settings.contains("name[" + lang + "]"))
                info.name = settings.value("name[" + lang + "]").toString();
            else if (settings.contains("name[" + langShort + "]"))
                info.name = settings.value("name[" + langShort + "]").toString();
            else
                info.name = settings.value("name").toString();
            // extensions handling
            QStringList rawExts = settings.value("extensions").toStringList();
            if (rawExts.isEmpty()) {
                QString str = settings.value("extensions").toString();
                if (!str.isEmpty())
                    rawExts = str.split(",", Qt::SkipEmptyParts);
            }
            info.extensions.clear();
            for (const QString &e : rawExts) {
                QString t = e.trimmed();
                if (!t.isEmpty())
                    info.extensions.append(t);
            }
            info.icon = settings.value("icon").toString();
            info.openCommand = settings.value("openCommand").toString();
            m_progIds.append(info);
            settings.endGroup();
        }
        settings.endGroup();
    }
    return true;
}

void AssociationManager::checkStatus() {
    QSettings classesReg("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);
    QString appRegKey = "Applications/" + m_targetApp;
    classesReg.beginGroup(appRegKey);
    m_isAppRegistered = !classesReg.value("FriendlyAppName").toString().isEmpty();
    classesReg.endGroup();
    qDebug() << "Check App Reg:" << appRegKey << "Registered:" << m_isAppRegistered;
    for (auto &info : m_progIds) {
        classesReg.beginGroup(info.id);
        info.registered = !classesReg.value(".").toString().isEmpty();
        classesReg.endGroup();
        qDebug() << "Check ProgID:" << info.id << "Registered:" << info.registered;
        bool allAssoc = true;
        if (info.extensions.isEmpty()) allAssoc = false;
        for (const QString &ext : info.extensions) {
            QString extKey = "." + ext;
            classesReg.beginGroup(extKey);
            classesReg.beginGroup("OpenWithProgids");
            bool isRegistered = classesReg.contains(info.id);
            classesReg.endGroup();
            classesReg.endGroup();
            qDebug() << "Check Ext:" << ext << "OpenWithProgids contains" << info.id << ":" << isRegistered;
            if (!isRegistered) allAssoc = false;
        }
        info.associated = allAssoc;
    }
    emit statusChanged();
}

void AssociationManager::applyAssociations(const QList<QString> &selectedProgIds) {
    QString appPath = getTargetAppFullPath();
    QString command = m_openCommand;
    
    // Replace placeholder with actual path
    command.replace("{targetAppFullPath}", appPath);
    
    qDebug() << "Applying associations. Selected:" << selectedProgIds;
    qDebug() << "Command:" << command;
    QSettings classesReg("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);
    // Register or unregister the Applications entry
    QString appRegKey = "Applications/" + m_targetApp;
    if (!selectedProgIds.isEmpty()) {
        classesReg.beginGroup(appRegKey);
        classesReg.setValue("FriendlyAppName", m_friendlyAppName);
        classesReg.beginGroup("shell/open/command");
        classesReg.setValue(".", command);
        classesReg.endGroup();
        
        // Register Capabilities (makes app appear in Windows Settings)
        classesReg.beginGroup("Capabilities");
        classesReg.setValue("ApplicationName", m_friendlyAppName);
        classesReg.setValue("ApplicationDescription", m_friendlyAppName);
        
        // Register FileAssociations under Capabilities
        classesReg.beginGroup("FileAssociations");
        for (const auto &info : m_progIds) {
            if (selectedProgIds.contains(info.id)) {
                for (const QString &ext : info.extensions) {
                    QString extKey = "." + ext;
                    classesReg.setValue(extKey, info.id);
                }
            }
        }
        classesReg.endGroup(); // FileAssociations
        classesReg.endGroup(); // Capabilities
        
        classesReg.endGroup(); // appRegKey
        classesReg.sync();
        qDebug() << "Registered App:" << appRegKey << "FriendlyName:" << m_friendlyAppName;
        
        // Register in RegisteredApplications (required for Windows Settings)
        QSettings regApps("HKEY_CURRENT_USER\\Software\\RegisteredApplications", QSettings::NativeFormat);
        QString capabilitiesPath = "Software\\Classes\\" + appRegKey + "\\Capabilities";
        capabilitiesPath.replace('/', '\\');
        regApps.setValue(m_friendlyAppName, capabilitiesPath);
        regApps.sync();
        qDebug() << "Registered in RegisteredApplications:" << m_friendlyAppName;
    } else {
        // Remove Capabilities
        classesReg.remove(appRegKey);
        classesReg.sync();
        qDebug() << "Unregistered App:" << appRegKey;
        
        // Remove from RegisteredApplications
        QSettings regApps("HKEY_CURRENT_USER\\Software\\RegisteredApplications", QSettings::NativeFormat);
        regApps.remove(m_friendlyAppName);
        regApps.sync();
        qDebug() << "Removed from RegisteredApplications:" << m_friendlyAppName;
    }
    // Handle ProgIDs and extensions
    for (const auto &info : m_progIds) {
        bool shouldRegister = selectedProgIds.contains(info.id);
        if (shouldRegister) {
            // Register ProgID
            classesReg.beginGroup(info.id);
            classesReg.setValue(".", info.name);
            if (!info.icon.isEmpty()) {
                QString iconPath = getAbsoluteIconPath(info.icon);
                if (QFile::exists(iconPath)) {
                    classesReg.beginGroup("DefaultIcon");
                    classesReg.setValue(".", iconPath);
                    classesReg.endGroup();
                }
            }
            classesReg.beginGroup("shell/open/command");
            QString progIdCommand = info.openCommand;
            if (progIdCommand.isEmpty()) {
                progIdCommand = m_openCommand;
            }
            progIdCommand.replace("{targetAppFullPath}", appPath);
            classesReg.setValue(".", progIdCommand);
            classesReg.endGroup();
            classesReg.endGroup();
            classesReg.sync();
            qDebug() << "Registered ProgID:" << info.id;
            // Register extensions using OpenWithProgids (Windows 8+ compatible)
            for (const QString &ext : info.extensions) {
                QString extKey = "." + ext;
                classesReg.beginGroup(extKey);
                
                // Register our ProgID in OpenWithProgids
                classesReg.beginGroup("OpenWithProgids");
                classesReg.setValue(info.id, "");  // Empty value, just the key name matters
                classesReg.endGroup();
                
                classesReg.endGroup();
                classesReg.sync();
                qDebug() << "Registered" << info.id << "in OpenWithProgids for" << extKey;
            }
        } else {
            // Unregister ProgID
            classesReg.remove(info.id);
            classesReg.sync();
            qDebug() << "Removed ProgID:" << info.id;
            // Remove from OpenWithProgids for each extension
            for (const QString &ext : info.extensions) {
                QString extKey = "." + ext;
                classesReg.beginGroup(extKey);
                
                // Remove our ProgID from OpenWithProgids
                classesReg.beginGroup("OpenWithProgids");
                if (classesReg.contains(info.id)) {
                    classesReg.remove(info.id);
                    qDebug() << "Removed" << info.id << "from OpenWithProgids for" << extKey;
                }
                classesReg.endGroup();
                
                classesReg.endGroup();
                classesReg.sync();
            }
        }
    }
    // Notify System
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    checkStatus();
}

int AssociationManager::associatedCount() const {
    int count = 0;
    for (const auto &info : m_progIds) {
        if (info.associated) count++;
    }
    return count;
}

QString AssociationManager::getAbsoluteIconPath(const QString &iconName) const {
    QDir dir = QCoreApplication::applicationDirPath();
    return QDir::toNativeSeparators(dir.absoluteFilePath(iconName));
}

QString AssociationManager::getTargetAppFullPath() const {
    if (QFileInfo(m_targetApp).isAbsolute()) {
        return QDir::toNativeSeparators(m_targetApp);
    }
    
    QDir baseDir;
    if (m_configPath.startsWith(":") || m_configPath.startsWith("qrc:")) {
        baseDir = QCoreApplication::applicationDirPath();
    } else {
        baseDir = QFileInfo(m_configPath).absoluteDir();
    }
    return QDir::toNativeSeparators(baseDir.absoluteFilePath(m_targetApp));
}

void AssociationManager::openDefaultAppsSettings() const {
    // Open Windows Settings directly to this app's file association page
    // Using registeredAppUser parameter to target our specific app
    QString appid(m_friendlyAppName);
    // URL encode appid
    appid = appid.toUtf8().toPercentEncoding();
    QString settingsUrl = QString("ms-settings:defaultapps?registeredAppUser=%1").arg(appid);
    ShellExecuteW(nullptr, L"open", (LPCWSTR)settingsUrl.utf16(), nullptr, nullptr, SW_SHOWNORMAL);
}

