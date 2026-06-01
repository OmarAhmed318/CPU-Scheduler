#ifndef GANTT_H
#define GANTT_H

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QVector>
#include <QTableWidget>
#include <QTimer>
#include <QPushButton>
#include <queue>
#include <vector>
#include <algorithm>
#include "mainwindow.h"

class GanttBar : public QWidget {
    Q_OBJECT
public:
    GanttBar(QWidget *parent = nullptr);
    void setGantt(const QVector<int> &g);
    void appendTick(int pid);        // live mode: add one tick at a time

protected:
    void paintEvent(QPaintEvent *) override;

private:
    QVector<int>       m_gantt;
    QMap<int, QColor>  m_colors;
    QList<QColor>      m_palette;
    int                m_colorIdx = 0;
};

class GanttWindow : public QWidget {
    Q_OBJECT

public:
    explicit GanttWindow(QVector<Process> processes, int mode, int submode,
                         int quantum, bool isLive, QWidget *parent = nullptr);

private slots:
    void onTick();
    void onPauseResume();
    void onAddProcess();

private:
    // ── scheduling ──
    void runFullSchedule();
    void initLiveScheduler();
    void stepOneTick();

    struct Compare {
        int mode;
        Compare(int m) : mode(m) {}
        bool operator()(Process a, Process b) {
            if (mode == 1)
                return a.getRemainingTime() > b.getRemainingTime();
            if (mode == 2) {
                if (a.getpriority() != b.getpriority())
                    return a.getpriority() > b.getpriority();
                else
                    return a.getArrivalTime() > b.getArrivalTime();
            }
            return false;
        }
    };

    // ── data ──
    QVector<Process> m_processes;      // original input
    QVector<Process> m_finished;
    QVector<int>     m_gantt;
    int  m_mode, m_submode, m_quantum;
    bool m_isLive;
    int  m_totalProcessCount;

    // ── live scheduler state ──
    std::vector<Process>  m_liveAll;   // processes not yet arrived
    std::vector<Process>  m_liveFinished;
    int  m_currentTime  = 0;
    bool m_paused       = false;
    bool m_done         = false;

    // for non-preemptive live: track current running process
    bool    m_hasRunning    = false;
    Process m_runningProcess;
    int     m_runningLeft   = 0;       // ticks left for current burst

    // priority_queue can't be stored directly with custom Compare easily,
    // so we use a sorted vector as the ready queue for live mode
    std::vector<Process> m_readyVec;   // kept sorted by Compare
    std::queue<Process>  m_rrQueue;    // for RR live
    std::queue<Process>  m_fcfsQueue;  // for FCFS live
    int m_rrIdx  = 0;                  // index into m_liveAll for RR/FCFS
    int m_rrLeft = 0;                  // remaining quantum ticks in current RR slice

    // ── UI ──
    void buildUI();
    void fillStatsTable();
    void updateRemainingTable();
    void updateAverages();
    void pushToReadyQueue(Process p);  // inserts sorted for SJF/Priority

    GanttBar     *m_ganttBar;
    QScrollArea  *m_scrollArea;
    QTableWidget *m_statsTable;
    QTableWidget *m_remainingTable;    // live remaining-time table
    QLabel       *m_avgLabel;
    QLabel       *m_clockLabel;
    QPushButton  *m_pauseBtn;
    QPushButton  *m_addBtn;
    QTimer       *m_timer;
};

#endif // GANTT_H