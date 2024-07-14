#include "ImageViewer.h"

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass), currentLayer(-1)
{
	ui->setupUi(this);
	vW = new ViewerWidget(QSize(500, 500));
	ui->scrollArea->setWidget(vW);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(true);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	fillingColor = Qt::blue;
	borderColor = Qt::blue;
	QString style_sheet = QString("background-color: #%1;").arg(fillingColor.rgba(), 0, 16);
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
	ui->pushButtonSetBorderColor->setStyleSheet(style_sheet);

	vW->setBorderColor(borderColor);
	vW->setFillingColor(fillingColor);

	connect(ui->listWidget, &QListWidget::currentRowChanged, this, &ImageViewer::layerSelectionChanged);
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return false;
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}
void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
	static bool polygonActive = false;
	static bool curveActive = false;

	//	>> Line Drawing
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawLine->isChecked() && !ui->pushButtonMove->isChecked())
	{
		if (w->getDrawLineActivated()) {
			int layerIndex = ui->listWidget->count();
			ui->listWidget->addItem(QString("Line %1").arg(layerIndex + 1));
			int newRowIndex = ui->listWidget->count() - 1;
			ui->listWidget->setCurrentRow(newRowIndex);
			layerSelectionChanged(newRowIndex);

			line = new Line(w->getDrawLineBegin(), e->pos(), layerIndex, ui->checkBoxFilling->isChecked(), borderColor, fillingColor);
			w->drawLine(*line);
			w->addToZBuffer(*line, line->getZBufferPosition());

			w->setDrawLineActivated(false);
			w->update();
		}
		else {
			w->setDrawLineBegin(e->pos());
			w->setDrawLineActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), borderColor);
			w->update();
		}
	}

	//	>> Circle Drawing
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawCircle->isChecked() && !ui->pushButtonMove->isChecked())
	{
		if (w->getDrawCircleActivated()) {
			int layerIndex = ui->listWidget->count();
			ui->listWidget->addItem(QString("Circle %1").arg(layerIndex + 1));
			int newRowIndex = ui->listWidget->count() - 1;
			ui->listWidget->setCurrentRow(newRowIndex);

			circle = new Circle(w->getDrawCircleCenter(), e->pos(), layerIndex, ui->checkBoxFilling->isChecked(), borderColor, fillingColor);
			w->drawCircle(*circle);
			vW->addToZBuffer(*circle, circle->getZBufferPosition());
			w->setDrawCircleActivated(false);
		}
		else {
			w->setDrawCircleCenter(e->pos());
			w->setDrawCircleActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), borderColor);
			w->update();
		}
	}

	//	>> Polygon Drawing
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawPolygon->isChecked() && !ui->pushButtonMove->isChecked()) {
		if (!polygonActive) {
			int layerIndex = ui->listWidget->count();
			ui->listWidget->addItem(QString("Polygon %1").arg(layerIndex + 1));
			int newRowIndex = ui->listWidget->count() - 1;
			ui->listWidget->setCurrentRow(newRowIndex);

			polygon = new MyPolygon(QVector<QPoint>(), layerIndex, ui->checkBoxFilling->isChecked(), borderColor, fillingColor);
			polygonActive = true;
		}

		w->setPixel(e->pos().x(), e->pos().y(), borderColor);
		polygon->addPoint(e->pos());
		w->update();
	}
	if (e->button() == Qt::RightButton && ui->toolButtonDrawPolygon->isChecked()) {
		if (polygonActive) {
			w->drawPolygon(*polygon);
			vW->addToZBuffer(*polygon, polygon->getZBufferPosition());
			polygonActive = false;
		}
	}
	//	>> Curve Drawing
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawCurve->isChecked() && !ui->pushButtonMove->isChecked()) {
		if (!curveActive) {
			int layerIndex = ui->listWidget->count();
			ui->listWidget->addItem(QString("Bezier Curve %1").arg(ui->listWidget->count() + 1));
			int newRowIndex = ui->listWidget->count() - 1;
			ui->listWidget->setCurrentRow(newRowIndex);

			curve = new BezierCurve(QVector<QPoint>(), ui->listWidget->count(), ui->checkBoxFilling->isChecked(), borderColor, fillingColor);
			curveActive = true;
		}

		curve->addPoint(e->pos());
		w->setPixel(e->pos().x(), e->pos().y(), borderColor);
		w->update();
	}
	if (e->button() == Qt::RightButton && ui->toolButtonDrawCurve->isChecked()) {
		if (curveActive) {
			w->drawCurve(*curve);
			vW->addToZBuffer(*curve, curve->getZBufferPosition());
			curveActive = false;
		}
	}

	//	>> Rectangle Drawing
	if (e->button() == Qt::LeftButton && ui->toolButtonDrawRectangle->isChecked() && !ui->pushButtonMove->isChecked())
	{
		if (w->getDrawRectangleActivated()) {
			int layerIndex = ui->listWidget->count();
			ui->listWidget->addItem(QString("Rectangle %1").arg(layerIndex + 1));
			int newRowIndex = ui->listWidget->count() - 1;
			ui->listWidget->setCurrentRow(newRowIndex);
			
			if ((e->pos().y() > w->getDrawRectangleBegin().y() && e->pos().x() > w->getDrawRectangleBegin().x()) || (e->pos().y() < w->getDrawRectangleBegin().y() && w->getDrawRectangleBegin().x() > e->pos().x())) {
				rectangle = new MyRectangle(w->getDrawRectangleBegin(), QPoint(e->pos().x(), w->getDrawRectangleBegin().y()), e->pos(), QPoint(w->getDrawRectangleBegin().x(), e->pos().y()), layerIndex, ui->checkBoxFilling->isChecked(), borderColor, fillingColor);
			}
			else {
				rectangle = new MyRectangle(w->getDrawRectangleBegin(), QPoint(w->getDrawRectangleBegin().x(), e->pos().y()), e->pos(), QPoint(e->pos().x(), w->getDrawRectangleBegin().y()), layerIndex, ui->checkBoxFilling->isChecked(), borderColor, fillingColor);
			}

			w->drawRectangle(*rectangle);
			vW->addToZBuffer(*rectangle, rectangle->getZBufferPosition());
			w->setDrawRectangleActivated(false);
		}
		else {
			w->setDrawRectangleBegin(e->pos());
			w->setDrawRectangleActivated(true);
			w->setPixel(e->pos().x(), e->pos().y(), borderColor);
			w->update();
		}
	}
}
void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);
}
void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	//	>> Polygon Movement
	if (ui->toolButtonDrawPolygon->isChecked()) {
		if (e->buttons() & Qt::LeftButton && ui->pushButtonMove->isChecked()) {
			QPoint offset = e->pos() -  w->getMoveStart();
			if (!w->getMoveStart().isNull()) {
				w->movePolygon(offset);
			}
			w->setMoveStart(e->pos());
		}
		else if (ui->pushButtonMove->isChecked()) {
			w->setMoveStart(QPoint());
		}
	}
	
	//	>> Line Movement
	if (ui->toolButtonDrawLine->isChecked()) {
		if (e->buttons() & Qt::LeftButton && ui->pushButtonMove->isChecked()) {
			QPoint offset = e->pos() - w->getMoveStart();
			if (!w->getMoveStart().isNull()) {
				w->moveLine(offset);
			}
			w->setMoveStart(e->pos());
		}
		else if (ui->pushButtonMove->isChecked()) {
			w->setMoveStart(QPoint());
		}
	}
	
	//	>> Curve Movement
	if (ui->toolButtonDrawCurve->isChecked()) {
		if (e->buttons() & Qt::LeftButton && ui->pushButtonMove->isChecked()) {
			QPoint offset = e->pos() - w->getMoveStart();
			if (!w->getMoveStart().isNull()) {
				w->moveCurve(offset);
			}
			w->setMoveStart(e->pos());
		}
		else if (ui->pushButtonMove->isChecked()) {
			w->setMoveStart(QPoint());
		}
	}

	//	>> Circle Movement
	if (ui->toolButtonDrawCircle->isChecked()) {
		if (e->buttons() & Qt::LeftButton && ui->pushButtonMove->isChecked()) {
			QPoint offset = e->pos() - w->getMoveStart();
			if (!w->getMoveStart().isNull()) {
				w->moveCircle(offset);
			}
			w->setMoveStart(e->pos());
		}
		else if (ui->pushButtonMove->isChecked()) {
			w->setMoveStart(QPoint());
		}
	}

	//	>> Rectangle Movement
	if (ui->toolButtonDrawRectangle->isChecked()) {
		if (e->buttons() & Qt::LeftButton && ui->pushButtonMove->isChecked()) {
			QPoint offset = e->pos() - w->getMoveStart();
			if (!w->getMoveStart().isNull()) {
				w->moveRectangle(offset);
			}
			w->setMoveStart(e->pos());
		}
		else if (ui->pushButtonMove->isChecked()) {
			w->setMoveStart(QPoint());
		}
	}
}
void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}
void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);

	if (ui->checkBoxScale->isChecked()) {
		int deltaY = wheelEvent->angleDelta().y();
		double scale = 1.0;
		if (deltaY < 0) {
			scale = 0.75;
		}
		else if (deltaY > 0) {
			scale = 1.25;
		}
		if (ui->toolButtonDrawPolygon->isChecked()) {
			w->scalePolygon(scale, scale);
		}
		if (ui->toolButtonDrawLine->isChecked()) {
			w->scaleLine(scale, scale);
		}
		if (ui->toolButtonDrawCurve->isChecked()) {
			w->scaleCurve(scale, scale);
		}
		if (ui->toolButtonDrawCircle->isChecked()) {
			w->scaleCircle(scale, scale);
		}
		if (ui->toolButtonDrawRectangle->isChecked()) {
			w->scaleRectangle(scale, scale);
		}
	}
}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

