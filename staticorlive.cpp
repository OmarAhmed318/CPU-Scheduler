#include "staticorlive.h"
#include "gantt.h"


StaticOrLive::StaticOrLive(QVector<Process> processes, int mode,
                           int submode, int quantum, QWidget *parent)
    : QWidget(parent),
    m_processes(processes),
    m_mode(mode),
    m_submode(submode),
    m_quantum(quantum)
{
    setWindowTitle("Run Scheduler");
    setMinimumSize(400, 220);

    QLabel *title = new QLabel("How do you want to run the scheduler?");
    title->setAlignment(Qt::AlignCenter);
    QFont f; f.setPointSize(13); f.setBold(true);
    title->setFont(f);

    staticBtn = new QPushButton("Static  (run all at once)");
    liveBtn   = new QPushButton("Live  (1 tick = 1 second)");
    staticBtn->setMinimumHeight(50);
    liveBtn->setMinimumHeight(50);

    connect(staticBtn, &QPushButton::clicked, this, &StaticOrLive::onStaticClicked);
    connect(liveBtn,   &QPushButton::clicked, this, &StaticOrLive::onLiveClicked);

    QHBoxLayout *btnRow = new QHBoxLayout;
    btnRow->addWidget(staticBtn);
    btnRow->addWidget(liveBtn);
    btnRow->setSpacing(20);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->addWidget(title);
    root->addSpacing(20);
    root->addLayout(btnRow);
    root->setContentsMargins(40, 40, 40, 40);
}

void StaticOrLive::onStaticClicked()
{
    GanttWindow *gantt = new GanttWindow(
        m_processes, m_mode, m_submode, m_quantum,
        false   // false = static
        );
    gantt->show();
    this->close();
}

void StaticOrLive::onLiveClicked()
{
    GanttWindow *gantt = new GanttWindow(
        m_processes, m_mode, m_submode, m_quantum,
        true    // true = live
        );
    gantt->show();
    this->close();
}

