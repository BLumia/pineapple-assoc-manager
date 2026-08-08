// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

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

bool AssociationManager::loadConfig(const QString &configPath, const QString &targetAppOverride) {
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
    m_targetApp = targetAppOverride.isEmpty() ? settings.value("targetApp").toString() : targetAppOverride;
    m_openCommand = settings.value("openCommand").toString();
    m_genericFileIcon = settings.value("genericFileIcon", "icons/generic.ico").toString();
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
        m_friendlyAppName = targetApp();
    // Load ProgIDs
    m_progIds.clear();
    if (settings.childGroups().contains("ProgId")) {
        settings.beginGroup("ProgId");
        for (const QString &id : settings.childGroups()) {
            settings.beginGroup(id);
            ProgIdInfo info;
            QString baseName = targetApp(true);
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
            // Fallback: if no extensions specified, use the ProgId name as extension
            if (info.extensions.isEmpty()) {
                info.extensions.append(id);
            }
            info.icon = settings.value("icon", QString("icons/%1.ico").arg(id)).toString();
            info.openCommand = settings.value("openCommand").toString();
            m_progIds.append(info);
            settings.endGroup();
        }
        settings.endGroup();
    }
    // Load ContextMenu items
    m_contextMenuItems.clear();
    if (settings.childGroups().contains("ContextMenu")) {
        settings.beginGroup("ContextMenu");
        for (const QString &id : settings.childGroups()) {
            settings.beginGroup(id);
            ContextMenuItem item;
            item.id = id;
            item.regKeyName = targetApp(true) + "." + id;
            // name localization
            if (settings.contains("name[" + lang + "]"))
                item.name = settings.value("name[" + lang + "]").toString();
            else if (settings.contains("name[" + langShort + "]"))
                item.name = settings.value("name[" + langShort + "]").toString();
            else
                item.name = settings.value("name").toString();
            if (item.name.isEmpty())
                item.name = id;
            // target parsing
            item.targets.clear();
            QStringList rawTargets = settings.value("target").toStringList();
            if (rawTargets.isEmpty()) {
                QString str = settings.value("target", "*").toString();
                rawTargets = str.split(",", Qt::SkipEmptyParts);
            }
            for (const QString &t : rawTargets) {
                QString trimmed = t.trimmed();
                if (!trimmed.isEmpty())
                    item.targets.append(trimmed);
            }
            if (item.targets.isEmpty())
                item.targets.append("*");
            item.command = settings.value("command").toString();
            item.icon = settings.value("icon").toString();
            item.registered = false;
            m_contextMenuItems.append(item);
            settings.endGroup();
        }
        settings.endGroup();
    }
    return true;
}

