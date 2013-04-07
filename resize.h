#ifndef RESIZE_H
#define RESIZE_H

#include <QMainWindow>
#include <QDebug>
#include <QImageWriter>
#include <QImageIOHandler>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QLabel>
#include <QGridLayout>

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <exiv2/image.hpp>
#include <exiv2/exif.hpp>

namespace Ui {
class Resize;
}

struct Image{
    cv::Mat original;
    QPixmap preview;
    QString folder;
    QString filename;
};

class Resize : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Resize(QWidget *parent = 0);
    ~Resize();
    
private:
    Ui::Resize *ui;
    QStringList files;

    QMap<QString,Image> mapImages;

    QList<Image> listImages;

    QString logoPath;

    //QGridLayout *gridPreview;

protected:
    void setList(QStringList paths);

    void rotate(cv::Mat &img, int mode);
    cv::Mat createSmall(cv::Mat &img, int size);

    int readOrientation(QString filepath);

    void addList(QStringList);
    void addFile(QString);
    void removeFile(QString);

    void repaintGrid();


public slots:
    void pressOpenFolder();
    void pressOpenFiles();
    void editPixels(QString);
    void editRatio(QString);
    void comboPixels(QString);
    void comboRatio(QString);

    void resizeAll();

    void handleMessage(const QString& message);

    void setRatioMode(bool);
    void setSizeMode(bool);

    void openLogo();

signals:
    void needToShow();

};

#endif // RESIZE_H
