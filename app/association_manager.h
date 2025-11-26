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

class AssociationManager : public QObject {
    Q_OBJECT
public:
    explicit AssociationManager(QObject *parent = nullptr);

    bool loadConfig(const QString &configPath);
    void checkStatus();
    void applyAssociations(const QList<QString> &selectedProgIds);

    QString targetApp() const { return m_targetApp; }
    QString friendlyAppName() const { return m_friendlyAppName; }
    QList<ProgIdInfo> progIds() const { return m_progIds; }
    bool isAppRegistered() const { return m_isAppRegistered; }
    int associatedCount() const;

    void openDefaultAppsSettings() const;

signals:
    void statusChanged();

private:
    QString m_configPath;
    QString m_targetApp;
    QString m_friendlyAppName;
    QString m_openCommand;
    QList<ProgIdInfo> m_progIds;
    bool m_isAppRegistered = false;

    QString getAbsoluteIconPath(const QString &iconName) const;
    QString getTargetAppFullPath() const;
};
