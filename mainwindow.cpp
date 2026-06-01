#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "staticorlive.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // Populate mode dropdown
    ui->modeComboBox->addItem("-");
    ui->modeComboBox->addItem("SJF");
    ui->modeComboBox->addItem("Priority");
    ui->modeComboBox->addItem("Round Robin");
    ui->modeComboBox->addItem("FCFS");

    // Set initial state — default disable everything
    ui->quantumSpinBox->setEnabled(false);
    ui->submodeComboBox->setEnabled(false);
    ui->processSpinBox->setEnabled(false);
    ui->predit->setEnabled(false);
    ui->arredit->setEnabled(false);
    ui->burstedit->setEnabled(false);
    ui->addButton->setEnabled(false);
    ui->submitButton->setEnabled(false);
    ui->tableWidget->setColumnHidden(2,true);


    // Connect combobox change to slot
    connect(ui->modeComboBox, &QComboBox::currentIndexChanged,
            this, &MainWindow::on_modeComboBox_changed);
    connect(ui->arredit, &QLineEdit::textChanged,
            this, &MainWindow::validateAddButton);
    connect(ui->burstedit, &QLineEdit::textChanged,
            this, &MainWindow::validateAddButton);
    connect(ui->predit, &QLineEdit::textChanged,
            this, &MainWindow::validateAddButton);
    connect(ui->addButton, &QPushButton::clicked,
            this, &MainWindow::addToTable);
    connect(ui->processSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() {
                ui->tableWidget->setRowCount(0);
                ui->submitButton->setEnabled(false);
                // Re-enable inputs based on current mode
                on_modeComboBox_changed(ui->modeComboBox->currentIndex());
            });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->arredit->copy();
    ui->tableWidget->insertRow(ui->tableWidget->rowCount());
}

void MainWindow::on_modeComboBox_changed(int index) {
    ui->tableWidget->setRowCount(0);
    ui->submitButton->setEnabled(false);

    switch(index) {
    case 0: // -
        ui->quantumSpinBox->setEnabled(false);
        ui->submodeComboBox->setEnabled(false);
        ui->submodeComboBox->clear();
        ui->predit->setEnabled(false);
        ui->arredit->setEnabled(false);
        ui->burstedit->setEnabled(false);
        ui->addButton->setEnabled(false);
        ui->processSpinBox->setEnabled(false);
        break;

    case 1: // SJF
        ui->quantumSpinBox->setEnabled(false);
        ui->submodeComboBox->setEnabled(true);
        ui->submodeComboBox->clear();
        ui->submodeComboBox->addItem("Non-Preemptive");
        ui->submodeComboBox->addItem("Preemptive");
        ui->processSpinBox->setEnabled(true);
        ui->predit->setEnabled(false);
        ui->arredit->setEnabled(true);
        ui->burstedit->setEnabled(true);
        ui->addButton->setEnabled(false);
        break;

    case 2: // Priority
        ui->quantumSpinBox->setEnabled(false);
        ui->submodeComboBox->setEnabled(true);
        ui->submodeComboBox->clear();
        ui->submodeComboBox->addItem("Non-Preemptive");
        ui->submodeComboBox->addItem("Preemptive");
        ui->processSpinBox->setEnabled(true);
        ui->predit->setEnabled(true);
        ui->arredit->setEnabled(true);
        ui->burstedit->setEnabled(true);
        ui->addButton->setEnabled(false);
        break;

    case 3: // Round Robin
        ui->quantumSpinBox->setEnabled(true);   // enable quantum
        ui->submodeComboBox->setEnabled(false);  // no submode for RR
        ui->submodeComboBox->clear();
        ui->processSpinBox->setEnabled(true);
        ui->predit->setEnabled(false);
        ui->arredit->setEnabled(true);
        ui->burstedit->setEnabled(true);
        ui->addButton->setEnabled(false);
        break;

    case 4: // FCFS
        ui->quantumSpinBox->setEnabled(false);
        ui->submodeComboBox->setEnabled(false);  // no submode for FCFS
        ui->submodeComboBox->clear();
        ui->processSpinBox->setEnabled(true);
        ui->predit->setEnabled(false);
        ui->arredit->setEnabled(true);
        ui->burstedit->setEnabled(true);
        ui->addButton->setEnabled(false);
        break;
    }
}
void MainWindow::validateAddButton() {
    int currentMode = ui->modeComboBox->currentIndex();

    bool arrivalFilled = !ui->arredit->text().trimmed().isEmpty();
    bool burstFilled   = !ui->burstedit->text().trimmed().isEmpty();
    bool priorityFilled = !ui->predit->text().trimmed().isEmpty();

    if (currentMode == 2) {
        // Priority mode — all 3 fields required
        ui->addButton->setEnabled(arrivalFilled && burstFilled && priorityFilled);
    } else {
        // SJF, RR, FCFS — only arrival and burst required
        ui->addButton->setEnabled(arrivalFilled && burstFilled);
    }
}


