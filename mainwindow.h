#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Merged.cpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void on_pushButton_clicked();
    void on_modeComboBox_changed(int index);
    void validateAddButton();
    void addToTable();
    QVector<Process> collectProcesses();

    void on_addButton_clicked();

    void on_submitButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
