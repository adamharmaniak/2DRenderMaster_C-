#include   "ViewerWidget.h"

ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete painter;
	delete img;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//-----------------------------------------
//		*** Image functions ***
//-----------------------------------------

bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img != nullptr) {
		delete painter;
		delete img;
	}
	img = new QImage(inputImg);
	if (!img) {
		return false;
	}
	resizeWidget(img->size());
	setPainter();
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete painter;
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setPainter();
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::clear()
{
	img->fill(Qt::white);
	update();
}

void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}

//-----------------------------------------
//		*** Point drawing functions ***
//-----------------------------------------

void ViewerWidget::setPixel(int x, int y, uchar r, uchar g, uchar b, uchar a)
{
	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = b;
	data[startbyte + 1] = g;
	data[startbyte + 2] = r;
	data[startbyte + 3] = a;
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(255 * valB);
	data[startbyte + 1] = static_cast<uchar>(255 * valG);
	data[startbyte + 2] = static_cast<uchar>(255 * valR);
	data[startbyte + 3] = static_cast<uchar>(255 * valA);
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (!color.isValid() || x < 0 || y < 0 || x >= img->width() || y >= img->height()) {
		return;
	}

	size_t startbyte = y * img->bytesPerLine() + x * 4;

	data[startbyte] = color.blue();
	data[startbyte + 1] = color.green();
	data[startbyte + 2] = color.red();
	data[startbyte + 3] = color.alpha();
}

//-----------------------------------------
//		*** Drawing functions ***
//-----------------------------------------
void ViewerWidget::changeLayerColor(int zBufferPosition, const QColor& newBorderColor, const QColor& newFillingColor) {
	for (auto& pair : zBuffer) {
		if (pair.second == zBufferPosition) {
			Shape& shape = pair.first.get();

			shape.setBorderColor(newBorderColor);
			shape.setFillingColor(newFillingColor);

			switch (shape.getType()) {
			case Shape::LINE:
				drawLine(static_cast<Line&>(shape));
				break;
			case Shape::RECTANGLE:
				drawRectangle(static_cast<MyRectangle&>(shape));
				break;
			case Shape::POLYGON:
				drawPolygon(static_cast<MyPolygon&>(shape));
				break;
			case Shape::CIRCLE:
				drawCircle(static_cast<Circle&>(shape));
				break;
			case Shape::BEZIER_CURVE:
				drawCurve(static_cast<BezierCurve&>(shape));
				break;
			default:
				break;
			}

			update();
			break;
		}
	}
}

void ViewerWidget::drawShape(Shape& shape) {
	switch (shape.getType()) {
	case Shape::LINE: {
		Line& line = static_cast<Line&>(shape);
		drawLine(line);
		break;
	}
	case Shape::RECTANGLE: {
		MyRectangle& rectangle = static_cast<MyRectangle&>(shape);
		drawRectangle(rectangle);
		break;
	}
	case Shape::POLYGON: {
		MyPolygon& polygon = static_cast<MyPolygon&>(shape);
		drawPolygon(polygon);
		break;
	}
	case Shape::CIRCLE: {
		Circle& circle = static_cast<Circle&>(shape);
		drawCircle(circle);
		break;
	}
	case Shape::BEZIER_CURVE: {
		BezierCurve& curve = static_cast<BezierCurve&>(shape);
		drawCurve(curve);
		break;
	}
	default:
		break;
	}
}

void ViewerWidget::addToZBuffer(Shape& shape, int depth) {
	zBuffer.push_back(std::make_pair(std::ref(shape), depth));
	std::sort(zBuffer.begin(), zBuffer.end(), [](const std::pair<std::reference_wrapper<Shape>, int>& a, const std::pair<std::reference_wrapper<Shape>, int>& b) {
		return a.second < b.second;
		});
}

void ViewerWidget::deleteObjectFromZBuffer(int currentIndex) {
	if (currentIndex >= 0 && currentIndex < zBuffer.size()) {
		zBuffer.erase(zBuffer.begin() + currentIndex);
	}
}

void ViewerWidget::moveShapeUp(int zBufferPosition) {
	auto it = std::find_if(zBuffer.begin(), zBuffer.end(), [zBufferPosition](const auto& pair) {
		return pair.second == zBufferPosition;
		});

	if (it != zBuffer.end() && it != zBuffer.begin()) {
		auto prevIt = std::prev(it);
		std::iter_swap(it, prevIt);
		std::swap(it->second, prevIt->second);
		redrawAllShapes();
	}
}