//-----------------------------------------
//		*** Tools 2D slots ***
//-----------------------------------------

void ImageViewer::layerSelectionChanged(int currentRow) {
	currentLayer = currentRow;
	vW->setLayer(currentLayer);
}

void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm .*xbm .* xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}

void ImageViewer::on_actionClear_triggered()
{
	objectLoaded = false;

	ui->toolButtonDrawPolygon->setDisabled(false);
	ui->toolButtonDrawCurve->setDisabled(false);
	ui->toolButtonDrawCurve->setChecked(false);
	ui->toolButtonDrawPolygon->setChecked(false);
	ui->toolButtonDrawRectangle->setChecked(false);
	ui->toolButtonDrawCircle->setChecked(false);
	ui->toolButtonDrawLine->setChecked(false);
	ui->pushButtonMove->setChecked(false);
	ui->checkBoxScale->setChecked(false);
	ui->checkBoxFilling->setChecked(false);
	ui->listWidget->clear();

	vW->clearZBuffer();
	vW->clear();
}

void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(fillingColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		fillingColor = newColor;
	}
	vW->setFillingColor(fillingColor);
}

void ImageViewer::on_pushButtonSetBorderColor_clicked()
{
	QColor newColor = QColorDialog::getColor(borderColor, this);
	if (newColor.isValid()) {
		QString style_sheet = QString("background-color: #%1;").arg(newColor.rgba(), 0, 16);
		ui->pushButtonSetBorderColor->setStyleSheet(style_sheet);
		borderColor = newColor;
	}
	vW->setBorderColor(borderColor);
}

