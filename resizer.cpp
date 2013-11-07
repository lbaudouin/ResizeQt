#include "resizer.h"
#include "ui_resizer.h"

Resizer::Resizer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Resizer)
{
    ui->setupUi(this);

    QStringList listSize;
    listSize << "320" << "480" << "640" << "720" << "800" << "1024" << "1280" << "2048" << "4096";
    ui->comboPixels->addItems( listSize );
    ui->comboPixels->setCurrentIndex(5);
    QStringList listRatio;
    listRatio << "10" << "20" << "30" << "33" << "40" << "50" << "60" << "66" << "70" << "80" << "90" << "125" << "150" << "200" << "250" << "300";
    ui->comboRatio->addItems( listRatio );
    ui->comboRatio->setCurrentIndex(3);

    QIntValidator *validatorNotNull = new QIntValidator(this);
    validatorNotNull->setRange(8,32768);
    ui->comboPixels->lineEdit()->setValidator(validatorNotNull);


    QIntValidator *validator = new QIntValidator(this);
    validator->setRange(0,32768);
    ui->horizontalLineEdit->setValidator(validator);
    ui->verticalLineEdit->setValidator(validator);



    connect(ui->buttonOpenFolder,SIGNAL(pressed()),this,SLOT(pressOpenFolder()));
    connect(ui->buttonOpenFiles,SIGNAL(pressed()),this,SLOT(pressOpenFiles()));
    connect(ui->actionAdd_folder,SIGNAL(triggered()),this,SLOT(pressOpenFolder()));
    connect(ui->actionAdd_files,SIGNAL(triggered()),this,SLOT(pressOpenFiles()));

    connect(ui->groupRatio,SIGNAL(clicked(bool)),this,SLOT(setRatioMode(bool)));
    connect(ui->groupSize,SIGNAL(clicked(bool)),this,SLOT(setSizeMode(bool)));

    connect(ui->buttonLogo,SIGNAL(clicked()),this,SLOT(openLogo()));

    /*QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(ui->radioTopLeft);
    buttonGroup->addButton(ui->radioBottomRight);
    buttonGroup->setExclusive(true);*/

    ui->buttonOpenFiles->setText("");
    ui->buttonOpenFolder->setText("");
    ui->buttonOpenFiles->setIcon(QIcon(":images/pictures"));
    ui->buttonOpenFolder->setIcon(QIcon(":images/folder"));


    ui->actionAdd_files->setIcon(QIcon(":images/pictures"));
    ui->actionAdd_folder->setIcon(QIcon(":images/folder"));

    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(close()));
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(resizeAll()));

    readSettings();

    setLogo(logoPath);

    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(pressAbout()));
}

Resizer::~Resizer()
{
    writeSettings();
    delete ui;
}

void Resizer::writeSettings()
{
    QSettings settings(qAppName(), "config");

    settings.setValue("options/keep_exif",ui->checkExif->isChecked());
    settings.setValue("options/no_resize",ui->checkNotResize->isChecked());
    settings.setValue("options/rotate",ui->checkRotate->isChecked());

    settings.setValue("small/mode",ui->groupRatio->isChecked()?"ratio":"size");
    settings.setValue("small/ratio",ui->comboRatio->currentText());
    settings.setValue("small/pixels",ui->comboPixels->currentText());

    settings.setValue("logo/attach",ui->selector->position());
    settings.setValue("logo/shift-x",ui->horizontalLineEdit->text().toInt());
    settings.setValue("logo/shift-y",ui->verticalLineEdit->text().toInt());
    settings.setValue("logo/path",logoPath);
}

