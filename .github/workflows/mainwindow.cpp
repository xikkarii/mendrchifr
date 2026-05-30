#include "mainwindow.h"
#include "meanderwidget.h"
#include "crypto.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTime>
#include <QElapsedTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QByteArray>
#include <QString>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("MeandrCipher v1.0");
    resize(900, 800);
    buildUi();
    log("Программа запущена. Выберите порядок меандра и нажмите «Сгенерировать».");
}

void MainWindow::buildUi() {
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);

    // ── Параметры ──
    auto* paramsBox = new QGroupBox("Параметры", central);
    auto* paramsLay = new QHBoxLayout(paramsBox);

    paramsLay->addWidget(new QLabel("Порядок меандра (n):", paramsBox));
    m_orderSpin = new QSpinBox(paramsBox);
    m_orderSpin->setRange(1, 10);
    m_orderSpin->setValue(3);
    m_orderSpin->setFixedWidth(60);
    paramsLay->addWidget(m_orderSpin);

    auto* genBtn = new QPushButton("Сгенерировать", paramsBox);
    paramsLay->addWidget(genBtn);

    paramsLay->addSpacing(20);
    paramsLay->addWidget(new QLabel("Меандр №:", paramsBox));
    m_meanderCombo = new QComboBox(paramsBox);
    m_meanderCombo->setEnabled(false);
    m_meanderCombo->setFixedWidth(80);
    paramsLay->addWidget(m_meanderCombo);

    m_countLabel = new QLabel("", paramsBox);
    paramsLay->addWidget(m_countLabel);
    paramsLay->addStretch();

    root->addWidget(paramsBox);

    // ── Кнопки операций ──
    auto* btnLay = new QHBoxLayout();
    auto* encBtn   = new QPushButton("Зашифровать", central);
    auto* decBtn   = new QPushButton("Расшифровать", central);
    auto* loadBtn  = new QPushButton("Загрузить", central);
    auto* saveBtn  = new QPushButton("Сохранить", central);
    auto* keyBtn   = new QPushButton("Экспорт ключа", central);
    for (auto* b : {encBtn, decBtn, loadBtn, saveBtn, keyBtn})
        btnLay->addWidget(b);
    root->addLayout(btnLay);

    // ── Исходный текст ──
    auto* inBox = new QGroupBox("Исходный текст", central);
    auto* inLay = new QVBoxLayout(inBox);
    m_inputEdit = new QTextEdit(inBox);
    m_inputEdit->setMinimumHeight(90);
    inLay->addWidget(m_inputEdit);
    root->addWidget(inBox);

    // ── Результат ──
    auto* outBox = new QGroupBox("Результат", central);
    auto* outLay = new QVBoxLayout(outBox);
    m_outputEdit = new QTextEdit(outBox);
    m_outputEdit->setMinimumHeight(90);
    outLay->addWidget(m_outputEdit);
    root->addWidget(outBox);

    // ── Визуализация ──
    auto* visBox = new QGroupBox("Визуализация меандра", central);
    auto* visLay = new QVBoxLayout(visBox);
    m_meanderView = new MeanderWidget(visBox);
    visLay->addWidget(m_meanderView);
    root->addWidget(visBox);

    // ── Лог ──
    auto* logBox = new QGroupBox("Лог", central);
    auto* logLay = new QVBoxLayout(logBox);
    m_logEdit = new QTextEdit(logBox);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(110);
    logLay->addWidget(m_logEdit);
    root->addWidget(logBox);

    setCentralWidget(central);

    // ── Сигналы ──
    connect(genBtn,  &QPushButton::clicked, this, &MainWindow::onGenerate);
    connect(encBtn,  &QPushButton::clicked, this, &MainWindow::onEncrypt);
    connect(decBtn,  &QPushButton::clicked, this, &MainWindow::onDecrypt);
    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::onLoad);
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSave);
    connect(keyBtn,  &QPushButton::clicked, this, &MainWindow::onExportKey);
    connect(m_meanderCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onMeanderSelected);
}