void ImageViewer::on_pushButtonChangeLayerColor_clicked() {
	vW->changeLayerColor(ui->listWidget->currentRow(), borderColor, fillingColor);
	vW->redrawAllShapes();
}

void ImageViewer::on_pushButtonTurn_clicked() {
	if (ui->toolButtonDrawPolygon->isChecked()) {
		vW->turnPolygon(ui->spinBoxTurn->value());
	}
	if (ui->toolButtonDrawLine->isChecked()) {
		vW->turnLine(ui->spinBoxTurn->value());
	}
	if (ui->toolButtonDrawCurve->isChecked()) {
		vW->turnCurve(ui->spinBoxTurn->value());
	}
	if (ui->toolButtonDrawRectangle->isChecked()) {
		vW->turnRectangle(ui->spinBoxTurn->value());
	}
}

void ImageViewer::on_pushButtonScale_clicked() {
	if (ui->toolButtonDrawPolygon->isChecked()) {
		vW->scalePolygon(ui->doubleSpinBoxScaleX->value(), ui->doubleSpinBoxScaleY->value());
	}
	if (ui->toolButtonDrawLine->isChecked()) {
		vW->scaleLine(ui->doubleSpinBoxScaleX->value(), ui->doubleSpinBoxScaleY->value());
	}
	if (ui->toolButtonDrawCurve->isChecked()) {
		vW->scaleCurve(ui->doubleSpinBoxScaleX->value(), ui->doubleSpinBoxScaleY->value());
	}
	if (ui->toolButtonDrawCircle->isChecked()) {
		vW->scaleCircle(ui->doubleSpinBoxScaleX->value(), ui->doubleSpinBoxScaleY->value());
	}
	if (ui->toolButtonDrawRectangle->isChecked()) {
		vW->scaleRectangle(ui->doubleSpinBoxScaleX->value(), ui->doubleSpinBoxScaleY->value());
	}
}

void ImageViewer::on_pushButtonLayerUp_clicked() {
	int currentRow = ui->listWidget->currentRow();

	if (currentRow > 0) {
		vW->moveShapeUp(currentRow);
		int newRowIndex = currentRow - 1;

		QListWidgetItem* currentItem = ui->listWidget->takeItem(currentRow);
		QListWidgetItem* aboveItem = ui->listWidget->takeItem(newRowIndex);

		ui->listWidget->insertItem(currentRow, aboveItem);
		ui->listWidget->insertItem(newRowIndex, currentItem);

		ui->listWidget->setCurrentRow(newRowIndex);
	}
	else {
		QMessageBox::warning(this, "Invalid Depth", "Dosiahli ste maximalnu hlbku pre Z-Buffer.");
	}
}

