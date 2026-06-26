#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(std::shared_ptr<IOCContainer> container, QWidget *parent)
    :  QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_loadButton(nullptr)
    , m_removeButton(nullptr)
    , m_printButton(nullptr)
    , m_container(container)
    , m_fileListView(nullptr)
    , m_fileModel(nullptr)
    , m_rightLayout(nullptr)
    , m_chartView(nullptr)
    , m_chartTypeCombo(nullptr)
    , m_grayCheckBox(nullptr)
{
    ui->setupUi(this);

    // 1. Устанавливаем размер окна
    this->setGeometry(100, 100, 800, 400);
    this->setStatusBar(new QStatusBar(this));
    this->statusBar()->showMessage("Готов к работе");
    this->setWindowTitle("Печать графиков");

    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    //левая часть
    QWidget* leftWidget = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftWidget);

    QLabel* titleLabel = new QLabel("Загруженные файлы");
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    leftLayout->addWidget(titleLabel);

    // Модель
    m_fileModel = new QStringListModel(this);

    // Представление
    m_fileListView = new QListView();
    m_fileListView->setModel(m_fileModel);
    m_fileListView->setSelectionMode(QAbstractItemView::SingleSelection);
    leftLayout->addWidget(m_fileListView);


    // Кнопки
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_loadButton = new QPushButton("Загрузить файл");
    m_removeButton = new QPushButton("Удалить");
    m_removeButton->setEnabled(false);
    buttonLayout->addWidget(m_loadButton);
    buttonLayout->addWidget(m_removeButton);
    leftLayout->addLayout(buttonLayout);

    splitter->addWidget(leftWidget);

    //правая часть
    QWidget* rightWidget = new QWidget();
    m_rightLayout = new QVBoxLayout(rightWidget);  // ← сохраняем layout!

    QHBoxLayout* ButtonsLayout = new QHBoxLayout();

    //выбор типа графика
    m_chartTypeCombo = new QComboBox();
    m_chartTypeCombo->addItem("BarChart");
    m_chartTypeCombo->addItem("LineChart");
    ButtonsLayout->addWidget(m_chartTypeCombo);

    //Создание чекбокса на ч/б
    m_grayCheckBox = new QCheckBox("Черно-белый график");
    ButtonsLayout->addWidget(m_grayCheckBox);

    // Кнопка печати PDF
    QPushButton* m_printButton = new QPushButton("Печать PDF");
    ButtonsLayout->addWidget(m_printButton);


    m_rightLayout->addLayout(ButtonsLayout);//добавляем горизонтальный layout в основной вертикальный

    //пустой график
    m_chartView = new QChartView();
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(400);
    m_rightLayout->addWidget(m_chartView);

    splitter->addWidget(rightWidget);
    splitter->setSizes({250, 750});

    setCentralWidget(splitter);


    // Подключаем сигналы
    connect(m_loadButton, &QPushButton::clicked, this, &MainWindow::onLoadButtonClicked);

    connect(m_removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveFileClicked);

    connect(m_fileListView, &QListView::clicked, this, &MainWindow::onFileSelected);

    connect(m_chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onChartTypeChanged);

    connect(m_grayCheckBox, &QCheckBox::stateChanged, this, &MainWindow::onGray);

    connect(m_printButton, &QPushButton::clicked, this, &MainWindow::onPrintButtonClicked);


    // 6. Проверяем, что контейнер получен
    if (!m_container) {
        qDebug() << "Container is null!";
    } else {
        qDebug() << "Container received successfully";
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::onLoadButtonClicked()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        "Выберите файлы с данными",
        QDir::homePath(),
        "Файлы данных (*.db *.sqlite *.json);;Все файлы (*.*)"
        );
    if (filePaths.isEmpty()) {
        this->statusBar()->showMessage("Выбор файла отменён");
        return;
    }

    int loadedCount = 0;
    for (const QString& filePath : filePaths) {
        QFileInfo fileInfo(filePath);
        QString extension = fileInfo.suffix().toLower();

        // Проверка формата
        if (extension != "db" && extension != "sqlite" &&
            extension != "sqlite3" && extension != "json") {
            qDebug() << "Неподдерживаемый формат:" << filePath;
            continue;
        }

        // Получаем адаптер
        auto adapter = m_container->GetObject<IAdapter>(filePath);
        if (!adapter) {
            qDebug() << "Нет адаптера для:" << filePath;
            continue;
        }

        // Загружаем данные
        DataSet data = adapter->loadData(filePath);
        if (data.isEmpty()) {
            qDebug() << "Нет данных в:" << filePath;
            continue;
        }

        // Сохраняем
        m_filePaths.append(filePath);
        m_fileData.append(data);

        loadedCount++;
        qDebug() << "Загружен:" << fileInfo.fileName();
    }

    updateFileList();

    m_removeButton->setEnabled(m_filePaths.size() > 0);

    this->statusBar()->showMessage(
        "Загружено файлов: " + QString::number(loadedCount) +
        " из " + QString::number(filePaths.size())
    );

    // Автоматически выбираем первый файл
    if (m_filePaths.size() > 0) {
        m_fileListView->setCurrentIndex(m_fileModel->index(0));
        onFileSelected(m_fileModel->index(0));
    }
}

