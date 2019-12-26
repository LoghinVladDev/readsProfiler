#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include <QtCore>
#include <QtGui>

#include <QtWidgets>

#include "documentwidget.h"

class PdfViewer : public QScrollArea
{
    Q_OBJECT
private:
    DocumentWidget* document;
    QHash<int, QImage> renderedImages;
    QHash<int, QLabel*> pages;

    QList<int> visiblePages;

    double zoom;

    int currentPage;
    int viewMode;
    int layoutMode;


    bool isPageVisible( int );

    void reshapeView();

private slots:
    void on_ZoomIn();
    void on_ZoomOut();
    void renderPages();

protected:
    void resizeEvent( QResizeEvent* );
    void wheelEvent( QWheelEvent* );
    void keyPressEvent( QKeyEvent* );

public:
    enum ViewMode{
        singlePageView,
        doublePageView,
        fitPageForWidth,
        fitPageForHeight,
        bookView
    };

    enum LayoutMode{
        singlePage,
        continuous,
        book
    };

    PdfViewer(QWidget* parent);

    void documentLoad( const char* );
    void documentReload();
    void setViewMode( ViewMode );
    void setLayoutMode( LayoutMode );
    void setZoom( double );

    inline int getViewMode() const { return this->viewMode; }
    inline int getLayoutMode() const { return this->layoutMode; }

    inline double getZoom() const {return this->zoom; }
};

#endif // PDFVIEWER_H
