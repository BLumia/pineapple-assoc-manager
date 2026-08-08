// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QString>
#include <QList>
#include <QMap>
#include <QObject>

struct ProgIdInfo {
    QString id;             // e.g. "MyApp.jpg"
    QString name;           // Display name
    QStringList extensions; // e.g. "jpg", "jpeg"
    QString icon;           // Icon path
    QString openCommand;    // Per-ProgId open command (optional)
    bool registered;        // Is ProgID key present?
    bool associated;        // Are extensions associated?
};

struct ContextMenuItem {
    QString id;             // config id, e.g. "editWithMyApp"
    QString regKeyName;     // registry verb key name (prefixed), e.g. "MyApp.editWithMyApp"
    QString name;           // Display name (localized)
    QStringList targets;    // ["*"] or ["txt","md"]
    QString command;        // Command (may contain {targetAppFullPath})
    QString icon;           // Icon path (optional)
    bool registered;        // Is this menu item currently in registry?
};

class AssociationManager : public QObject {
    Q_OBJECT
public:
    explicit AssociationManager(QObject *parent = nullptr);

    bool loadConfig(const QString &configPath, const QString &targetAppOverride = QString());
    void checkStatus();
    void applyAssociations(const QList<QString> &selectedProgIds);
    void applyContextMenuItems(const QList<QString> &selectedIds);

    QString friendlyAppName() const { return m_friendlyAppName; }
    QList<ProgIdInfo> progIds() const { return m_progIds; }
    QList<ContextMenuItem> contextMenuItems() const { return m_contextMenuItems; }
    bool isAppRegistered() const { return m_isAppRegistered; }
    QString targetApp(bool withoutSuffix = false) const;
    int associatedCount() const;
    QString getTargetAppFullPath() const;

    void openDefaultAppsSettings() const;

signals:
    void statusChanged();

private:
    QString m_configPath;
    QString m_targetApp;
    QString m_friendlyAppName;
    QString m_openCommand;
    QString m_genericFileIcon;
    QList<ProgIdInfo> m_progIds;
    QList<ContextMenuItem> m_contextMenuItems;
    bool m_isAppRegistered = false;

    QString getAbsoluteFilePath(const QString &relativeFilePathAndName) const;
};