void ViewerWidget::moveShapeDown(int zBufferPosition) {
	auto it = std::find_if(zBuffer.begin(), zBuffer.end(), [zBufferPosition](const auto& pair) {
		return pair.second == zBufferPosition;
		});

	if (it != zBuffer.end() && (it + 1) != zBuffer.end()) {
		auto nextIt = std::next(it);
		std::iter_swap(it, nextIt);
		std::swap(it->second, nextIt->second);
		redrawAllShapes();
	}
}

void ViewerWidget::redrawAllShapes() {
	clear();
	for (auto& shapePair : zBuffer) {
		drawShape(shapePair.first.get());
	}
	update();
}

void ViewerWidget::saveCurrentImageState() {
	QString filePath = QFileDialog::getSaveFileName(this, "Save Image State", "C:\\Pocitacova_grafika_projects\\ImageViewer_projekt_zaverecny", "CSV Files (*.csv)");
	if (filePath.isEmpty()) {
		return;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, "File Error", "Unable to open file for writing.");
		return;
	}

	QTextStream out(&file);

	out << "ShapeType,ZBufferPosition,IsFilled,BorderColor,FillingColor,Points\n";

	for (const auto& pair : zBuffer) {
		Shape& shape = pair.first.get();
		int zBufferPosition = pair.second;
		QString shapeType;
		switch (shape.getType()) {
		case Shape::LINE:
			shapeType = "Line";
			break;
		case Shape::RECTANGLE:
			shapeType = "Rectangle";
			break;
		case Shape::POLYGON:
			shapeType = "Polygon";
			break;
		case Shape::CIRCLE:
			shapeType = "Circle";
			break;
		case Shape::BEZIER_CURVE:
			shapeType = "BezierCurve";
			break;
		}

		QString borderColor = shape.getBorderColor().name();
		QString fillingColor = shape.getFillingColor().name();
		QString isFilled = shape.getIsFilled() ? "true" : "false";

		QString points;
		QVector<QPoint> shapePoints = shape.getPoints();
		for (const QPoint& point : shapePoints) {
			points += QString("(%1,%2) ").arg(point.x()).arg(point.y());
		}
		points = points.trimmed();

		out << shapeType << "," << zBufferPosition << "," << isFilled << "," << borderColor << "," << fillingColor << "," << points << "\n";
	}

	file.close();

	QMessageBox::information(this, "Save Successful", "The current state has been saved successfully.");
}

//-----------------------------------------
//		*** Line functions ***
//-----------------------------------------
void ViewerWidget::drawLine(Line& line)
{
	borderColor = line.getBorderColor();
	painter->setPen(QPen(borderColor));

	QVector<QPoint> linePoints = line.getPoints();

	QVector<QPoint> lineToClip = line.getPoints();

	clipLineWithPolygon(lineToClip);

	if (lineToClip.size() == 2) {
		linePoints.append(lineToClip[0]);
		linePoints.append(lineToClip[1]);
	}
	else {
		linePoints.append(line.getPoints()[0]);
		linePoints.append(line.getPoints()[1]);
	}

	for (const QPoint& point : linePoints) {
		qDebug() << point;
	}

	drawLineBresenham(linePoints);
	line.setPoints(linePoints);
	update();
}

void ViewerWidget::clipLineWithPolygon(QVector<QPoint> linePoints) {
	if (linePoints.size() < 2) {
		return;
	}

	QVector<QPoint> clippedPoints;
	QPoint P1 = linePoints[0], P2 = linePoints[1];
	double t_min = 0, t_max = 1;
	QPoint d = P2 - P1;

	QVector<QPoint> E = { QPoint(0,0), QPoint(500,0), QPoint(500,500), QPoint(0,500) };

	for (int i = 0; i < E.size(); i++) {
		QPoint E1 = E[i];
		QPoint E2 = E[(i + 1) % E.size()];

		QPoint normal = QPoint(E2.y() - E1.y(), E1.x() - E2.x());

		QPoint w = P1 - E1;

		double dn = d.x() * normal.x() + d.y() * normal.y();
		double wn = w.x() * normal.x() + w.y() * normal.y();
		if (dn != 0) {
			double t = -wn / dn;
			if (dn > 0 && t <= 1) {
				t_min = std::max(t, t_min);
			}
			else if (dn < 0 && t >= 0) {
				t_max = std::min(t, t_max);
			}
		}
	}


	if (t_min < t_max) {
		QPoint clippedP1 = P1 + (P2 - P1) * t_min;
		QPoint clippedP2 = P1 + (P2 - P1) * t_max;

		clippedPoints.push_back(clippedP1);
		clippedPoints.push_back(clippedP2);
	}
	else {
		//qDebug() << "Useckovy segment je uplne mimo orezovacej oblasti alebo je neplatny.";
	}

	if (!clippedPoints.isEmpty()) {
		linePoints = clippedPoints;
	}
}