void Resizer::readSettings()
{
    QSettings settings(qAppName(), "config");

    ui->checkExif->setChecked( settings.value("options/keep_exif",true).toBool() );
    ui->checkNotResize->setChecked( settings.value("options/no_resize",false).toBool() );
    ui->checkRotate->setChecked( settings.value("options/rotate",true).toBool() );


    setSizeMode( settings.value("small/mode","size")=="size"?true:false );
    ui->comboRatio->setCurrentIndex(ui->comboRatio->findText(settings.value("small/ratio",33).toString()));
    int pixelsIndex = ui->comboPixels->findText(settings.value("small/pixels",1024).toString());
    if(pixelsIndex>=0)
        ui->comboPixels->setCurrentIndex(pixelsIndex);
    else
        ui->comboPixels->lineEdit()->setText(settings.value("small/pixels",1024).toString());

    ui->selector->setPosition( static_cast<PositionSelector::POSITION>(settings.value("logo/attach",3).toInt()) );
    ui->horizontalLineEdit->setText( settings.value("logo/shift-x",25).toString() );
    ui->verticalLineEdit->setText( settings.value("logo/shift-y",25).toString() );
    logoPath = settings.value("logo/path",QDesktopServices::storageLocation(QDesktopServices::PicturesLocation)  + QDir::separator() + "logo.png").toString();
}

void Resizer::setRatioMode(bool ratioMode)
{
    ui->groupRatio->setChecked(ratioMode);
    ui->groupSize->setChecked(!ratioMode);
}

void Resizer::setSizeMode(bool sizeMode)
{
    ui->groupSize->setChecked(sizeMode);
    ui->groupRatio->setChecked(!sizeMode);
}

void Resizer::pressOpenFolder()
{
    QString path = QFileDialog::getExistingDirectory(this,tr("Select a Folder"),QDir::homePath());
    if(path.isEmpty()) return;
    QDir dir(path);
    QStringList filenames = dir.entryList(QStringList() << "*.jpg" << "*.JPG"<< "*.jpeg" << "*.JPEG" << "*.png" << "*.PNG");

    QStringList absoluteFilepaths;
    for(int i=0;i<filenames.size();i++)
        absoluteFilepaths << dir.absolutePath() + QDir::separator() + filenames.at(i);
    addList(absoluteFilepaths);
}

void Resizer::pressOpenFiles()
{
    QStringList paths = QFileDialog::getOpenFileNames(this,tr("Select Files"),QDir::homePath(), tr("Image files (*.jpg *.jpeg *.png)"));
    if(paths.isEmpty()) return;
    QStringList absoluteFilepaths;
    for(int i=0;i<paths.size();i++){
        QFileInfo fi(paths[i]);
        absoluteFilepaths << fi.absoluteFilePath();
    }
    addList(absoluteFilepaths);
}

void Resizer::addList(QStringList paths)
{
    QProgressDialog *diag = new QProgressDialog(tr("Loading"),tr("Cancel"),0,paths.size()-1,this);
    if(paths.size()>1)
        diag->show();

    for(int i=0;i<paths.size();i++){
        QString filepath = paths[i];

        QFileInfo fi(filepath);

        if(fi.isDir()){
            QDir dir(filepath);
            QStringList filenames = dir.entryList(QStringList() << "*.jpg" << "*.JPG"<< "*.jpeg" << "*.JPEG" << "*.png" << "*.PNG");

            QStringList absoluteFilepaths;
            for(int i=0;i<filenames.size();i++)
                absoluteFilepaths << dir.absolutePath() + QDir::separator() + filenames.at(i);
            addList(absoluteFilepaths);

            continue;
        }

        QImage small;

        QImageReader reader(filepath);
        QSize imageSize = reader.size();

        if(imageSize.isValid()){
            imageSize.scale(320,320,Qt::KeepAspectRatio);
            reader.setScaledSize(imageSize);
            small = reader.read();
        }else{
            small = QImage(filepath).scaled(320,320,Qt::KeepAspectRatio);
        }

        bool rotateNeeded = ui->checkRotate->isChecked();

        QTransform transform;
        if(rotateNeeded){
            int orientation = readOrientation(filepath);

            switch(orientation){
            case 6: transform.rotate(90); break;
            case 3: transform.rotate(180); break;
            case 8: transform.rotate(270); break;
            default: rotateNeeded = false;
            }
        }

        QPixmap pix;
        if(rotateNeeded){
            QImage rotated = small.transformed(transform);
            pix = QPixmap::fromImage(rotated);
        }else{
            pix = QPixmap::fromImage(small);
        }

        QLabel *label = new QLabel;
        label->setPixmap(pix);

        ImageInfo *imageinfo = new ImageInfo;
        imageinfo->preview = pix;
        imageinfo->fileinfo = fi;

        int k = mapImages.size();

        ui->gridLayout_4->addWidget(label,k/5,k%5,Qt::AlignHCenter);

        mapImages.insert(fi.absoluteFilePath(),imageinfo);

        diag->setValue( diag->value()+1 );
    }
    diag->close();
}

