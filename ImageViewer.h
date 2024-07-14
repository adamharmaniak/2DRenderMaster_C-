#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"
#include "lighting.h"
#include "representation.h"

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);

private:
	Ui::ImageViewerClass* ui;
	ViewerWidget* vW;

	QColor fillingColor;
	QColor borderColor;
	QSettings settings;
	QMessageBox msgBox;

	bool objectLoaded = false;
	int currentLayer;

	MyPolygon* polygon = nullptr;
	MyRectangle* rectangle = nullptr;
	BezierCurve* curve = nullptr;
	Line* line = nullptr;
	Circle* circle = nullptr;

	//Event filters
	bool eventFilter(QObject* obj, QEvent* event);

	//ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject* obj, QEvent* event);
	void ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event);
	void ViewerWidgetLeave(ViewerWidget* w, QEvent* event);
	void ViewerWidgetEnter(ViewerWidget* w, QEvent* event);
	void ViewerWidgetWheel(ViewerWidget* w, QEvent* event);

	//ImageViewer Events
	void closeEvent(QCloseEvent* event);

	//Image functions
	bool openImage(QString filename);
	bool saveImage(QString filename);

private slots:
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();
	void layerSelectionChanged(int currentRow);
	void on_pushButtonSaveImage_clicked();
	void on_pushButtonLoadImage_clicked();

//-----------------------------------------
//		*** Tools 2D slots ***
//-----------------------------------------
	void on_pushButtonSetColor_clicked();
	void on_pushButtonSetBorderColor_clicked();
	void on_pushButtonTurn_clicked();
	void on_pushButtonScale_clicked();
	void on_pushButtonChangeLayerColor_clicked();
	void on_pushButtonLayerUp_clicked();
	void on_pushButtonLayerDown_clicked();
	void on_pushButtonDeleteObject_clicked();
};
