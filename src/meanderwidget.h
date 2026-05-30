#ifndef MEANDERWIDGET_H
#define MEANDERWIDGET_H

#include <QWidget>
#include "meander.h"

// Виджет графического отображения меандра средствами QPainter.
// Рисует горизонтальную прямую, верхние (синие) и нижние (красные) дуги,
// точки пересечения с номерами и подпись последовательности обхода.
class MeanderWidget : public QWidget {
    Q_OBJECT
public:
    explicit MeanderWidget(QWidget* parent = nullptr);

    void setMeander(const Meander& m);
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    Meander m_meander;
    bool m_hasMeander = false;
};

#endif // MEANDERWIDGET_H