void Resizer::addFile(QString filepath)
{
    addList(QStringList() << filepath);
}

void Resizer::removeFile(QString filepath)
{
    QFileInfo fi(filepath);

    mapImages.remove(fi.absoluteFilePath());

    repaintGrid();
}

void Resizer::repaintGrid()
{
    int k =0;
    foreach(ImageInfo *img, mapImages){

        QLabel *label = new QLabel;
        label->setPixmap( img->preview );

        ui->gridLayout_4->addWidget(label,k/5,k%5,Qt::AlignHCenter);

        k++;
    }
}

int Resizer::readOrientation(QString filepath)
{
    if(QFile::exists(filepath)){
        return QExifImageHeader(filepath).value(QExifImageHeader::Orientation).toShort();
    }
    return 0;
}

void Resizer::openLogo()
{
    QString logo = QFileDialog::getOpenFileName(this,tr("Select Logo"),"",tr("Image files (*.jpg *.jpeg *.png)"));

    if(logo.isEmpty())
        return;

    setLogo(logo);
}

void Resizer::setLogo(QString path)
{
    logoPath = path;
    ui->editLogo->setText(path);

    if(logoPath.isEmpty() || !QFile::exists(logoPath)){
        ui->labelLogo->setText(tr("Logo"));
    }else{
        QPixmap pix(logoPath);
        ui->labelLogo->setPixmap(pix.scaled(400,200,Qt::KeepAspectRatio));
    }
}