void ViewerWidget::drawLineBresenham(QVector<QPoint>& linePoints) {
	int p, k1, k2;
	int dx = linePoints.last().x() - linePoints.first().x();
	int dy = linePoints.last().y() - linePoints.first().y();

	int adx = abs(dx);
	int ady = abs(dy);

	int x = linePoints.first().x();
	int y = linePoints.first().y();

	int incrementX = (dx > 0) ? 1 : -1;
	int incrementY = (dy > 0) ? 1 : -1;

	if (adx > ady) {
		p = 2 * ady - adx;
		k1 = 2 * ady;
		k2 = 2 * (ady - adx);

		while (x != linePoints.last().x()) {
			setPixel(x, y, borderColor);
			x += incrementX;
			if (p >= 0) {
				y += incrementY;
				p += k2;
			}
			else {
				p += k1;
			}
		}
	}
	else {
		p = 2 * adx - ady;
		k1 = 2 * adx;
		k2 = 2 * (adx - ady);

		while (y != linePoints.last().y()) {
			setPixel(x, y, borderColor);
			y += incrementY;
			if (p >= 0) {
				x += incrementX;
				p += k2;
			}
			else {
				p += k1;
			}
		}
	}

	setPixel(linePoints.last().x(), linePoints.last().y(), borderColor);
}

void ViewerWidget::moveLine(const QPoint& offset) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		qDebug() << "ShapeType: " << pair.first.get().getType();
		if (pair.first.get().getType() == Shape::LINE) {
			Line& line = static_cast<Line&>(pair.first.get());
			QVector<QPoint> points = line.getPoints();

			for (QPoint& point : points) {
				point += offset;
			}

			pair.first.get().setPoints(points);
			redrawAllShapes();
		}
	}
}

void ViewerWidget::turnLine(int angle) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::LINE) {
			Line& line = static_cast<Line&>(pair.first.get());
			QVector<QPoint> points = line.getPoints();
			QPoint center = getLineCenter(line);

			double radians = qDegreesToRadians(static_cast<double>(angle));
			double cosAngle = std::cos(radians);
			double sinAngle = std::sin(radians);

			QVector<QPoint> rotatedPoints;

			for (QPoint& point : points) {
				int translatedX = point.x() - center.x();
				int translatedY = point.y() - center.y();

				int rotatedX = static_cast<int>(translatedX * cosAngle - translatedY * sinAngle);
				int rotatedY = static_cast<int>(translatedX * sinAngle + translatedY * cosAngle);

				rotatedX += center.x();
				rotatedY += center.y();

				rotatedPoints.push_back(QPoint(rotatedX, rotatedY));
			}

			pair.first.get().setPoints(rotatedPoints);
			redrawAllShapes();
		}
	}
}

QPoint ViewerWidget::getLineCenter(Line& line) const {
	QVector<QPoint> points = line.getPoints();
	if (points.isEmpty()) {
		return QPoint();
	}

	double centroidX = (points.first().x() + points.last().x()) / 2;
	double centroidY = (points.first().y() + points.last().y()) / 2;

	return QPoint(static_cast<int>(centroidX), static_cast<int>(centroidY));
}

void ViewerWidget::scaleLine(double scaleX, double scaleY) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::LINE) {
			Line& line = static_cast<Line&>(pair.first.get());
			QVector<QPoint> points = line.getPoints();
			QPoint center = getLineCenter(line);

			QVector<QPoint> scaledPoints;

			for (QPoint& point : points) {
				double newX = center.x() + (point.x() - center.x()) * scaleX;
				double newY = center.y() + (point.y() - center.y()) * scaleY;

				scaledPoints.append(QPoint(static_cast<int>(std::round(newX)), static_cast<int>(std::round(newY))));
			}

			pair.first.get().setPoints(scaledPoints);
			redrawAllShapes();
		}
	}
}

