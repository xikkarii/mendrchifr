#include "meanderwidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QString>
#include <algorithm>
#include <cmath>

MeanderWidget::MeanderWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(180);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);
}

void MeanderWidget::setMeander(const Meander& m) {
    m_meander = m;
    m_hasMeander = true;
    update();
}

void MeanderWidget::clear() {
    m_hasMeander = false;
    update();
}

void MeanderWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), Qt::white);

    const int W = width();
    const int H = height();

    if (!m_hasMeander) {
        p.setPen(QColor(0xBD, 0xBD, 0xBD));
        QFont f = p.font();
        f.setPointSize(13);
        p.setFont(f);
        p.drawText(rect(), Qt::AlignCenter, "Меандр не сгенерирован");
        return;
    }

    const int n = m_meander.order;
    const int num = 2 * n;
    const int margin = 50;
    const int yMid = H / 2;

    const double spacing =
        (num > 1) ? static_cast<double>(W - 2 * margin) / (num - 1) : 0.0;

    // Позиции точек пересечения по оси X (точки нумеруются с 1).
    std::vector<double> pos(num + 1, 0.0);
    for (int i = 0; i < num; ++i)
        pos[i + 1] = margin + i * spacing;

    const double maxSpan = std::max(W - 2 * margin, 1);
    const double maxH = std::min(yMid - 20.0, 110.0);

    // Горизонтальная прямая (пунктир).
    QPen linePen(QColor(0xBD, 0xBD, 0xBD));
    linePen.setWidth(2);
    linePen.setStyle(Qt::DashLine);
    p.setPen(linePen);
    p.drawLine(margin - 20, yMid, W - margin + 20, yMid);

    // Вспомогательная лямбда рисования полудуги-эллипса.
    auto drawArc = [&](const Arc& arc, bool upper) {
        double x1 = pos[std::min(arc.first, arc.second)];
        double x2 = pos[std::max(arc.first, arc.second)];
        double span = x2 - x1;
        double ah = std::max(18.0, maxH * span / maxSpan);
        QRectF box(x1, yMid - ah, span, 2 * ah);
        // В Qt угол задаётся в 1/16 градуса; верхняя дуга 0..180, нижняя 180..180.
        int startAngle = upper ? 0 : 180 * 16;
        int spanAngle = 180 * 16;
        p.drawArc(box, startAngle, spanAngle);
    };

    // Верхние дуги (синие).
    QPen upperPen(QColor(0x15, 0x65, 0xC0));
    upperPen.setWidth(2);
    p.setPen(upperPen);
    for (const auto& arc : m_meander.upper)
        drawArc(arc, true);

    // Нижние дуги (красные).
    QPen lowerPen(QColor(0xC6, 0x28, 0x28));
    lowerPen.setWidth(2);
    p.setPen(lowerPen);
    for (const auto& arc : m_meander.lower)
        drawArc(arc, false);

    // Точки пересечения и их номера.
    QFont nf = p.font();
    nf.setPointSize(9);
    nf.setBold(true);
    p.setFont(nf);
    for (int i = 0; i < num; ++i) {
        double x = pos[i + 1];
        p.setBrush(QColor(0x33, 0x33, 0x33));
        p.setPen(Qt::NoPen);
        p.drawEllipse(QPointF(x, yMid), 5, 5);
        p.setPen(QColor(0x33, 0x33, 0x33));
        p.drawText(QRectF(x - 15, yMid + 8, 30, 16),
                   Qt::AlignCenter, QString::number(i + 1));
    }

    // Подпись последовательности обхода.
    QString trav;
    for (std::size_t i = 0; i < m_meander.traversal.size(); ++i) {
        if (i) trav += " → ";
        trav += QString::number(m_meander.traversal[i]);
    }
    QFont tf = p.font();
    tf.setBold(false);
    tf.setPointSize(9);
    p.setFont(tf);
    p.setPen(QColor(0x55, 0x55, 0x55));
    p.drawText(QRectF(0, H - 22, W, 18), Qt::AlignCenter, "Обход: " + trav);
}