void MainWindow::updateFileList()
{
    QStringList displayStrings;
    for (int i = 0; i < m_filePaths.size(); ++i) {
        QFileInfo fileInfo(m_filePaths[i]);
        displayStrings << QString("%1 (%2 точек)")
                              .arg(fileInfo.fileName())
                              .arg(m_fileData[i].size());
    }
    m_fileModel->setStringList(displayStrings);
}

void MainWindow::onFileSelected(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_fileData.size()) {
        return;
    }

    const DataSet& data = m_fileData[index.row()];
    QFileInfo fileInfo(m_filePaths[index.row()]);

    updateStatusBar(data);
    updateChart(data, m_chartTypeCombo->currentText());   //вызываем updateChart с типом графика

    qDebug() << "Выбран файл:" << fileInfo.fileName();
    qDebug() << "   Точек:" << data.size();

}

void MainWindow::onChartTypeChanged(int index)
{
    Q_UNUSED(index);

    // Если есть загруженные данные — обновляем график
    if (m_filePaths.isEmpty()) return;

    int currentRow = m_fileListView->currentIndex().row();
    if (currentRow < 0 || currentRow >= m_fileData.size()) return;

    const DataSet& data = m_fileData[currentRow];
    updateChart(data, m_chartTypeCombo->currentText());
}

void MainWindow::onGray(int state)
{
    Q_UNUSED(state);
    if (m_filePaths.isEmpty()) return;

    int currentRow = m_fileListView->currentIndex().row();
    if (currentRow < 0 || currentRow >= m_fileData.size()) return;

    const DataSet& data = m_fileData[currentRow];
    updateChart(data, m_chartTypeCombo->currentText());
}

void MainWindow::GrayStyle(QChart* chart)
{
    if (!chart) return;

    //фон
    chart->setBackgroundBrush(QBrush(Qt::white));
    chart->setPlotAreaBackgroundBrush(QBrush(Qt::white));
    chart->setPlotAreaBackgroundVisible(true);

    //оси и линии сетки
    QList<QAbstractAxis*> axes = chart->axes();
    for (QAbstractAxis* axis : axes) {
        axis->setLinePen(QPen(Qt::black, 1));
        axis->setGridLinePen(QPen(Qt::gray, 0.5, Qt::DashLine));
        axis->setLabelsColor(Qt::black);
        axis->setTitleBrush(QBrush(Qt::black));
    }

    //легенда
    QLegend* legend = chart->legend();
    if (legend) {
        legend->setLabelColor(Qt::black);
        legend->setBrush(QBrush(Qt::white));
        legend->setPen(QPen(Qt::black));
    }

    //серии
    QList<QAbstractSeries*> seriesList = chart->series();
    for (QAbstractSeries* series : seriesList) {
        if (QLineSeries* lineSeries = qobject_cast<QLineSeries*>(series)) {
            lineSeries->setColor(Qt::black);
            lineSeries->setPen(QPen(Qt::black, 2));
        } else if (QBarSeries* barSeries = qobject_cast<QBarSeries*>(series)) {
            // QBarSeries не имеет setColor, поэтому работаем через QBarSet
            QList<QBarSet*> sets = barSeries->barSets();
            for (QBarSet* set : sets) {
                set->setColor(Qt::black);
                set->setBorderColor(Qt::black);
            }
        }
    }

    //Заголовок
    chart->setTitleBrush(QBrush(Qt::black));

    chart->setAnimationOptions(QChart::NoAnimation);
    chart->setAnimationOptions(QChart::SeriesAnimations);
}