//-----------------------------------------
//		*** Circle functions ***
//-----------------------------------------
void ViewerWidget::drawCircle(Circle& circle) {
	borderColor = circle.getBorderColor();
	fillingColor = circle.getFillingColor();
	QPoint center = circle.getPoints()[0];
	QPoint radiusPoint = circle.getPoints()[1];
	int r = std::sqrt(std::pow(radiusPoint.x() - center.x(), 2) + std::pow(radiusPoint.y() - center.y(), 2));
	int x = 0;
	int y = r;
	int p = 1 - r;

	if (circle.getIsFilled()) {
		drawSymmetricPoints(center, x, y);
		drawSymmetricPointsFilled(center, x, y);
	}
	else {
		drawSymmetricPoints(center, x, y);
	}

	while (x < y) {
		x++;
		if (p < 0) {
			p += 2 * x + 1;
		}
		else {
			y--;
			p += 2 * (x - y) + 1;
		}

		if (circle.getIsFilled()) {
			drawSymmetricPoints(center, x, y);
			drawSymmetricPointsFilled(center, x, y);
		}
		else {
			drawSymmetricPoints(center, x, y);
		}
	}

	update();
}

void ViewerWidget::drawSymmetricPoints(const QPoint& center, int x, int y) {
	QPoint points[8] = {
		QPoint(x, y),
		QPoint(y, x),
		QPoint(-x, y),
		QPoint(-y, x),
		QPoint(-x, -y),
		QPoint(-y, -x),
		QPoint(x, -y),
		QPoint(y, -x)
	};

	for (auto& point : points) {
		setPixel(center.x() + point.x(), center.y() + point.y(), borderColor);
	}
}

void ViewerWidget::drawSymmetricPointsFilled(const QPoint& center, int x, int y) {
	for (int i = -x; i <= x; i++) {
		setPixel(center.x() + i, center.y() + y, fillingColor);
		setPixel(center.x() + i, center.y() - y, fillingColor);
	}
	for (int i = -y; i <= y; i++) {
		setPixel(center.x() + i, center.y() + x, fillingColor);
		setPixel(center.x() + i, center.y() - x, fillingColor);
	}
}

void ViewerWidget::moveCircle(const QPoint& offset) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::CIRCLE) {
			Circle& circle = static_cast<Circle&>(pair.first.get());
			QVector<QPoint> points = circle.getPoints();

			for (QPoint& point : points) {
				point += offset;
			}

			pair.first.get().setPoints(points);
			redrawAllShapes();
		}
	}
}

void ViewerWidget::scaleCircle(double scaleX, double scaleY) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::CIRCLE) {
			Circle& circle = static_cast<Circle&>(pair.first.get());
			QVector<QPoint> points = circle.getPoints();
			QPoint center = points[0];
			QPoint radiusPoint = points[1];

			int newX = center.x() + static_cast<int>((radiusPoint.x() - center.x()) * scaleX);
			int newY = center.y() + static_cast<int>((radiusPoint.y() - center.y()) * scaleY);
			points[1] = QPoint(newX, newY);

			pair.first.get().setPoints(points);
			redrawAllShapes();
		}
	}
}


//-----------------------------------------
//		*** Polygon Functions ***
//-----------------------------------------
void ViewerWidget::drawPolygon(MyPolygon& polygon) {
	borderColor = polygon.getBorderColor();
	fillingColor = polygon.getFillingColor();
	const QVector<QPoint>& pointsVector = polygon.getPoints();

	if (pointsVector.size() < 2) {
		QMessageBox::warning(this, "Nizky pocet bodov", "Nebol dosiahnuty minimalny pocet bodov pre vykreslenie polygonu.");
		return;
	}

	painter->setPen(QPen(borderColor));
	QVector<QPoint> polygonPoints = pointsVector;

	bool allPointsOutside = std::all_of(pointsVector.begin(), pointsVector.end(), [this](const QPoint& point) {
		return !isInside(point);
		});

	if (allPointsOutside) {
		qDebug() << "Polygon je mimo hranicu.";
		return;
	}

	for (QPoint point : pointsVector) {
		if (!isInside(point)) {
			polygonPoints = trimPolygon(polygon);
			break;
		}
	}

	if (polygon.getIsFilled()) {
		fillPolygon(polygon);
	}
	painter->setPen(QPen(borderColor));

	std::vector<Line> lines;
	if (!polygonPoints.isEmpty()) {
		for (int i = 0; i < polygonPoints.size() - 1; i++) {
			lines.emplace_back(polygonPoints.at(i), polygonPoints.at(i + 1), polygon.getZBufferPosition(), polygon.getIsFilled(), borderColor, fillingColor);
		}
		lines.emplace_back(polygonPoints.last(), polygonPoints.first(), polygon.getZBufferPosition(), polygon.getIsFilled(), borderColor, fillingColor);
	}

	for (Line& line : lines) {
		drawLine(line);
	}

	update();
}

