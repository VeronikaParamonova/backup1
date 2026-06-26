#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDebug>
#include "IAdapter.h"
#include "ioccontainer.h"
#include "ModelData.h"
#include <QFileInfo>
#include <QDir>
#include <QtCharts/QChartView>
#include "IChart.h"
#include "barchart.h"
#include "linechart.h"
#include <QPrinter>
#include <QPainter>
#include <QPdfWriter>
#include <QFileDialog>

#include <QStringListModel>
#include <QListView>  //в примере был QTreeView
#include <QItemSelectionModel>
#include <QSplitter>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Конструктор принимает контейнер
    MainWindow(std::shared_ptr<IOCContainer> container, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLoadButtonClicked();  // ← слот для кнопки
    void onFileSelected(const QModelIndex &index);//слот для выбора файла
    void onRemoveFileClicked();//удаление файла
    void onChartTypeChanged(int index);
    void onGray(int state); //перекрас в чёрно-белый
    void onPrintButtonClicked(); //печать PDF

private:
    Ui::MainWindow *ui;
    QPushButton* m_loadButton;   // ← кнопка "Загрузить файл"
    QPushButton* m_removeButton;
    QPushButton* m_printButton;
    std::shared_ptr<IOCContainer> m_container;  // ← контейнер
    QVBoxLayout* m_rightLayout; //Нужно найти правый виджет и добавить туда
    QComboBox* m_chartTypeCombo;
    QCheckBox* m_grayCheckBox;

    // MVC
    QListView* m_fileListView;
    QStringListModel* m_fileModel;
    QChartView* m_chartView;  // ← представление графика

    // Данные (синхронизированы по индексу)
    QStringList m_filePaths;      // пути к файлам
    QList<DataSet> m_fileData;    // данные файлов

    void updateFileList();
    void updateStatusBar(const DataSet& data);
    void updateChart(const DataSet& data, const QString& chartType);
    void GrayStyle(QChart* chart);
    void showErrorMessage(const QString& title, const QString& message);
};
#endif // MAINWINDOW_H
