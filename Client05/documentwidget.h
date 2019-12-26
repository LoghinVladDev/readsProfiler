#ifndef DOCUMENTWIDGET_H
#define DOCUMENTWIDGET_H

#include <QtCore>
#include <QtGui>
#include <QWidget>
#include <poppler-qt5.h>

#define DEFAULTWIDTH 72
#define DEFAULTHEIGHT 72

typedef Poppler::Page page;
typedef Poppler::Document document;

class DocumentWidget : public QObject
{
    Q_OBJECT
public:
   DocumentWidget(const char* documentPath = nullptr);

   void documentReload();
   inline void setZoom( double zoom ) { this->zoom = zoom; }

   inline bool isReady() const { return isLoaded; }

   const char* documentName() const;
   const char* documentPath() const;
   const char * pageText(int) const;
   const char * text ( int, QRectF ) const;

   int pageCount() const;

   double zoomWidth( int, double) const;
   double zoomHeight( int, double) const;

   QSizeF pageSize( int ) const;

   QImage renderPage( int ) const;
   QImage renderPageWidth( int, double) const;
   QImage renderPageHeight( int, double) const;


signals:
    void documentLoaded();
    void documentLoadFailed();

public slots:
    void documentLoad();

private:
    document *bookDocument;

    QList <page*> pdfPages;

    char* pathDocument;

    double zoom;

    bool isLoaded;
};

#endif // DOCUMENTWIDGET_H
