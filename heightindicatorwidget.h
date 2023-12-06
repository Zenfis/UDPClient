#ifndef HEIGHTINDICATORWIDGET_H
#define HEIGHTINDICATORWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPixmap>

class HeightIndicatorWidget : public QWidget
{
    Q_OBJECT

public:
    HeightIndicatorWidget(QWidget* parent = nullptr);

    void setHeight(int height);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    int m_height = 0;
    QSize referenceSize;
};

#endif // HEIGHTINDICATORWIDGET_H
