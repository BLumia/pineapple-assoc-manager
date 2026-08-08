// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFont>

namespace {
    // UserRole values to distinguish item types in the list widget
    const int TypeRole = Qt::UserRole;       // "header", "progid", "contextmenu"
    const int IdRole = Qt::UserRole + 1;     // the item id
}

MainWindow::MainWindow(AssociationManager *manager, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_manager(manager) {
    ui->setupUi(this);
    
    connect(ui->selectAllBtn, &QPushButton::clicked, this, &MainWindow::onSelectAllClicked);
    connect(ui->clearAllBtn, &QPushButton::clicked, this, &MainWindow::onClearAllClicked);
    connect(ui->applyBtn, &QPushButton::clicked, this, &MainWindow::onApplyClicked);

    connect(m_manager, &AssociationManager::statusChanged, this, &MainWindow::updateUI);
    
    updateUI();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateUI() {
    setWindowTitle(tr("File Association Manager for %1").arg(m_manager->targetApp()));

    ui->listWidget->clear();

    // --- File Type Associations section ---
    bool hasProgIds = !m_manager->progIds().isEmpty();
    bool hasContextMenuItems = !m_manager->contextMenuItems().isEmpty();

    if (hasProgIds) {
        QListWidgetItem *header = new QListWidgetItem(tr("File Type Associations"), ui->listWidget);
        QFont headerFont = header->font();
        headerFont.setBold(true);
        header->setFont(headerFont);
        header->setFlags(Qt::ItemIsEnabled);
        header->setData(TypeRole, "header");

        for (const auto &info : m_manager->progIds()) {
            QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
            item->setText(QString("%1 (%2)").arg(info.extensions.join(", "), info.name));
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(info.associated ? Qt::Checked : Qt::Unchecked);
            item->setData(TypeRole, "progid");
            item->setData(IdRole, info.id);
        }
    }

    // --- Context Menu Items section ---
    if (hasContextMenuItems) {
        QListWidgetItem *header = new QListWidgetItem(tr("Context Menu Items"), ui->listWidget);
        QFont headerFont = header->font();
        headerFont.setBold(true);
        header->setFont(headerFont);
        header->setFlags(Qt::ItemIsEnabled);
        header->setData(TypeRole, "header");

        for (const auto &item : m_manager->contextMenuItems()) {
            QListWidgetItem *listItem = new QListWidgetItem(ui->listWidget);
            QString targetDesc = item.targets.contains("*") ? tr("All Files") : item.targets.join(", ");
            listItem->setText(QString("%1 (%2)").arg(item.name, targetDesc));
            listItem->setFlags(listItem->flags() | Qt::ItemIsUserCheckable);
            listItem->setCheckState(item.registered ? Qt::Checked : Qt::Unchecked);
            listItem->setData(TypeRole, "contextmenu");
            listItem->setData(IdRole, item.id);
        }
    }

    QString status = tr("Registered: %1 | Associated: %2 format(s)")
                        .arg(m_manager->isAppRegistered() ? tr("Yes") : tr("No"))
                        .arg(m_manager->associatedCount());
    ui->statusbar->showMessage(status);
}

void MainWindow::onSelectAllClicked() {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        if (item->data(TypeRole).toString() == "header") continue;
        item->setCheckState(Qt::Checked);
    }
}

void MainWindow::onClearAllClicked() {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        if (item->data(TypeRole).toString() == "header") continue;
        item->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::onApplyClicked() {
    QList<QString> selectedProgIds;
    QList<QString> selectedContextMenuIds;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            QString type = item->data(TypeRole).toString();
            QString id = item->data(IdRole).toString();
            if (type == "progid")
                selectedProgIds.append(id);
            else if (type == "contextmenu")
                selectedContextMenuIds.append(id);
        }
    }
    qDebug() << "selected ProgIds:" << selectedProgIds;
    qDebug() << "selected ContextMenu:" << selectedContextMenuIds;
    m_manager->applyAssociations(selectedProgIds);
    m_manager->applyContextMenuItems(selectedContextMenuIds);
    bool hasAnySelection = !selectedProgIds.isEmpty() || !selectedContextMenuIds.isEmpty();
    if (hasAnySelection) {
        QMessageBox infoBox(this);
        infoBox.setIcon(QMessageBox::Information);
        infoBox.setWindowTitle(tr("Success"));
        QPushButton * btn = infoBox.addButton(tr("System Settings"), QMessageBox::ActionRole);
        connect(btn, &QPushButton::clicked, this, [this](){
            on_defaultAppSettingsButton_clicked();
        });
        infoBox.addButton(tr("Skip"), QMessageBox::RejectRole); // we don't really care if it's "Reject" tho, this button simply do nothing...
        infoBox.setText(tr("Associations information updated successfully."));
        infoBox.setInformativeText(tr("You might also want to open System Settings' default apps management page. Open now?"));
        infoBox.exec();
    } else {
        QMessageBox::information(this, tr("Success"), tr("Associations information removed successfully."));
    }
}

void MainWindow::on_defaultAppSettingsButton_clicked()
{
    m_manager->openDefaultAppsSettings();
}


void MainWindow::on_helpButton_clicked()
{
    QMessageBox infoBox(this);

    QStringList helpTexts {
        tr("This program helps you register file type assoication information and capabilities to system register."),
        tr(R"(When you click the "Register Association" button, if you have checked at least one file type association, this program will
register `%1` to your system, with the file type association capabilities you've selected.)"),
        tr(R"(Due to Windows 10+ limitation, **if you already associated the selected formats with other program, then we cannot directly modify
existing file type association for you**, so after you done the registration by clicking "Register Association", you can then click
the "System Settings" button to directly open the system control panel for `%1`, so you can manage the file type association there.)"),
        tr(R"(When you want to remove the system registration for `%1`, you can simply uncheck all file types on the left-hand side,
then click "Register Association" again. It will remove all related information from the system registery.)")
    };

    QString helpText = helpTexts.join("\n\n").arg(!m_manager->friendlyAppName().isEmpty() ? m_manager->friendlyAppName() : m_manager->targetApp());

    infoBox.setIcon(QMessageBox::Information);
    infoBox.setWindowTitle(tr("Help"));
    infoBox.setText(helpText);
    infoBox.setTextFormat(Qt::MarkdownText);
    infoBox.exec();
}