void MainWindow::log(const QString& msg) {
    QString ts = QTime::currentTime().toString("HH:mm:ss");
    m_logEdit->append("[" + ts + "] " + msg);
}

void MainWindow::refreshKey() {
    m_currentKey = deriveKey(m_currentMeander);
    m_hasKey = true;
    Bytes keyBytes(m_currentKey.begin(), m_currentKey.end());
    log("Ключ (SHA-256): " + QString::fromStdString(toHex(keyBytes)));
}

void MainWindow::onGenerate() {
    int n = m_orderSpin->value();
    log(QString("Генерация меандров порядка %1…").arg(n));

    QElapsedTimer timer;
    timer.start();
    m_meanders = generateMeanders(n);
    double elapsed = timer.nsecsElapsed() / 1e9;

    m_meanderCombo->blockSignals(true);
    m_meanderCombo->clear();

    if (m_meanders.empty()) {
        m_meanderCombo->setEnabled(false);
        m_meanderCombo->blockSignals(false);
        m_countLabel->setText("");
        m_hasKey = false;
        m_meanderView->clear();
        log(QString("Меандров порядка %1 не найдено.").arg(n));
        return;
    }

    for (std::size_t i = 0; i < m_meanders.size(); ++i)
        m_meanderCombo->addItem(QString::number(i + 1));
    m_meanderCombo->setEnabled(true);
    m_meanderCombo->setCurrentIndex(0);
    m_meanderCombo->blockSignals(false);

    m_countLabel->setText(QString("(всего: %1)").arg(m_meanders.size()));

    m_currentMeander = m_meanders[0];
    m_meanderView->setMeander(m_currentMeander);

    QString trav;
    for (std::size_t i = 0; i < m_currentMeander.traversal.size(); ++i) {
        if (i) trav += " → ";
        trav += QString::number(m_currentMeander.traversal[i]);
    }
    log(QString("Найдено %1 меандр(ов) порядка %2 за %3 с.  Обход: %4")
            .arg(m_meanders.size()).arg(n).arg(elapsed, 0, 'f', 3).arg(trav));
    refreshKey();
}

void MainWindow::onMeanderSelected(int index) {
    if (index < 0 || index >= static_cast<int>(m_meanders.size()))
        return;
    m_currentMeander = m_meanders[index];
    m_meanderView->setMeander(m_currentMeander);

    QString trav;
    for (std::size_t i = 0; i < m_currentMeander.traversal.size(); ++i) {
        if (i) trav += " → ";
        trav += QString::number(m_currentMeander.traversal[i]);
    }
    log(QString("Выбран меандр #%1.  Обход: %2").arg(index + 1).arg(trav));
    refreshKey();
}

void MainWindow::onEncrypt() {
    if (!m_hasKey) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте меандр.");
        return;
    }
    QString text = m_inputEdit->toPlainText();
    if (text.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Введите текст для шифрования.");
        return;
    }

    QByteArray utf8 = text.toUtf8();
    Bytes plain(utf8.begin(), utf8.end());

    QElapsedTimer timer;
    timer.start();
    Bytes cipher = xorCipher(plain, m_currentKey);
    double elapsed = timer.nsecsElapsed() / 1e9;

    m_outputEdit->setPlainText(QString::fromStdString(toHex(cipher)));

    double entP = shannonEntropy(plain);
    double entC = shannonEntropy(cipher);
    log(QString("Зашифровано %1 байт за %2 с.  Энтропия: %3 → %4 бит/байт")
            .arg(plain.size()).arg(elapsed, 0, 'f', 4)
            .arg(entP, 0, 'f', 2).arg(entC, 0, 'f', 2));
}

