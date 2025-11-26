// SPDX-FileCopyrightText: 2025 Gary Wang <opensource@blumia.net>
//
// SPDX-License-Identifier: MIT

#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include "association_manager.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(AssociationManager *manager, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onApplyClicked();
    void onSelectAllClicked();
    void onClearAllClicked();
    void updateUI();

    void on_defaultAppSettingsButton_clicked();
    void on_helpButton_clicked();

private:
    Ui::MainWindow *ui;
    AssociationManager *m_manager;
    QLabel *m_titleLabel; // We will add this dynamically or use window title
};