QPoint ViewerWidget::getPolygonCenter(Shape& polygon) const {
	const QVector<QPoint>& points = polygon.getPoints();
	if (points.isEmpty()) {
		return QPoint();
	}

	double centroidX = 0;
	double centroidY = 0;
	for (const QPoint& point : points) {
		centroidX += point.x();
		centroidY += point.y();
	}

	centroidX /= points.size();
	centroidY /= points.size();

	return QPoint(static_cast<int>(centroidX), static_cast<int>(centroidY));
}

void ViewerWidget::scalePolygon(double scaleX, double scaleY) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::POLYGON) {
			MyPolygon& polygon = static_cast<MyPolygon&>(pair.first.get());
			const QVector<QPoint>& points = polygon.getPoints();
			QPoint center = getPolygonCenter(polygon);

			QVector<QPoint> scaledPoints;
			for (const QPoint& point : points) {
				double newX = center.x() + (point.x() - center.x()) * scaleX;
				double newY = center.y() + (point.y() - center.y()) * scaleY;

				scaledPoints.append(QPoint(static_cast<int>(std::round(newX)), static_cast<int>(std::round(newY))));
			}

			pair.first.get().setPoints(scaledPoints);
			redrawAllShapes();
		}
	}
}

void ViewerWidget::movePolygon(const QPoint& offset) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		qDebug() << "ShapeType: " << pair.first.get().getType();
		if (pair.first.get().getType() == Shape::POLYGON) {
			MyPolygon& polygon = static_cast<MyPolygon&>(pair.first.get());
			const QVector<QPoint>& points = polygon.getPoints();

			QVector<QPoint> movedPoints;
			for (const QPoint& point : points) {
				movedPoints.append(point + offset);
			}

			pair.first.get().setPoints(movedPoints);
			redrawAllShapes();
		}
	}
}

void ViewerWidget::turnPolygon(int angle) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::POLYGON) {
			MyPolygon& polygon = static_cast<MyPolygon&>(pair.first.get());
			const QVector<QPoint>& points = polygon.getPoints();
			QPoint center = getPolygonCenter(polygon);

			double radians = qDegreesToRadians(static_cast<double>(angle));
			double cosAngle = std::cos(radians);
			double sinAngle = std::sin(radians);

			QVector<QPoint> rotatedPoints;

			for (const QPoint& point : points) {
				int translatedX = point.x() - center.x();
				int translatedY = point.y() - center.y();

				int rotatedX = static_cast<int>(translatedX * cosAngle - translatedY * sinAngle);
				int rotatedY = static_cast<int>(translatedX * sinAngle + translatedY * cosAngle);

				rotatedX += center.x();
				rotatedY += center.y();

				rotatedPoints.append(QPoint(rotatedX, rotatedY));
			}

			pair.first.get().setPoints(rotatedPoints);
			redrawAllShapes();
		}
	}
}

