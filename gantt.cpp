#include "gantt.h"
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QFrame>
#include <QFont>
#include <QColor>
#include <QMap>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QScrollBar>

// ═══════════════════════════════════════════════════════════════════════════
//  GanttBar
// ═══════════════════════════════════════════════════════════════════════════
GanttBar::GanttBar(QWidget *parent) : QWidget(parent)
{
    m_palette = {"#4A90D9","#E67E22","#2ECC71","#9B59B6",
                 "#E74C3C","#1ABC9C","#F39C12","#3498DB"};
    m_colors[0] = QColor("#cccccc");
}

void GanttBar::setGantt(const QVector<int> &g)
{
    m_gantt = g;
    // pre-assign colours
    for (int pid : g)
        if (pid != 0 && !m_colors.contains(pid))
            m_colors[pid] = m_palette[m_colorIdx++ % m_palette.size()];

    int w = qMax(m_gantt.size() * 60 + 60, 600);
    setMinimumSize(w, 90);
    update();
}

void GanttBar::appendTick(int pid)
{
    if (pid != 0 && !m_colors.contains(pid))
        m_colors[pid] = m_palette[m_colorIdx++ % m_palette.size()];
    m_gantt.append(pid);
    int w = qMax(m_gantt.size() * 60 + 60, 600);
    setMinimumSize(w, 90);
    update();
}

