// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

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
    for (const auto &info : m_manager->progIds()) {
        QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
        item->setText(QString("%1 (%2)").arg(info.extensions.join(", "), info.name));
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(info.associated ? Qt::Checked : Qt::Unchecked);
        item->setData(Qt::UserRole, info.id);
    }

    QString status = QString("Registered: %1 | Associated: %2 format(s)")
                         .arg(m_manager->isAppRegistered() ? "Yes" : "No")
                         .arg(m_manager->associatedCount());
    ui->statusbar->showMessage(status);
}

void MainWindow::onSelectAllClicked() {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        ui->listWidget->item(i)->setCheckState(Qt::Checked);
    }
}

void MainWindow::onClearAllClicked() {
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        ui->listWidget->item(i)->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::onApplyClicked() {
    QList<QString> selectedIds;
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem *item = ui->listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedIds.append(item->data(Qt::UserRole).toString());
        }
    }
    qDebug() << "selected:" << selectedIds;
    m_manager->applyAssociations(selectedIds);
    if (selectedIds.count() > 0) {
        QMessageBox infoBox(this);
        infoBox.setIcon(QMessageBox::Information);
        infoBox.setWindowTitle(tr("Success"));
        infoBox.setStandardButtons(QMessageBox::Discard);
        QPushButton * btn = infoBox.addButton(tr("System Settings"), QMessageBox::ActionRole);
        connect(btn, &QPushButton::clicked, this, [this](){
            on_defaultAppSettingsButton_clicked();
        });
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

    QString helpText = tr(R"(This program helps you register file type assoication information and capabilities to system register.

When you click the "Register Association" button, if you have checked at least one file type association, this program will
register `%1` to your system, with the file type association capabilities you've selected.

Due to Windows 10+ limitation, **if you already associated the selected formats with other program, then we cannot directly modify
existing file type association for you**, so after you done the registration by clicking "Register Association", you can then click
the "System Settings" button to directly open the system control panel for `%1`, so you can manage the file type association there.

When you want to remove the system registration for `%1`, you can simply uncheck all file types on the left-hand side,
then click "Register Association" again. It will remove all related information from the system registery.
)").arg(!m_manager->friendlyAppName().isEmpty() ? m_manager->friendlyAppName() : m_manager->targetApp());

    infoBox.setIcon(QMessageBox::Information);
    infoBox.setWindowTitle(tr("Help"));
    infoBox.setText(helpText);
    infoBox.setTextFormat(Qt::MarkdownText);
    infoBox.exec();
}