QVector<QPoint> ViewerWidget::trimPolygon(Shape& polygon) {
	QVector<QPoint> pointsVector = polygon.getPoints();

	if (pointsVector.isEmpty()) {
		qDebug() << "pointsVector je prazdny";
		return QVector<QPoint>();
	}

	QVector<QPoint> W, polygonPoints = pointsVector;
	QPoint S;

	int xMin[] = { 0,0,-499,-499 };

	for (int i = 0; i < 4; i++) {
		if (pointsVector.size() == 0) {
			return polygonPoints;
		}

		S = polygonPoints[polygonPoints.size() - 1];

		for (int j = 0; j < polygonPoints.size(); j++) {
			if (polygonPoints[j].x() >= xMin[i]) {
				if (S.x() >= xMin[i]) {
					W.push_back(polygonPoints[j]);
				}
				else {
					QPoint P(xMin[i], S.y() + (xMin[i] - S.x()) * ((polygonPoints[j].y() - S.y()) / static_cast<double>((polygonPoints[j].x() - S.x()))));
					W.push_back(P);
					W.push_back(polygonPoints[j]);
				}
			}
			else {
				if (S.x() >= xMin[i]) {
					QPoint P(xMin[i], S.y() + (xMin[i] - S.x()) * ((polygonPoints[j].y() - S.y()) / static_cast<double>((polygonPoints[j].x() - S.x()))));
					W.push_back(P);
				}
			}
			S = polygonPoints[j];
		}
		polygonPoints = W;
		W.clear();

		for (int j = 0; j < polygonPoints.size(); j++) {
			QPoint swappingPoint = polygonPoints[j];
			polygonPoints[j].setX(swappingPoint.y());
			polygonPoints[j].setY(-swappingPoint.x());
		}
	}

	return polygonPoints;
}

QVector<ViewerWidget::Edge> ViewerWidget::loadEdges(const QVector<QPoint>& points) {
	QVector<Edge> edges;

	for (int i = 0; i < points.size(); i++) {
		QPoint startPoint = points[i];
		QPoint endPoint = points[(i + 1) % points.size()];

		Edge edge(startPoint, endPoint);
		edge.adjustEndPoint();

		edges.push_back(edge);
	}

	std::sort(edges.begin(), edges.end(), compareByY);
	return edges;
}

void ViewerWidget::fillPolygon(Shape& polygon) {
	const QVector<QPoint>& points = polygon.getPoints();

	if (points.isEmpty()) {
		//qDebug() << "Neobsahuje body pre vyplnanie.";
		return;
	}
	QVector<Edge> edges = loadEdges(points);
	if (edges.isEmpty()) {
		//qDebug() << "Vektor hran je prazdny.";
		return;
	}

	int yMin = edges.front().startPoint().y();
	int yMax = edges.front().endPoint().y();

	for (const Edge& edge : edges) {
		int y1 = edge.startPoint().y();
		int y2 = edge.endPoint().y();
		yMin = qMin(yMin, qMin(y1, y2));
		yMax = qMax(yMax, qMax(y1, y2));
	}

	if (yMin >= yMax) {
		//qDebug() << "Neplatne yMin a yMax hodnoty. Mozne nespravne nastavenie hrany.";
		return;
	}
	QVector<QVector<Edge>> TH(yMax - yMin + 1);

	for (const auto& edge : edges) {
		int index = edge.startPoint().y() - yMin;
		if (index < 0 || index >= TH.size()) {
			//qDebug() << "Invalid index:" << index << "for edge start point y:" << edge.startPoint().y();
			continue;
		}
		TH[index].append(edge);
	}

	QVector<Edge> activeEdgeList;

	for (int y = yMin; y <= yMax; y++) {
		for (const auto& edge : TH[y - yMin]) {
			activeEdgeList.append(edge);
		}

		std::sort(activeEdgeList.begin(), activeEdgeList.end(), [](const Edge& a, const Edge& b) {
			return a.x() < b.x();
			});

		for (int i = 0; i < activeEdgeList.size(); i += 2) {
			if (i + 1 < activeEdgeList.size()) {
				int startX = qRound(activeEdgeList[i].x());
				int endX = qRound(activeEdgeList[i + 1].x());
				for (int x = startX; x <= endX; x++) {
					setPixel(x, y, fillingColor);
				}
			}
		}
		
		QMutableVectorIterator<Edge> it(activeEdgeList);
		while (it.hasNext()) {
			Edge& edge = it.next();
			if (edge.endPoint().y() == y) {
				it.remove();
			}
			else {
				edge.setX(edge.x() + edge.w());
			}
		}
	}
}

//-----------------------------------------
//		*** Curve functions ***
//-----------------------------------------