void ImageViewer::on_pushButtonLayerDown_clicked() {
	int currentRow = ui->listWidget->currentRow();

	if (currentRow < 0) {
		QMessageBox::warning(this, "No Selection", "Vrstva pre posunutie neexistuje");
		return;
	}

	int newRowIndex = currentRow + 1;

	if (newRowIndex < ui->listWidget->count()) {
		vW->moveShapeDown(currentRow);

		QListWidgetItem* currentItem = ui->listWidget->takeItem(currentRow);
		QListWidgetItem* belowItem = ui->listWidget->takeItem(newRowIndex);

		ui->listWidget->insertItem(currentRow, belowItem);
		ui->listWidget->insertItem(newRowIndex, currentItem);

		ui->listWidget->setCurrentRow(newRowIndex);

		vW->redrawAllShapes();
	}
	else {
		QMessageBox::warning(this, "Invalid Depth", "You have reached the minimum depth for Z-Buffer.");
	}
}

void ImageViewer::on_pushButtonDeleteObject_clicked() {
	int currentRow = ui->listWidget->currentRow();

	if (currentRow < 0) {
		QMessageBox::warning(this, "No Selection", "Neexistuje vrstva na vymazanie");
		return;
	}

	vW->deleteObjectFromZBuffer(currentRow);

	QListWidgetItem* item = ui->listWidget->takeItem(currentRow);
	delete item;

	vW->redrawAllShapes();
}

void ImageViewer::on_pushButtonSaveImage_clicked() {
	vW->saveCurrentImageState();
}

void ImageViewer::on_pushButtonLoadImage_clicked() {
	QString filePath = QFileDialog::getOpenFileName(this, "Load Image State", "C:\\Pocitacova_grafika_projects\\ImageViewer_projekt_zaverecny", "CSV Files (*.csv)");
	if (filePath.isEmpty()) {
		return;
	}

	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this, "File Error", "Unable to open file for reading.");
		return;
	}

	vW->clearZBuffer();
	ui->listWidget->clear();

	QTextStream in(&file);

	in.readLine();

	while (!in.atEnd()) {
		QString line = in.readLine();
		QStringList fields = line.split(',');

		if (fields.size() < 6) {
			QMessageBox::warning(this, "File Error", "Invalid file format.");
			return;
		}

		QString shapeType = fields[0];
		int zBufferPosition = fields[1].toInt();
		bool isFilled = (fields[2] == "true");
		QColor borderColor(fields[3]);
		QColor fillingColor(fields[4]);

		QVector<QPoint> points;
		QString pointsStr = fields.mid(5).join(",");
		QStringList pointPairs = pointsStr.split(' ', Qt::SkipEmptyParts);
		for (const QString& pair : pointPairs) {
			QString cleanPair = pair.trimmed().remove('(').remove(')');
			QStringList coords = cleanPair.split(',');
			if (coords.size() == 2) {
				int x = coords[0].toInt();
				int y = coords[1].toInt();
				points.append(QPoint(x, y));
			}
		}

		Shape* shape = nullptr;
		if (shapeType == "Line" && points.size() == 2) {
			shape = new Line(points[0], points[1], zBufferPosition, isFilled, borderColor, fillingColor);
		}
		else if (shapeType == "Rectangle" && points.size() == 4) {
			shape = new MyRectangle(points[0], points[1], points[2], points[3], zBufferPosition, isFilled, borderColor, fillingColor);
		}
		else if (shapeType == "Polygon" && points.size() >= 3) {
			shape = new MyPolygon(points, zBufferPosition, isFilled, borderColor, fillingColor);
		}
		else if (shapeType == "Circle" && points.size() == 2) {
			shape = new Circle(points[0], points[1], zBufferPosition, isFilled, borderColor, fillingColor);
		}
		else if (shapeType == "BezierCurve" && points.size() >= 3) {
			shape = new BezierCurve(points, zBufferPosition, isFilled, borderColor, fillingColor);
		}
		else {
			QMessageBox::warning(this, "File Error", "Invalid shape type or points in file.");
			continue;
		}

		if (shape != nullptr) {
			vW->addToZBuffer(*shape, zBufferPosition);
			ui->listWidget->addItem(shapeType + " " + QString::number(zBufferPosition + 1));
		}
	}

	file.close();
	vW->redrawAllShapes();

	QMessageBox::information(this, "Load Successful", "The saved state has been loaded successfully.");
}