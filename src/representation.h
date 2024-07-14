#pragma once

#include <QPoint>
#include <QVector>
#include <memory>
#include <variant>

class Shape {
public:
    enum ShapeType { LINE, RECTANGLE, POLYGON, CIRCLE, BEZIER_CURVE };

    Shape(ShapeType type, int zBufferPosition, bool isFilled, const QColor& borderColor, const QColor& fillingColor)
        : type(type), zBufferPosition(zBufferPosition), isFilled(isFilled), borderColor(borderColor), fillingColor(fillingColor) {}
    
    virtual ~Shape() {}

    ShapeType getType() const { return type; }
    int getZBufferPosition() const { return zBufferPosition; }
    bool getIsFilled() const { return isFilled; }
    QColor getBorderColor() const { return borderColor; }
    QColor getFillingColor() const { return fillingColor; }

    void setZBufferPosition(int zBufferPos) { zBufferPosition = zBufferPos; }
    void setBorderColor(const QColor& color) { borderColor = color; }
    void setFillingColor(const QColor& color) { fillingColor = color; }

    virtual QVector<QPoint> getPoints() { return { QPoint(), QPoint() }; }
    virtual void setPoints(const QVector<QPoint>& points) {}
    virtual void addPoint(QPoint point) {}

protected:
    ShapeType type;
    int zBufferPosition;
    bool isFilled;
    QColor borderColor;
    QColor fillingColor;
};

class Line : public Shape {
public:
    Line(const QPoint& p1, const QPoint& p2, int zBufferPosition, bool isFilled, const QColor& borderColor, const QColor& fillingColor)
        : Shape(Shape::LINE, zBufferPosition, isFilled, borderColor, fillingColor), p1(p1), p2(p2) {}

    ~Line() override {}

    QVector<QPoint> getPoints() override {
        return { p1, p2 };
    }

    void setPoints(const QVector<QPoint>& points) override {
        if (points.size() >= 2) {
            p1 = points[0];
            p2 = points[1];
        }
    }

private:
    QPoint p1, p2;
};

class MyRectangle : public Shape {
public:
    MyRectangle(const QPoint& p1, const QPoint& p2, const QPoint& p3, const QPoint& p4, int zBufferPosition, bool isFilled, const QColor& borderColor, const QColor& fillingColor)
        : Shape(Shape::RECTANGLE, zBufferPosition, isFilled, borderColor, fillingColor), p1(p1), p2(p2), p3(p3), p4(p4) {}

    ~MyRectangle() override {}

    QVector<QPoint> getPoints() override {
        return { p1, p2, p3, p4 };
    }

    void setPoints(const QVector<QPoint>& points) override {
        if (points.size() >= 2) {
            p1 = points[0];
            p2 = points[1];
            p3 = points[2];
            p4 = points[3];
        }
    }

private:
    QPoint p1, p2, p3, p4;
};

class MyPolygon : public Shape {
public:
    MyPolygon(const QVector<QPoint>& points, int zBufferPosition, bool isFilled, const QColor& borderColor, const QColor& fillingColor)
        : Shape(Shape::POLYGON, zBufferPosition, isFilled, borderColor, fillingColor), points(points) {}

    ~MyPolygon() override {}

    QVector<QPoint> getPoints() override {
        return points;
    }

    void setPoints(const QVector<QPoint>& newPoints) override {
        points = newPoints;
    }

    void addPoint(QPoint point) override {
        points.append(point);
    }

private:
    QVector<QPoint> points;
};

class Circle : public Shape {
public:
    Circle(const QPoint& center, const QPoint& edge, int zBufferPosition, bool isFilled, const QColor& borderColor, const QColor& fillingColor)
        : Shape(Shape::CIRCLE, zBufferPosition, isFilled, borderColor, fillingColor), center(center), edge(edge) {}

    ~Circle() override {}

    QVector<QPoint> getPoints() override {
        return { center, edge };
    }

    void setPoints(const QVector<QPoint>& points) override {
        if (points.size() >= 2) {
            center = points[0];
            edge = points[1];
        }
    }

private:
    QPoint center, edge;
};

class BezierCurve : public Shape {
public:
    BezierCurve(const QVector<QPoint>& controlPoints, int zBufferPosition, bool isFilled, const QColor& borderColor, const QColor& fillingColor)
        : Shape(Shape::BEZIER_CURVE, zBufferPosition, isFilled, borderColor, fillingColor), controlPoints(controlPoints) {}

    ~BezierCurve() override {}

    QVector<QPoint> getPoints() override {
        return controlPoints;
    }

    void setPoints(const QVector<QPoint>& points) override {
        controlPoints = points;
    }

    void addPoint(QPoint point) override {
        controlPoints.append(point);
    }

private:
    QVector<QPoint> controlPoints;
};