void ViewerWidget::drawCurve(BezierCurve& curve) {
	// << Bezierova krivka >>
	borderColor = curve.getBorderColor();
	fillingColor = curve.getFillingColor();
	const QVector<QPoint>& curvePoints = curve.getPoints();
	if (curvePoints.size() < 2) {
		QMessageBox::warning(this, "Nedostatocny pocet bodov", "Nemozno nakreslit krivku s menej ako dvomi riadiacimi bodmi.", QMessageBox::Ok);
		return;
	}

	painter->setPen(QPen(borderColor));
	float deltaT = 0.01f;
	QPoint Q0 = curvePoints[0];

	std::vector<Line> lines;

	for (float t = deltaT; t <= 1; t += deltaT) {
		QVector<QPoint> tempPoints = curvePoints;

		for (int i = 1; i < tempPoints.size(); i++) {
			for (int j = 0; j < tempPoints.size() - i; j++) {
				tempPoints[j] = tempPoints[j] * (1 - t) + tempPoints[j + 1] * t;
			}
		}

		lines.emplace_back(Q0, tempPoints[0], curve.getZBufferPosition(), curve.getIsFilled(), borderColor, fillingColor);
		Q0 = tempPoints[0];
	}

	if (deltaT * floor(1 / deltaT) < 1) {
		lines.emplace_back(Q0, curvePoints.last(), curve.getZBufferPosition(), curve.getIsFilled(), borderColor, fillingColor);
	}

	for (Line& line : lines) {
		drawLine(line);
	}
}

void ViewerWidget::moveCurve(const QPoint& offset) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::BEZIER_CURVE) {
			BezierCurve& curve = static_cast<BezierCurve&>(pair.first.get());
			const QVector<QPoint>& points = curve.getPoints();

			QVector<QPoint> movedPoints;
			for (const QPoint& point : points) {
				movedPoints.append(point + offset);
			}

			pair.first.get().setPoints(movedPoints);
			redrawAllShapes();
		}
	}
}

void ViewerWidget::scaleCurve(double scaleX, double scaleY) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::BEZIER_CURVE) {
			BezierCurve& curve = static_cast<BezierCurve&>(pair.first.get());
			const QVector<QPoint>& points = curve.getPoints();
			QPoint center = calculateCurveCenter(curve);

			QVector<QPoint> scaledPoints;
			for (const QPoint& point : points) {
				double newX = center.x() + (point.x() - center.x()) * scaleX;
				double newY = center.y() + (point.y() - center.y()) * scaleY;

				scaledPoints.append(QPoint(static_cast<int>(std::round(newX)), static_cast<int>(std::round(newY))));
			}

			pair.first.get().setPoints(scaledPoints);
			redrawAllShapes();
		}
	}
}

void ViewerWidget::turnCurve(int angle) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::BEZIER_CURVE) {
			BezierCurve& curve = static_cast<BezierCurve&>(pair.first.get());
			const QVector<QPoint>& points = curve.getPoints();
			QPoint center = calculateCurveCenter(curve);

			double radians = qDegreesToRadians(static_cast<double>(angle));
			double cosAngle = std::cos(radians);
			double sinAngle = std::sin(radians);

			QVector<QPoint> rotatedPoints;

			for (const QPoint& point : points) {
				int translatedX = point.x() - center.x();
				int translatedY = point.y() - center.y();

				int rotatedX = static_cast<int>(translatedX * cosAngle - translatedY * sinAngle);
				int rotatedY = static_cast<int>(translatedX * sinAngle + translatedY * cosAngle);

				rotatedX += center.x();
				rotatedY += center.y();

				rotatedPoints.append(QPoint(rotatedX, rotatedY));
			}

			pair.first.get().setPoints(rotatedPoints);
			redrawAllShapes();
		}
	}
}

QPoint ViewerWidget::calculateCurveCenter(BezierCurve& curve) const {
	const QVector<QPoint>& points = curve.getPoints();
	if (points.isEmpty()) {
		return QPoint();
	}

	double centroidX = 0;
	double centroidY = 0;
	for (const QPoint& point : points) {
		centroidX += point.x();
		centroidY += point.y();
	}

	centroidX /= points.size();
	centroidY /= points.size();

	return QPoint(static_cast<int>(centroidX), static_cast<int>(centroidY));
}

//-----------------------------------------
//		*** Rectangle functions ***
//-----------------------------------------