void AssociationManager::checkStatus() {
    QSettings classesReg("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);
    QString appRegKey = "Applications/" + targetApp();
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
    // Check ContextMenu items status
    for (auto &item : m_contextMenuItems) {
        item.registered = false;
        for (const QString &target : item.targets) {
            QStringList pathParts;
            if (target == "*") {
                pathParts << "*" << "shell" << item.regKeyName;
            } else {
                pathParts << "SystemFileAssociations" << ("." + target) << "shell" << item.regKeyName;
            }
            // Navigate into the verb key and check default value
            for (const QString &part : pathParts)
                classesReg.beginGroup(part);
            bool exists = !classesReg.value(".").toString().isEmpty();
            for (int i = 0; i < pathParts.size(); ++i)
                classesReg.endGroup();
            if (exists) {
                item.registered = true;
                break;
            }
        }
        qDebug() << "Check ContextMenu:" << item.id << "Registered:" << item.registered;
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
    QString appRegKey = "Applications/" + targetApp();
    if (!selectedProgIds.isEmpty()) {
        classesReg.beginGroup(appRegKey);
        classesReg.setValue("FriendlyAppName", m_friendlyAppName);
        classesReg.beginGroup("shell/open/command");
        classesReg.setValue(".", command);
        classesReg.endGroup(); // shell/open/command

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
            // Check and remove existing xxx_auto_file ProgID
            // This handles the case where user previously manually associated the extension
            // with any app via "Open With", result having such auto-file ProgId, which has higher precedence than
            // non auto-file ProgIDs. We want our new ProgID to take effect if user chose it.
            for (const QString &ext : info.extensions) {
                QString autoFileProgId = ext + "_auto_file";
                // Check if the auto-file ProgID exists
                if (classesReg.childGroups().contains(autoFileProgId)) {
                    // User explicitly associated the extension with our app, thus this is safe to remove.
                    qDebug() << "Found and will remove auto-file ProgID:" << autoFileProgId;
                    classesReg.remove(autoFileProgId); // Remove the entire ProgID key
                    classesReg.sync(); // Sync immediately after removal
                }
            }

            // Register the main ProgID
            classesReg.beginGroup(info.id);
            classesReg.setValue(".", info.name);
            if (!info.icon.isEmpty()) {
                QString iconPath = getAbsoluteFilePath(info.icon);
                if (!QFile::exists(iconPath) && !m_genericFileIcon.isEmpty()) {
                    iconPath = getAbsoluteFilePath(m_genericFileIcon);
                }
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
            classesReg.endGroup(); // shell/open/command
            classesReg.endGroup(); // info.id
            classesReg.sync();
            qDebug() << "Registered ProgID:" << info.id;
            // Register extensions using OpenWithProgids (Windows 8+ compatible)
            for (const QString &ext : info.extensions) {
                QString extKey = "." + ext;
                classesReg.beginGroup(extKey);

                // Register our ProgID in OpenWithProgids
                classesReg.beginGroup("OpenWithProgids");
                classesReg.setValue(info.id, "");  // Empty value, just the key name matters
                classesReg.endGroup(); // OpenWithProgids
                classesReg.endGroup(); // extKey
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
                classesReg.endGroup(); // OpenWithProgids

                classesReg.endGroup(); // extKey
                classesReg.sync();
            }
        }
    }
    // Notify System
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    // Refresh status after changes
    checkStatus();
}

void AssociationManager::applyContextMenuItems(const QList<QString> &selectedIds) {
    QString appPath = getTargetAppFullPath();

    qDebug() << "Applying context menu items. Selected:" << selectedIds;
    QSettings classesReg("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);

    for (const auto &item : m_contextMenuItems) {
        bool shouldRegister = selectedIds.contains(item.id);
        for (const QString &target : item.targets) {
            QStringList pathParts;
            if (target == "*") {
                pathParts << "*" << "shell" << item.regKeyName;
            } else {
                pathParts << "SystemFileAssociations" << ("." + target) << "shell" << item.regKeyName;
            }

            if (shouldRegister) {
                // Navigate into the verb key
                for (const QString &part : pathParts)
                    classesReg.beginGroup(part);
                // Set default value (display name)
                classesReg.setValue(".", item.name);
                // Write icon if available
                QString iconPath;
                if (!item.icon.isEmpty()) {
                    iconPath = getAbsoluteFilePath(item.icon);
                }
                if (iconPath.isEmpty() || !QFile::exists(iconPath)) {
                    iconPath = appPath;
                }
                if (QFile::exists(iconPath)) {
                    classesReg.setValue("Icon", iconPath);
                }
                // Write command sub-key
                classesReg.beginGroup("command");
                QString cmd = item.command;
                if (cmd.isEmpty())
                    cmd = m_openCommand;
                cmd.replace("{targetAppFullPath}", appPath);
                classesReg.setValue(".", cmd);
                classesReg.endGroup(); // command
                // Navigate back out
                for (int i = 0; i < pathParts.size(); ++i)
                    classesReg.endGroup();
                classesReg.sync();
                qDebug() << "Registered ContextMenu:" << item.id << "for target:" << target;
            } else {
                // Remove the verb key entirely
                classesReg.remove(pathParts.join("/"));
                classesReg.sync();
                qDebug() << "Removed ContextMenu:" << item.id << "for target:" << target;
            }
        }
    }

    // Notify System
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

    // Refresh status after changes
    checkStatus();
}

QString AssociationManager::targetApp(bool withoutSuffix) const
{
    QFileInfo fi(m_targetApp);
    if (withoutSuffix) {
        return fi.completeBaseName();
    }
    return fi.fileName();
}

int AssociationManager::associatedCount() const {
    int count = 0;
    for (const auto &info : m_progIds) {
        if (info.associated) count++;
    }
    return count;
}

QString AssociationManager::getAbsoluteFilePath(const QString &relativeFilePathAndName) const {
    QFileInfo targetAppFullPath(getTargetAppFullPath());
    QDir targetAppDir(targetAppFullPath.absoluteDir());
    return QDir::toNativeSeparators(targetAppDir.absoluteFilePath(relativeFilePathAndName));
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

