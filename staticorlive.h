
#ifndef STATICORLIVE_H
#define STATICORLIVE_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVector>
#include "mainwindow.h"


class StaticOrLive : public QWidget {
    Q_OBJECT
public:
    // NEW constructor — receives everything from MainWindow
    explicit StaticOrLive(QVector<Process> processes, int mode,
                          int submode, int quantum,
                          QWidget *parent = nullptr);

private slots:
    void onStaticClicked();
    void onLiveClicked();

private:
    // store what was passed in
    QVector<Process> m_processes;
    int m_mode;
    int m_submode;
    int m_quantum;

    QPushButton *staticBtn;
    QPushButton *liveBtn;
};
#endif