void Resizer::resizeAll()
{

    bool ok;
    int maxSize = ui->comboPixels->currentText().toInt(&ok);
    if(!ui->checkNotResize->isChecked() && ui->groupSize->isChecked() && !ok){
        //TODO, display error
        return;
    }

    int size = mapImages.size();

    QProgressDialog *diag = new QProgressDialog(tr("Progress"),tr("Cancel"),0,size-1,this);
    diag->show();


    foreach(ImageInfo *img, mapImages){
        /*QExifImageHeader exif(img.fileinfo.absoluteFilePath());

        QList<QExifImageHeader::ImageTag> list1 = exif.imageTags();
        QList<QExifImageHeader::ExifExtendedTag> list2 = exif.extendedTags();
        QList<QExifImageHeader::GpsTag> list3 = exif.gpsTags();

        for(int i=0;i<list1.size();i++){
            qDebug() << exif.value(list1[i]).toString();
        }
        for(int i=0;i<list2.size();i++){
            qDebug() << exif.value(list2[i]).toString();
        }
        for(int i=0;i<list3.size();i++){
            qDebug() << exif.value(list3[i]).toString();
        }*/

        QString output = img->fileinfo.absoluteDir().absolutePath() + QDir::separator() + "Small" + QDir::separator() + img->fileinfo.fileName();

        QDir dir(img->fileinfo.absoluteDir());
        if(!dir.exists() || !dir.mkpath("Small")){
            //TODO display only one error
            //QMessageBox::critical(this,"error","dir");
            diag->close();
            return;
        }

        QImage small;

        QImageReader reader(img->fileinfo.absoluteFilePath());
        QSize imageSize = reader.size();

        if(ui->checkNotResize->isChecked()){
            small.load(img->fileinfo.absoluteFilePath());
        }else{
            if(imageSize.isValid()){
                if(ui->groupSize->isChecked()){
                    imageSize.scale(maxSize,maxSize,Qt::KeepAspectRatio);
                }else{
                    imageSize *= (double) ui->comboRatio->currentText().toInt() / 100.0;
                }
                reader.setScaledSize(imageSize);
                small = reader.read();
            }else{
                QImage original(img->fileinfo.absoluteFilePath());
                imageSize = original.size();
                if(ui->groupSize->isChecked()){
                    imageSize.scale(maxSize,maxSize,Qt::KeepAspectRatio);
                }else{
                    imageSize *= (double) ui->comboRatio->currentText().toInt() / 100.0;
                }
                small = original.scaled(imageSize,Qt::KeepAspectRatio);
            }
        }

        bool rotateNeeded = ui->checkRotate->isChecked();

        QTransform transform;
        if(rotateNeeded){
            int orientation = readOrientation(img->fileinfo.absoluteFilePath());

            switch(orientation){
            case 6: transform.rotate(90); break;
            case 3: transform.rotate(180); break;
            case 8: transform.rotate(270); break;
            default: rotateNeeded = false;
            }
            small = small.transformed(transform);
        }

        if(ui->groupLogo->isChecked() && !logoPath.isEmpty() && QFile::exists(logoPath)){
            bool ok_X,ok_Y;

            int posX = ui->horizontalLineEdit->text().toInt(&ok_X);
            int posY = ui->verticalLineEdit->text().toInt(&ok_Y);

            if(ok_X && ok_Y){
                QImage logo(logoPath);

                switch(ui->selector->position()){
                case PositionSelector::TOP_LEFT:
                    break;
                case PositionSelector::TOP_RIGHT:
                    posX = small.width() - logo.width() - posX;
                    break;
                case PositionSelector::BOTTOM_LEFT:
                    posY = small.height() - logo.height() - posY;
                    break;
                case PositionSelector::BOTTOM_RIGHT:
                    posX = small.width() - logo.width() - posX;
                    posY = small.height() - logo.height() - posY;
                    break;
                case PositionSelector::CENTER:
                    posX = small.width()/2.0 - logo.width()/2.0 + posX;
                    posY = small.height()/2.0 - logo.height()/2.0 + posY;
                    break;
                default: break;
                }

                QPainter painter(&small);
                painter.drawImage(posX,posY,logo);
                painter.end();
            }
        }

        small.save(output);

        //exif.setValue(QExifImageHeader::Orientation,0);
        //exif.setValue(QExifImageHeader::ImageWidth,small.width());
        //exif.setValue(QExifImageHeader::ImageLength,small.height());

        diag->setValue( diag->value()+1 );
    }

    diag->close();
    this->close();
}

void Resizer::pressAbout()
{
    QMessageBox *mess = new QMessageBox(this);
    mess->setWindowTitle(tr("About"));
    mess->setText(tr("Written by %1 (%2)\nVersion: %3","author, year, version").arg(QString::fromUtf8("Léo Baudouin"),"2013",version_));
    mess->setIcon(QMessageBox::Information);
    mess->exec();
}

void Resizer::restart(QString path)
{
    QProcess process;
    process.startDetached("\""+path+"\"");
    this->close();
}

void Resizer::handleMessage(const QString& message)
{
    enum Action { Nothing, Open, Print } action;

    action = Nothing;
    QString filename = message;
    if (message.toLower().startsWith("/print ")) {
        filename = filename.mid(7);
        action = Print;
    } else if (!message.isEmpty()) {
        action = Open;
    }
    if (action == Nothing) {
        emit needToShow();
        return;
    }

    switch(action) {
    case Print:
        QMessageBox::information(this,tr("Information"),filename);
    break;

    case Open:
    {
        addList(filename.split("\n"));
        emit needToShow();
    }
    break;
    default:
    break;
    };
}