void MainWindow::onDecrypt() {
    if (!m_hasKey) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте меандр.");
        return;
    }
    QString hexText = m_inputEdit->toPlainText().trimmed();
    if (hexText.isEmpty()) {
        QMessageBox::warning(this, "Внимание",
                             "Введите шифротекст (hex) для дешифрования.");
        return;
    }

    Bytes cipher;
    if (!fromHex(hexText.toStdString(), cipher)) {
        QMessageBox::critical(this, "Ошибка", "Некорректный hex-формат шифротекста.");
        return;
    }

    QElapsedTimer timer;
    timer.start();
    Bytes plain = xorCipher(cipher, m_currentKey);
    double elapsed = timer.nsecsElapsed() / 1e9;

    QByteArray ba(reinterpret_cast<const char*>(plain.data()),
                  static_cast<int>(plain.size()));
    QString result = QString::fromUtf8(ba);

    m_outputEdit->setPlainText(result);
    log(QString("Дешифровано %1 байт за %2 с.")
            .arg(cipher.size()).arg(elapsed, 0, 'f', 4));
}

void MainWindow::onLoad() {
    QString path = QFileDialog::getOpenFileName(
        this, "Загрузить файл", QString(),
        "Текстовые файлы (*.txt);;Зашифрованные файлы (*.enc);;Все файлы (*.*)");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (path.endsWith(".enc")) {
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл.");
            return;
        }
        QByteArray data = file.readAll();
        file.close();
        Bytes bytes(data.begin(), data.end());
        m_inputEdit->setPlainText(QString::fromStdString(toHex(bytes)));
        log(QString("Загружен бинарный файл: %1 (%2 байт)").arg(path).arg(data.size()));
    } else {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть файл.");
            return;
        }
        QByteArray data = file.readAll();
        file.close();
        m_inputEdit->setPlainText(QString::fromUtf8(data));
        log(QString("Загружен файл: %1").arg(path));
    }
}

void MainWindow::onSave() {
    QString content = m_outputEdit->toPlainText();
    if (content.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Нет данных для сохранения.");
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, "Сохранить", QString(),
        "Текстовые файлы (*.txt);;Зашифрованные файлы (*.enc)");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (path.endsWith(".enc")) {
        Bytes bytes;
        if (fromHex(content.toStdString(), bytes)) {
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл.");
                return;
            }
            file.write(reinterpret_cast<const char*>(bytes.data()),
                       static_cast<qint64>(bytes.size()));
        } else {
            if (!file.open(QIODevice::WriteOnly)) {
                QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл.");
                return;
            }
            file.write(content.toUtf8());
        }
    } else {
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл.");
            return;
        }
        file.write(content.toUtf8());
    }
    file.close();
    log(QString("Сохранено: %1").arg(path));
}

void MainWindow::onExportKey() {
    if (!m_hasKey) {
        QMessageBox::warning(this, "Внимание", "Сначала сгенерируйте меандр.");
        return;
    }

    QString path = QFileDialog::getSaveFileName(
        this, "Экспорт ключа", QString(),
        "JSON (*.json);;Текст (*.txt)");
    if (path.isEmpty())
        return;

    QJsonObject obj;
    obj["order"] = m_currentMeander.order;

    QJsonArray upperArr;
    for (const auto& [a, b] : m_currentMeander.upper) {
        QJsonArray pair; pair.append(a); pair.append(b);
        upperArr.append(pair);
    }
    obj["upper_matching"] = upperArr;

    QJsonArray lowerArr;
    for (const auto& [a, b] : m_currentMeander.lower) {
        QJsonArray pair; pair.append(a); pair.append(b);
        lowerArr.append(pair);
    }
    obj["lower_matching"] = lowerArr;

    QJsonArray travArr;
    for (int v : m_currentMeander.traversal)
        travArr.append(v);
    obj["traversal"] = travArr;

    Bytes keyBytes(m_currentKey.begin(), m_currentKey.end());
    obj["key_sha256_hex"] = QString::fromStdString(toHex(keyBytes));

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось сохранить файл.");
        return;
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
    file.close();
    log(QString("Ключ экспортирован: %1").arg(path));
}
