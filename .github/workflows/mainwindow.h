#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include <array>
#include <cstdint>
#include "meander.h"

class QSpinBox;
class QComboBox;
class QTextEdit;
class QLabel;
class MeanderWidget;

// Главное окно приложения МеандрШифр.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onGenerate();
    void onMeanderSelected(int index);
    void onEncrypt();
    void onDecrypt();
    void onLoad();
    void onSave();
    void onExportKey();

private:
    void buildUi();
    void log(const QString& msg);
    void refreshKey();

    QSpinBox*       m_orderSpin   = nullptr;
    QComboBox*      m_meanderCombo = nullptr;
    QLabel*         m_countLabel  = nullptr;
    QTextEdit*      m_inputEdit   = nullptr;
    QTextEdit*      m_outputEdit  = nullptr;
    QTextEdit*      m_logEdit     = nullptr;
    MeanderWidget*  m_meanderView = nullptr;

    std::vector<Meander>        m_meanders;
    Meander                     m_currentMeander;
    std::array<uint8_t, 32>     m_currentKey{};
    bool                        m_hasKey = false;
};

#endif // MAINWINDOW_H