void ViewerWidget::drawRectangle(MyRectangle& rectangle) {
	borderColor = rectangle.getBorderColor();
	fillingColor = rectangle.getFillingColor();
	const QVector<QPoint>& pointsVector = rectangle.getPoints();

	if (pointsVector.size() < 2) {
		QMessageBox::warning(this, "Insufficient Points", "Not enough points to render the rectangle.");
		return;
	}

	painter->setPen(QPen(borderColor));

	QVector<QPoint> rectanglePoints = rectangle.getPoints();

	bool allPointsOutside = std::all_of(pointsVector.begin(), pointsVector.end(), [this](const QPoint& point) {
		return !isInside(point);
		});

	if (allPointsOutside) {
		qDebug() << "Rectangle is outside the boundary.";
		return;
	}

	for (const QPoint& point : rectanglePoints) {
		if (!isInside(point)) {
			rectanglePoints = trimPolygon(rectangle);
			break;
		}
	}

	if (rectangle.getIsFilled()) {
		fillPolygon(rectangle);
	}
	painter->setPen(QPen(borderColor));

	std::vector<Line> lines;
	if (!rectanglePoints.isEmpty()) {
		lines.emplace_back(rectanglePoints.at(0), rectanglePoints.at(1), rectangle.getZBufferPosition(), rectangle.getIsFilled(), borderColor, fillingColor);
		lines.emplace_back(rectanglePoints.at(1), rectanglePoints.at(2), rectangle.getZBufferPosition(), rectangle.getIsFilled(), borderColor, fillingColor);
		lines.emplace_back(rectanglePoints.at(2), rectanglePoints.at(3), rectangle.getZBufferPosition(), rectangle.getIsFilled(), borderColor, fillingColor);
		lines.emplace_back(rectanglePoints.at(3), rectanglePoints.at(0), rectangle.getZBufferPosition(), rectangle.getIsFilled(), borderColor, fillingColor);
	}
	for (Line& line : lines) {
		drawLine(line);
	}

	update();
}

void ViewerWidget::moveRectangle(const QPoint& offset) {
	qDebug() << "Current Layer: " << currentLayer;
	qDebug() << "Z-Buffer Size: " << zBuffer.size();
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		qDebug() << "ShapeType: " << pair.first.get().getType();
		if (pair.first.get().getType() == Shape::RECTANGLE) {
			MyRectangle& rectangle = static_cast<MyRectangle&>(pair.first.get());
			const QVector<QPoint>& points = rectangle.getPoints();

			QVector<QPoint> movedPoints;
			for (const QPoint& point : points) {
				movedPoints.append(point + offset);
			}

			for (const QPoint& point : movedPoints) {
				qDebug() << "Rectangle Point: " << point.x() << "," << point.y();
			}

			pair.first.get().setPoints(movedPoints);
			redrawAllShapes();
		}
	}
}

void ViewerWidget::scaleRectangle(double scaleX, double scaleY) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::RECTANGLE) {
			MyRectangle& rectangle = static_cast<MyRectangle&>(pair.first.get());
			const QVector<QPoint>& points = rectangle.getPoints();
			QPoint center = getPolygonCenter(rectangle);

			QVector<QPoint> scaledPoints;
			for (const QPoint& point : points) {
				double newX = center.x() + (point.x() - center.x()) * scaleX;
				double newY = center.y() + (point.y() - center.y()) * scaleY;

				scaledPoints.append(QPoint(static_cast<int>(std::round(newX)), static_cast<int>(std::round(newY))));
			}

			pair.first.get().setPoints(scaledPoints);
			redrawAllShapes();
		}
	}
}

void ViewerWidget::turnRectangle(int angle) {
	if (currentLayer >= 0 && currentLayer < zBuffer.size()) {
		auto& pair = zBuffer[currentLayer];
		if (pair.first.get().getType() == Shape::RECTANGLE) {
			MyRectangle& rectangle = static_cast<MyRectangle&>(pair.first.get());
			const QVector<QPoint>& points = rectangle.getPoints();
			QPoint center = getPolygonCenter(rectangle);

			double radians = qDegreesToRadians(static_cast<double>(angle));
			double cosAngle = std::cos(radians);
			double sinAngle = std::sin(radians);

			QVector<QPoint> rotatedPoints;

			for (const QPoint& point : points) {
				int translatedX = point.x() - center.x();
				int translatedY = point.y() - center.y();

				int rotatedX = static_cast<int>(translatedX * cosAngle - translatedY * sinAngle);
				int rotatedY = static_cast<int>(translatedX * sinAngle + translatedY * cosAngle);

				rotatedX += center.x();
				rotatedY += center.y();

				rotatedPoints.append(QPoint(rotatedX, rotatedY));
			}

			pair.first.get().setPoints(rotatedPoints);
			redrawAllShapes();
		}
	}
}