void MainWindow::on_addButton_clicked()
{
    ui->submitButton->setEnabled(true);

}

void MainWindow::addToTable()
{
    int currentMode = ui->modeComboBox->currentIndex();
    QString arr = ui->arredit->text();
    QString burst = ui->burstedit->text();
    QString prio = ui->predit->text();



    if (currentMode == 2) {
        // Priority mode — all 3 fields required
        if (arr.isEmpty()||burst.isEmpty()||prio.isEmpty())
            return;

        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setColumnHidden(2,false);

        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(arr));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(burst));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(prio));
        ui->arredit->clear();
        ui->burstedit->clear();
        ui->predit->clear();
    }
     else {
        // SJF, RR, FCFS — only arrival and burst required
        if (arr.isEmpty()||burst.isEmpty())
            return;

        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);


        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(arr));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(burst));


        ui->arredit->clear();
        ui->burstedit->clear();
    }

    int limit = ui->processSpinBox->value();
    if (ui->tableWidget->rowCount() >= limit) {
        ui->arredit->setEnabled(false);
        ui->burstedit->setEnabled(false);
        ui->predit->setEnabled(false);
        ui->addButton->setEnabled(false);
        ui->submitButton->setEnabled(true);
    }
}



void MainWindow::on_submitButton_clicked()
{
    // 1. Read mode (comboBox index → your mode numbers)
    // comboBox:  0="-"  1=SJF  2=Priority  3=RR  4=FCFS
    // your code: mode   1=SJF  2=Priority  3=RR  4=FCFS
    int mode = ui->modeComboBox->currentIndex(); // already 1-4, index 0 is "-"

    // 2. Read submode (only for SJF and Priority)
    // submodeComboBox: index 0 = Non-Preemptive (your submode=1)
    //                  index 1 = Preemptive     (your submode=2)
    int submode = ui->submodeComboBox->currentIndex() + 1; // convert 0/1 → 1/2

    // 3. Read quantum (only for Round Robin)
    int quantum = ui->quantumSpinBox->value();

    // 4. Collect processes from table
    QVector<Process> processes = collectProcesses();

    // 5. Open StaticOrLive window, pass everything
    StaticOrLive *s = new StaticOrLive(processes, mode, submode, quantum);
    s->show();
    this->close();
}

// Add this function — reads every row from the tableWidget
// and builds the QVector<Process> to pass forward
QVector<Process> MainWindow::collectProcesses()
{
    QVector<Process> processes;
    int currentMode = ui->modeComboBox->currentIndex();
    int rows = ui->tableWidget->rowCount();

    for (int i = 0; i < rows; i++) {
        int at = ui->tableWidget->item(i, 0)->text().toInt();
        int bt = ui->tableWidget->item(i, 1)->text().toInt();

        if (currentMode == 2) {
            // Priority mode — read 3rd column too
            int pr = ui->tableWidget->item(i, 2)->text().toInt();
            processes.push_back(Process(i + 1, at, bt, pr));
        } else {
            processes.push_back(Process(i + 1, at, bt));
        }
    }
    return processes;

}