void GanttBar::paintEvent(QPaintEvent *)
{
    if (m_gantt.isEmpty()) return;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int cellW = 60;
    const int barH  = 44;
    const int barY  = 10;
    const int timeY = barY + barH + 16;

    // compress into blocks
    struct Block { int pid, start, len; };
    QVector<Block> blocks;
    for (int i = 0; i < m_gantt.size(); ) {
        int j = i;
        while (j < m_gantt.size() && m_gantt[j] == m_gantt[i]) j++;
        blocks.append({m_gantt[i], i, j - i});
        i = j;
    }

    for (auto &blk : blocks) {
        int x = blk.start * cellW;
        int w = blk.len   * cellW;
        QColor fill = m_colors[blk.pid];

        p.setBrush(fill);
        p.setPen(Qt::NoPen);
        p.drawRect(x, barY, w, barH);
        p.setPen(QPen(fill.darker(130), 1));
        p.drawRect(x, barY, w, barH);

        p.setPen(Qt::white);
        QFont f; f.setBold(true); f.setPointSize(10);
        p.setFont(f);
        QString lbl = (blk.pid == 0) ? "Idle" : QString("P%1").arg(blk.pid);
        p.drawText(QRect(x, barY, w, barH), Qt::AlignCenter, lbl);
    }

    // time axis
    QFont tf; tf.setPointSize(9); p.setFont(tf);
    p.setPen(QColor("#444444"));
    for (int i = 0; i <= m_gantt.size(); i++) {
        int x = i * cellW;
        p.drawLine(x, barY + barH, x, barY + barH + 6);
        p.drawText(QRect(x - 15, timeY, 30, 16),
                   Qt::AlignCenter, QString::number(i));
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  GanttWindow — constructor
// ═══════════════════════════════════════════════════════════════════════════
GanttWindow::GanttWindow(QVector<Process> processes, int mode, int submode,
                         int quantum, bool isLive, QWidget *parent)
    : QWidget(parent),
    m_processes(processes),
    m_mode(mode), m_submode(submode),
    m_quantum(quantum), m_isLive(isLive),
    m_totalProcessCount(processes.size())
{
    setWindowTitle(isLive ? "Live Scheduler" : "Gantt Chart");
    setMinimumSize(900, 600);

    if (!m_isLive)
        runFullSchedule();
    else
        initLiveScheduler();

    buildUI();

    if (m_isLive) {
        m_timer = new QTimer(this);
        m_timer->setInterval(1000);          // 1 tick = 1 second
        connect(m_timer, &QTimer::timeout, this, &GanttWindow::onTick);
        m_timer->start();
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  STATIC — run full schedule at once (unchanged from before)
// ═══════════════════════════════════════════════════════════════════════════
void GanttWindow::runFullSchedule()
{
    std::vector<Process> allProcesses(m_processes.begin(), m_processes.end());
    std::vector<Process> finishedProcesses;
    std::vector<int>     gantt;
    int current_time = 0;

    if (m_mode == 1 || m_mode == 2) {
        std::priority_queue<Process, std::vector<Process>, Compare>
            readyQueue{Compare(m_mode)};

        while (!allProcesses.empty() || !readyQueue.empty()) {
            for (int i = 0; i < (int)allProcesses.size(); i++) {
                if (allProcesses[i].getArrivalTime() == current_time) {
                    readyQueue.push(allProcesses[i]);
                    allProcesses.erase(allProcesses.begin() + i);
                    i--;
                }
            }
            if (m_submode == 1) {
                if (!readyQueue.empty()) {
                    Process cur = readyQueue.top(); readyQueue.pop();
                    for (int t = 0; t < cur.getBurstTime(); t++) {
                        gantt.push_back(cur.getPid());
                        current_time++;
                        for (int i = 0; i < (int)allProcesses.size(); i++) {
                            if (allProcesses[i].getArrivalTime() == current_time) {
                                readyQueue.push(allProcesses[i]);
                                allProcesses.erase(allProcesses.begin() + i);
                                i--;
                            }
                        }
                    }
                    cur.setCompletionTime(current_time);
                    cur.calculateTimes();
                    finishedProcesses.push_back(cur);
                } else { gantt.push_back(0); current_time++; }
            } else {
                if (!readyQueue.empty()) {
                    Process cur = readyQueue.top(); readyQueue.pop();
                    gantt.push_back(cur.getPid());
                    cur.setRemainingTime(cur.getRemainingTime() - 1);
                    if (cur.getRemainingTime() == 0) {
                        cur.setCompletionTime(current_time + 1);
                        cur.calculateTimes();
                        finishedProcesses.push_back(cur);
                    } else { readyQueue.push(cur); }
                } else { gantt.push_back(0); }
                current_time++;
            }
        }
    } else if (m_mode == 3) {
        std::queue<Process> rrQueue;
        std::sort(allProcesses.begin(), allProcesses.end(),
                  [](Process a, Process b){ return a.getArrivalTime() < b.getArrivalTime(); });
        int idx = 0;
        while (idx < (int)allProcesses.size() || !rrQueue.empty()) {
            while (idx < (int)allProcesses.size() &&
                   allProcesses[idx].getArrivalTime() <= current_time)
                rrQueue.push(allProcesses[idx++]);
            if (rrQueue.empty()) { gantt.push_back(0); current_time++; continue; }
            Process cur = rrQueue.front(); rrQueue.pop();
            int runTime = std::min(cur.getRemainingTime(), m_quantum);
            for (int t = 0; t < runTime; t++) {
                gantt.push_back(cur.getPid());
                current_time++;
                cur.setRemainingTime(cur.getRemainingTime() - 1);
                while (idx < (int)allProcesses.size() &&
                       allProcesses[idx].getArrivalTime() <= current_time)
                    rrQueue.push(allProcesses[idx++]);
            }
            if (cur.getRemainingTime() > 0) rrQueue.push(cur);
            else { cur.setCompletionTime(current_time); cur.calculateTimes(); finishedProcesses.push_back(cur); }
        }
    } else if (m_mode == 4) {
        std::queue<Process> fcfsQueue;
        std::sort(allProcesses.begin(), allProcesses.end(),
                  [](Process a, Process b){ return a.getArrivalTime() < b.getArrivalTime(); });
        int idx = 0;
        while (idx < (int)allProcesses.size() || !fcfsQueue.empty()) {
            while (idx < (int)allProcesses.size() &&
                   allProcesses[idx].getArrivalTime() <= current_time)
                fcfsQueue.push(allProcesses[idx++]);
            if (fcfsQueue.empty()) { gantt.push_back(0); current_time++; continue; }
            Process cur = fcfsQueue.front(); fcfsQueue.pop();
            while (cur.getRemainingTime() > 0) {
                gantt.push_back(cur.getPid());
                current_time++;
                cur.setRemainingTime(cur.getRemainingTime() - 1);
                while (idx < (int)allProcesses.size() &&
                       allProcesses[idx].getArrivalTime() <= current_time)
                    fcfsQueue.push(allProcesses[idx++]);
            }
            cur.setCompletionTime(current_time);
            cur.calculateTimes();
            finishedProcesses.push_back(cur);
        }
    }

    m_gantt    = QVector<int>(gantt.begin(), gantt.end());
    m_finished = QVector<Process>(finishedProcesses.begin(), finishedProcesses.end());
}

// ═══════════════════════════════════════════════════════════════════════════
//  LIVE — initialise state before first tick
// ═══════════════════════════════════════════════════════════════════════════
void GanttWindow::initLiveScheduler()
{
    m_liveAll = std::vector<Process>(m_processes.begin(), m_processes.end());

    // sort by arrival for RR / FCFS
    if (m_mode == 3 || m_mode == 4)
        std::sort(m_liveAll.begin(), m_liveAll.end(),
                  [](Process a, Process b){ return a.getArrivalTime() < b.getArrivalTime(); });

    m_currentTime  = 0;
    m_paused       = false;
    m_done         = false;
    m_hasRunning   = false;
    m_runningLeft  = 0;
    m_rrIdx        = 0;
    m_rrLeft       = 0;
}

// helper: insert into m_readyVec keeping SJF/Priority order
void GanttWindow::pushToReadyQueue(Process p)
{
    m_readyVec.push_back(p);
    // sort so front is highest priority (smallest remaining / smallest priority number)
    std::sort(m_readyVec.begin(), m_readyVec.end(), [&](Process a, Process b){
        return !Compare(m_mode)(a, b); // reverse of max-heap = ascending
    });
}

// ═══════════════════════════════════════════════════════════════════════════
//  LIVE — one tick per second
// ═══════════════════════════════════════════════════════════════════════════
void GanttWindow::onTick()
{
    if (m_done) { m_timer->stop(); return; }
    stepOneTick();
    m_clockLabel->setText(QString("Time: %1s").arg(m_currentTime));
    m_ganttBar->appendTick(m_gantt.last());

    // auto-scroll gantt to the right
    m_scrollArea->horizontalScrollBar()->setValue(
        m_scrollArea->horizontalScrollBar()->maximum());

    updateRemainingTable();

    if (m_done) {
        m_timer->stop();
        m_pauseBtn->setEnabled(false);
        m_addBtn->setEnabled(false);
        fillStatsTable();
        updateAverages();
        QMessageBox::information(this, "Done", "Scheduling complete!");
    }
}

void GanttWindow::stepOneTick()
{
    // ── arrive any processes at current_time ──────────────────────────────
    for (int i = 0; i < (int)m_liveAll.size(); ) {
        if (m_liveAll[i].getArrivalTime() <= m_currentTime) {
            if (m_mode == 1 || m_mode == 2)
                pushToReadyQueue(m_liveAll[i]);
            else if (m_mode == 3)
                m_rrQueue.push(m_liveAll[i]);
            else
                m_fcfsQueue.push(m_liveAll[i]);
            m_liveAll.erase(m_liveAll.begin() + i);
        } else { i++; }
    }

    // ── SJF / Priority ────────────────────────────────────────────────────
    if (m_mode == 1 || m_mode == 2) {

        if (m_submode == 1) {   // NON-PREEMPTIVE
            // if nothing running, pick next from ready queue
            if (!m_hasRunning && !m_readyVec.empty()) {
                m_runningProcess = m_readyVec.front();
                m_readyVec.erase(m_readyVec.begin());
                m_runningLeft = m_runningProcess.getBurstTime();
                m_hasRunning  = true;
            }
            if (m_hasRunning) {
                m_gantt.append(m_runningProcess.getPid());
                m_runningProcess.setRemainingTime(m_runningLeft - 1);
                m_runningLeft--;
                if (m_runningLeft == 0) {
                    m_runningProcess.setCompletionTime(m_currentTime + 1);
                    m_runningProcess.calculateTimes();
                    m_liveFinished.push_back(m_runningProcess);
                    m_hasRunning = false;
                }
            } else {
                m_gantt.append(0); // idle
            }

        } else {                // PREEMPTIVE
            if (!m_readyVec.empty()) {
                Process cur = m_readyVec.front();
                m_readyVec.erase(m_readyVec.begin());
                m_gantt.append(cur.getPid());
                cur.setRemainingTime(cur.getRemainingTime() - 1);
                if (cur.getRemainingTime() == 0) {
                    cur.setCompletionTime(m_currentTime + 1);
                    cur.calculateTimes();
                    m_liveFinished.push_back(cur);
                } else {
                    pushToReadyQueue(cur);
                }
            } else {
                m_gantt.append(0);
            }
        }

        // ── Round Robin ───────────────────────────────────────────────────────
    } else if (m_mode == 3) {

        if (!m_hasRunning && !m_rrQueue.empty()) {
            m_runningProcess = m_rrQueue.front();
            m_rrQueue.pop();
            m_runningLeft = std::min(m_runningProcess.getRemainingTime(), m_quantum);
            m_hasRunning  = true;
        }
        if (m_hasRunning) {
            m_gantt.append(m_runningProcess.getPid());
            m_runningProcess.setRemainingTime(m_runningProcess.getRemainingTime() - 1);
            m_runningLeft--;
            if (m_runningProcess.getRemainingTime() == 0) {
                m_runningProcess.setCompletionTime(m_currentTime + 1);
                m_runningProcess.calculateTimes();
                m_liveFinished.push_back(m_runningProcess);
                m_hasRunning = false;
            } else if (m_runningLeft == 0) {
                // quantum expired — re-queue
                m_rrQueue.push(m_runningProcess);
                m_hasRunning = false;
            }
        } else {
            m_gantt.append(0);
        }

        // ── FCFS ──────────────────────────────────────────────────────────────
    } else if (m_mode == 4) {

        if (!m_hasRunning && !m_fcfsQueue.empty()) {
            m_runningProcess = m_fcfsQueue.front();
            m_fcfsQueue.pop();
            m_hasRunning = true;
        }
        if (m_hasRunning) {
            m_gantt.append(m_runningProcess.getPid());
            m_runningProcess.setRemainingTime(m_runningProcess.getRemainingTime() - 1);
            if (m_runningProcess.getRemainingTime() == 0) {
                m_runningProcess.setCompletionTime(m_currentTime + 1);
                m_runningProcess.calculateTimes();
                m_liveFinished.push_back(m_runningProcess);
                m_hasRunning = false;
            }
        } else {
            m_gantt.append(0);
        }
    }

    m_currentTime++;

    // check if we are done
    bool queuesEmpty = m_readyVec.empty() && m_rrQueue.empty() &&
                       m_fcfsQueue.empty() && m_liveAll.empty() && !m_hasRunning;
    if (queuesEmpty)
        m_done = true;

    // sync m_finished for stats
    m_finished = QVector<Process>(m_liveFinished.begin(), m_liveFinished.end());
}

// ═══════════════════════════════════════════════════════════════════════════
//  PAUSE / RESUME
// ═══════════════════════════════════════════════════════════════════════════
void GanttWindow::onPauseResume()
{
    if (m_done) return;
    m_paused = !m_paused;
    if (m_paused) {
        m_timer->stop();
        m_pauseBtn->setText("Resume");
        m_addBtn->setEnabled(true);
    } else {
        m_addBtn->setEnabled(false);
        m_pauseBtn->setText("Pause");
        m_timer->start();
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  ADD PROCESS (only while paused)
// ═══════════════════════════════════════════════════════════════════════════
void GanttWindow::onAddProcess()
{
    // build a small dialog
    QDialog dlg(this);
    dlg.setWindowTitle("Add Process");
    QFormLayout *form = new QFormLayout(&dlg);

    QLineEdit *atEdit = new QLineEdit;
    QLineEdit *btEdit = new QLineEdit;
    QLineEdit *prEdit = new QLineEdit;

    // pre-fill arrival with current time so it enters immediately
    atEdit->setText(QString::number(m_currentTime));
    atEdit->setReadOnly(true);

    form->addRow("Arrival Time:", atEdit);
    form->addRow("Burst Time:",   btEdit);
    if (m_mode == 2)
        form->addRow("Priority:", prEdit);

    QDialogButtonBox *btns = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    form->addRow(btns);

    if (dlg.exec() != QDialog::Accepted) return;

    int bt = btEdit->text().toInt();
    if (bt <= 0) {
        QMessageBox::warning(this, "Invalid", "Burst time must be > 0");
        return;
    }

    // assign next PID
    m_totalProcessCount++;
    int newPid = m_totalProcessCount;
    int at     = m_currentTime;   // arrives right now

    Process newP;
    if (m_mode == 2) {
        int pr = prEdit->text().toInt();
        newP = Process(newPid, at, bt, pr);
    } else {
        newP = Process(newPid, at, bt);
    }

    // push directly into the correct queue since it's already "arrived"
    if (m_mode == 1 || m_mode == 2)
        pushToReadyQueue(newP);
    else if (m_mode == 3)
        m_rrQueue.push(newP);
    else
        m_fcfsQueue.push(newP);

    // also add to remaining table immediately
    updateRemainingTable();
}

// ═══════════════════════════════════════════════════════════════════════════
//  BUILD UI
// ═══════════════════════════════════════════════════════════════════════════
void GanttWindow::buildUI()
{
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setSpacing(12);
    root->setContentsMargins(16, 16, 16, 16);

    // ── top bar: title + clock + pause + add ──────────────────────────────
    QHBoxLayout *topBar = new QHBoxLayout;
    QLabel *title = new QLabel(m_isLive ? "Live Scheduler" : "Gantt Chart — Static");
    QFont tf; tf.setPointSize(13); tf.setBold(true);
    title->setFont(tf);
    topBar->addWidget(title);
    topBar->addStretch();

    if (m_isLive) {
        m_clockLabel = new QLabel("Time: 0s");
        m_clockLabel->setStyleSheet("font-size:13px; font-weight:bold; color:#2c7be5;");
        topBar->addWidget(m_clockLabel);
        topBar->addSpacing(16);

        m_pauseBtn = new QPushButton("Pause");
        m_pauseBtn->setFixedWidth(90);
        connect(m_pauseBtn, &QPushButton::clicked, this, &GanttWindow::onPauseResume);
        topBar->addWidget(m_pauseBtn);

        m_addBtn = new QPushButton("+ Add Process");
        m_addBtn->setEnabled(false);   // only enabled when paused
        connect(m_addBtn, &QPushButton::clicked, this, &GanttWindow::onAddProcess);
        topBar->addWidget(m_addBtn);
    }
    root->addLayout(topBar);

    // ── gantt bar ─────────────────────────────────────────────────────────
    QLabel *ganttLabel = new QLabel("Gantt Chart:");
    ganttLabel->setStyleSheet("font-weight:bold;");
    root->addWidget(ganttLabel);

    m_ganttBar = new GanttBar;
    if (!m_isLive)
        m_ganttBar->setGantt(m_gantt);   // static: draw all at once

    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidget(m_ganttBar);
    m_scrollArea->setWidgetResizable(false);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setMinimumHeight(110);
    m_scrollArea->setFrameShape(QFrame::StyledPanel);
    root->addWidget(m_scrollArea);

    // ── two tables side by side ───────────────────────────────────────────
    QHBoxLayout *tablesRow = new QHBoxLayout;

    // left: stats table (completion / turnaround / waiting — filled at end)
    QVBoxLayout *leftCol = new QVBoxLayout;
    QLabel *statsLbl = new QLabel("Process Statistics");
    statsLbl->setStyleSheet("font-weight:bold;");
    leftCol->addWidget(statsLbl);

    m_statsTable = new QTableWidget;
    m_statsTable->setColumnCount(6);
    m_statsTable->setHorizontalHeaderLabels({
                                             "PID","Arrival","Burst","Completion","Turnaround","Waiting"});
    m_statsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_statsTable->setAlternatingRowColors(true);
    m_statsTable->setMinimumHeight(160);
    leftCol->addWidget(m_statsTable);
    tablesRow->addLayout(leftCol, 3);

    // right: remaining time table (live only)
    if (m_isLive) {
        QVBoxLayout *rightCol = new QVBoxLayout;
        QLabel *remLbl = new QLabel("Remaining Burst Time");
        remLbl->setStyleSheet("font-weight:bold;");
        rightCol->addWidget(remLbl);

        m_remainingTable = new QTableWidget;
        m_remainingTable->setColumnCount(2);
        m_remainingTable->setHorizontalHeaderLabels({"PID", "Remaining"});
        m_remainingTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        m_remainingTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        m_remainingTable->setAlternatingRowColors(true);
        m_remainingTable->setMinimumHeight(160);
        rightCol->addWidget(m_remainingTable);
        tablesRow->addLayout(rightCol, 1);

        // populate remaining table with initial values
        updateRemainingTable();
    }

    root->addLayout(tablesRow);

    // ── averages ──────────────────────────────────────────────────────────
    m_avgLabel = new QLabel;
    m_avgLabel->setStyleSheet("font-size:13px; padding:4px;");
    root->addWidget(m_avgLabel);

    // for static mode fill everything now
    if (!m_isLive) {
        fillStatsTable();
        updateAverages();
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  REMAINING TIME TABLE  (updated every tick)
// ═══════════════════════════════════════════════════════════════════════════
void GanttWindow::updateRemainingTable()
{
    // collect all processes still in play
    // sources: m_liveAll (not arrived yet), readyVec, queues, running process
    struct Entry { int pid; int remaining; };
    QVector<Entry> entries;

    // not yet arrived
    for (auto &p : m_liveAll)
        entries.append({p.getPid(), p.getRemainingTime()});

    // in ready queue (SJF/Priority)
    for (auto &p : m_readyVec)
        entries.append({p.getPid(), p.getRemainingTime()});

    // in RR queue
    {
        std::queue<Process> tmp = m_rrQueue;
        while (!tmp.empty()) {
            entries.append({tmp.front().getPid(), tmp.front().getRemainingTime()});
            tmp.pop();
        }
    }

    // in FCFS queue
    {
        std::queue<Process> tmp = m_fcfsQueue;
        while (!tmp.empty()) {
            entries.append({tmp.front().getPid(), tmp.front().getRemainingTime()});
            tmp.pop();
        }
    }

    // currently running
    if (m_hasRunning)
        entries.append({m_runningProcess.getPid(), m_runningProcess.getRemainingTime()});

    // sort by PID for stable display
    std::sort(entries.begin(), entries.end(),
              [](Entry a, Entry b){ return a.pid < b.pid; });

    m_remainingTable->setRowCount(entries.size());
    for (int i = 0; i < entries.size(); i++) {
        auto *pidItem = new QTableWidgetItem(QString("P%1").arg(entries[i].pid));
        auto *remItem = new QTableWidgetItem(QString::number(entries[i].remaining));
        pidItem->setTextAlignment(Qt::AlignCenter);
        remItem->setTextAlignment(Qt::AlignCenter);

        // highlight the currently running process in light blue
        if (m_hasRunning && entries[i].pid == m_runningProcess.getPid()) {
            pidItem->setBackground(QColor("#d0e8ff"));
            remItem->setBackground(QColor("#d0e8ff"));
        }

        m_remainingTable->setItem(i, 0, pidItem);
        m_remainingTable->setItem(i, 1, remItem);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  STATS TABLE  (filled at end for live, immediately for static)
// ═══════════════════════════════════════════════════════════════════════════
void GanttWindow::fillStatsTable()
{
    m_statsTable->setRowCount(m_finished.size());
    for (int i = 0; i < m_finished.size(); i++) {
        Process &proc = m_finished[i];
        m_statsTable->setItem(i,0,new QTableWidgetItem(QString::number(proc.getPid())));
        m_statsTable->setItem(i,1,new QTableWidgetItem(QString::number(proc.getArrivalTime())));
        m_statsTable->setItem(i,2,new QTableWidgetItem(QString::number(proc.getBurstTime())));
        m_statsTable->setItem(i,3,new QTableWidgetItem(QString::number(proc.getCompletionTime())));
        m_statsTable->setItem(i,4,new QTableWidgetItem(QString::number(proc.getTurnaroundTime())));
        m_statsTable->setItem(i,5,new QTableWidgetItem(QString::number(proc.getWaitingTime())));
        for (int c = 0; c < 6; c++)
            m_statsTable->item(i,c)->setTextAlignment(Qt::AlignCenter);
    }
}

void GanttWindow::updateAverages()
{
    float totalWT = 0, totalTAT = 0;
    for (auto &p : m_finished) { totalWT += p.getWaitingTime(); totalTAT += p.getTurnaroundTime(); }
    int n = m_finished.size();
    if (n > 0)
        m_avgLabel->setText(
            QString("Average Waiting Time: <b>%1</b>  |  Average Turnaround Time: <b>%2</b>")
                .arg(totalWT/n, 0,'f',2).arg(totalTAT/n, 0,'f',2));
}