void MainWindow::updateChart(const DataSet& data, const QString& chartType)
{
    // Удаляем старый график (если есть)
    if (m_chartView) {
        m_rightLayout->removeWidget(m_chartView);
        delete m_chartView;
        m_chartView = nullptr;
    }

    // Создаём новый график по типу
    if (chartType == "BarChart") {
        BarChart chart;
        m_chartView = chart.createChart(data);
    } else if (chartType == "LineChart") {
        LineChart chart;
        m_chartView = chart.createChart(data);
    } else {
        m_chartView = new QChartView();  // пустой график
    }

    // ч/б
    if (m_grayCheckBox && m_grayCheckBox->isChecked())
    {
        GrayStyle(m_chartView->chart());
    }

    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_rightLayout->addWidget(m_chartView);
}



void MainWindow::onRemoveFileClicked()
{
    QModelIndex index = m_fileListView->currentIndex();
    if (!index.isValid()) return;

    QFileInfo fileInfo(m_filePaths[index.row()]);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Удаление файла",
        "Удалить файл \"" + fileInfo.fileName() + "\" из списка?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply != QMessageBox::Yes) return;

    // Удаляем из всех трёх списков
    m_filePaths.removeAt(index.row());
    m_fileData.removeAt(index.row());
    updateFileList();

    m_removeButton->setEnabled(m_filePaths.size() > 0);

    this->statusBar()->showMessage("Файл удалён: " + fileInfo.fileName());
}


void MainWindow::updateStatusBar(const DataSet& data)
{
    if (data.isEmpty()) {
        this->statusBar()->showMessage("Нет данных");
        return;
    }

    double minVal = data.points[0].value;
    double maxVal = data.points[0].value;
    double sum = 0;

    for (const auto& p : data.points) {
        minVal = qMin(minVal, p.value);
        maxVal = qMax(maxVal, p.value);
        sum += p.value;
    }
    double avg = sum / data.size();

    QString statusText = QString(
                             "Выбран: %1 | Точек: %2 | Мин: %3 | Макс: %4 | Ср: %5"
                             )
                             .arg(data.name)
                             .arg(data.size())
                             .arg(minVal, 0, 'f', 2)
                             .arg(maxVal, 0, 'f', 2)
                             .arg(avg, 0, 'f', 2);

    this->statusBar()->showMessage(statusText);
}

void MainWindow::onPrintButtonClicked()
{

    if (!m_chartView) {
        QMessageBox::warning(this, "Ошибка", "Нет графика для печати!");
        return;
    }

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Сохранить PDF",
        QDir::homePath() + "/график.pdf",
        "PDF файлы (*.pdf)"
    );

    if (filePath.isEmpty()) {
        this->statusBar()->showMessage("Сохранение PDF отменено");
        return;
    }

    // 1. Создаём PDF writer
    QPdfWriter writer(filePath);
    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setResolution(300);

    // 2. Ч/Б режим
    if (m_grayCheckBox && m_grayCheckBox->isChecked()) {
        writer.setPageLayout(QPageLayout(
            QPageSize(QPageSize::A4),
            QPageLayout::Portrait,
            QMarginsF(20, 20, 20, 20)
        ));
    }

    // 3. Painter
    QPainter painter(&writer);

    // 4. ЯВНО УСТАНАВЛИВАЕМ ОБЛАСТЬ РИСОВАНИЯ
    QRectF pageRect = writer.pageLayout().paintRect();
    QSizeF chartSize = m_chartView->size();

    // Масштабируем с отступами
    qreal margin = 0.9;
    qreal scaleX = (pageRect.width() * margin) / chartSize.width();
    qreal scaleY = (pageRect.height() * margin) / chartSize.height();
    qreal scale = qMin(scaleX, scaleY);

    // Вычисляем позицию для центрирования
    qreal offsetX = (pageRect.width() - chartSize.width() * scale) / 2;
    qreal offsetY = (pageRect.height() - chartSize.height() * scale) / 2;

    // 5. Применяем трансформацию
    painter.translate(offsetX, offsetY);
    painter.scale(scale, scale);

    // 6. Рендерим график
    m_chartView->render(&painter);

    painter.end();

    this->statusBar()->showMessage("PDF сохранён: " + filePath);
    qDebug() << "PDF saved to:" << filePath;
}

void MainWindow::showErrorMessage(const QString& title, const QString& message)
{
    QMessageBox::critical(this, title, message);
}
