/****************************************************************************

 - Codename: CHER-Ob (Yale Computer Graphics Group)

 - Writers:  Weiqi Shi (weiqi.shi@yale.edu)

 - License:  GNU General Public License Usage
   Alternatively, this file may be used under the terms of the GNU General
   Public License version 3.0 as published by the Free Software Foundation
   and appearing in the file LICENSE.GPL included in the packaging of this
   file. Please review the following information to ensure the GNU General
   Public License version 3.0 requirements will be met:
   http://www.gnu.org/copyleft/gpl.html.

 - Warranty: This software is distributed WITHOUT ANY WARRANTY; without even
   the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
   PURPOSE.

 - Acknowledgments: Some portions of this file are based on the example codes
   of ITK/VTK library from Kitware, QT API from Nokia. I would like to thank
   anonymous help from various software engineering communities.

   This file defines the different classes of bookmark that the user may work
   with (corresponding to the different file types that may be displayed).

*****************************************************************************/

#include "note.h"
#include <time.h> 
#include "../mainWindow.h"
#include <sstream>

std::string colortype2str(const ColorType type)
{
	switch(type)
	{
		case 0:  return "MAROON";
		case 1:  return "RED";
		case 2:  return "ORANGE";
		case 3:  return "YELLOW";
		case 4:  return "LIME";
		case 5:  return "GREEN";
		case 6:  return "AQUA";
		case 7:  return "BLUE";
		case 8:  return "PINK";
		case 9:  return "PURPLE";
		case 10: return "WHITE";
		default: return "WHITE";
	}
}

ColorType str2colortype(const std::string str)
{
	if (str == "MAROON")
		return MAROON;
	else if (str == "RED")
		return RED; 
	else if (str == "ORANGE")
		return ORANGE;
	else if (str == "YELLOW")
		return YELLOW;
	else if (str == "LIME")
		return LIME;
	else if (str == "GREEN")
		return GREEN;
	else if (str == "AQUA")
		return AQUA;
	else if (str == "BLUE")
		return BLUE;
	else if (str == "PINK")
		return PINK;
	else if (str == "PURPLE")
		return PURPLE;
	else if (str == "WHITE")
		return WHITE;
	else
		return WHITE;
}

Note::Note(const int noteId, const ColorType type)
{
	mNoteId = noteId;
	mColor = type;
	mDialog = new QDialog();
	mVbox = new QVBoxLayout();
	mInfo = new QString();
	mTextEdit = new QTextEdit(mDialog);
	mButtons = new QHBoxLayout();
	mLabel = new QLabel(mDialog);
	mSaveButton = new QPushButton("Save");
	mRemoveButton = new QPushButton("Remove");
	mAddImageButton = new QPushButton("Add Image");
	mSaveButton->setEnabled(true);
	mAddImageButton->setEnabled(true);
    mRemoveButton->setEnabled(true);

	connect(mTextEdit, SIGNAL(textChanged()), this, SLOT(textChanged()));
	connect(mSaveButton, SIGNAL(clicked()), this, SLOT(save()));
	connect(mAddImageButton, SIGNAL(clicked()), this, SLOT(addImage()));
    connect(mRemoveButton, SIGNAL(clicked()), this, SLOT(remove()));

	mVbox->addWidget(mLabel);
    mVbox->addWidget(mTextEdit);

	mButtons->addWidget(mAddImageButton);
	mButtons->addWidget(mSaveButton);
    mButtons->addWidget(mRemoveButton);

	mVbox->addLayout(mButtons);

	QScrollArea* scrollArea = new QScrollArea;
	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setWidget(this);
	
    mDialog->setLayout(mVbox);
	mDialog->adjustSize();
	isSaved = false;
	isRemoved = false;
	mDialog->hide();
}

QString Note::getContent()
{
	QString content(*mInfo + "\n" + mTextEdit->toPlainText());
	content.append("\nLinked Images:\n");
	for (int i = 0; i < mImageNotes.size(); i++)
	{
		content.append(mImageNotes[i]->getPath() + "\n");
	}
	return content;
}

void Note::setLabel(QString text)
{		
	mLabel->setText(text);
}

void Note::setInfo(QString text)
{
	mInfo = new QString(text);
}

void Note::textChanged()
{
	isSaved = false;
}

void Note::save()
{
	if (isRemoved)
		return;
    if (!mFile->open(QIODevice::WriteOnly | QIODevice::Text))
	{
		qDebug() << "Open Note file " << this->mNoteId << " " << mFileName << " Failed"; 
        return;
	}
    QTextStream out(mFile);

	out << *mInfo << "\n";
    out << mTextEdit->toPlainText();
	out << "\nLinked Images:\n";
	for (int i = 0; i < mImageNotes.size(); i++)
	{
		out << mImageNotes[i]->getPath() << "\n";
	}
	mFile->close();
	isSaved = true;
}

void Note::clearLayout(QLayout* layout, bool deleteWidgets)
{
    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (deleteWidgets)
        {
            if (QWidget* widget = item->widget())
                //delete widget;
				widget->deleteLater();
        }
        if (QLayout* childLayout = item->layout())
            clearLayout(childLayout, deleteWidgets);
        delete item;
    }
}

void Note::removeImage(int id)
{
	isSaved = false;
	mImageNotes.remove(id);
	clearLayout(mVbox->itemAt(2+id)->layout());
	mVbox->removeItem(mVbox->itemAt(2+id));
	for (int i = 0; i < mImageNotes.size(); i++)
	{
		mImageNotes[i]->setId(i);
	}
}

void Note::remove()
{
	mFile->remove();
	mDialog->hide();
	isRemoved = true;
	emit removeNote(mNoteId, mPath);
}

void Note::addImage(QString fileName)
{
	QString objectPath(*mPath);
	objectPath.truncate(objectPath.lastIndexOf(QDir::separator()));
	ImageNote* imageNote = new ImageNote(objectPath, fileName, mImageNotes.size());
	if (imageNote->getPath() == QString())
	{
		delete imageNote;
		return;
	}
	QVBoxLayout* mImageVBox = new QVBoxLayout();
	QHBoxLayout* mInfoHBox = new QHBoxLayout();
	mImageVBox->addWidget(imageNote->getImage());
	mInfoHBox->addWidget(imageNote->getName());
	mInfoHBox->addWidget(imageNote->getButton());
	mImageVBox->addLayout(mInfoHBox);
	connect(imageNote, SIGNAL(removeImage(int)), this, SLOT(removeImage(int)));
	mVbox->insertLayout(2+mImageNotes.size(), mImageVBox);
	mImageNotes.push_back(imageNote);
	mDialog->adjustSize();
	isSaved = false;
}

PointNote::PointNote(QString path, double* pos, const int cellId, const int noteId, const ColorType type)
	: Note(noteId, type)
{
	mPath = new QString(path);
	mPosition = new double[3];
	//qsrand(time(NULL));
	//int number = qrand();
	mFileName = new QString(path);
	//mFileName->append(QDir::separator() + QString("Note") + WORD_SEPARATOR + QString::number(qHash(QString::number(number))) + QString(".txt"));
	mFileName->append(QDir::separator() + QString("PointNote") + WORD_SEPARATOR + QString::number(cellId) + QString(".txt"));

	qDebug(mFileName->toLatin1());
	mFile = new QFile(*mFileName);
	for (int i = 0; i < 3; i++)
	{
		mPosition[i] = pos[i];
	}
	mCellId = cellId;
	QString label;
	label.append(QString("Point Note: Polygon Id (") + QString::number(cellId) + QString(")"));
	this->setLabel(label);
	QString info(label);
	info.append(QString(" Coordinates (") + QString::number(mPosition[0]) + QString(", ") + QString::number(mPosition[1])
		+ QString(", ") + QString::number(mPosition[2]) + QString(")"));
	info.append(QString("\nColor Type:\n"));
	info.append(QString(colortype2str(mColor).c_str()));
	info.append(QString("\nNote Start:"));
	this->setInfo(info);
	qDebug() << "finish PointNote instructor";
}

PointNote::PointNote(QString path, QString fileName, const int noteId, bool& isSucceed)
	: Note(noteId)
{	
	mPath = new QString(path);
	mPosition = new double[3];
	mFileName = new QString(path);
	mFileName->append(QDir::separator() + fileName);

	qDebug(mFileName->toLatin1());
	mFile = new QFile(*mFileName);
    if (!mFile->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Open Note file " << this->mNoteId << " " << mFileName << " Failed"; 
		isSucceed = false;
		return;
	}
    QTextStream in(mFile);
    QString firstLine = in.readLine();
	if (firstLine.split(" ").size() != 9)
	{
		qDebug() << "The Syntax of First Line is incorrect. The First Line is " << firstLine;
		isSucceed = false;
		return;
	}

	bool ok0, ok1, ok2, ok3;
	double pos[3];
	int cellId;
	cellId = firstLine.split(" ")[4].split(")")[0].split("(")[1].toInt(&ok0);
	pos[0] = firstLine.split(" ")[6].split(",")[0].split("(")[1].toDouble(&ok1);
	pos[1] = firstLine.split(" ")[7].split(",")[0].toDouble(&ok2);
	pos[2] = firstLine.split(" ")[8].split(")")[0].toDouble(&ok3);
	if (!ok0 || !ok1 || !ok2 || !ok3)
	{
		qDebug() << "The Syntax of First Line is incorrect. The First Line is " << firstLine;
		isSucceed = false;
		return;
	}
	while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Color Type:"))
			break;
	}
	QString colorType;
	in >> colorType;
	mColor = str2colortype(colorType.toStdString());
    while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Note Start:"))
			break;
	}
	QString content = in.readAll();
	QString text = content.split("\nLinked Images:\n")[0];
	QString imagePaths = content.split("\nLinked Images:\n")[1];
	QStringList imagePathList = imagePaths.split("\n");
	for (int i = 0; i < imagePathList.size(); i++)
	{
		//qDebug()<<"Image Path"<<imagePathList[i];
		QFileInfo finfo(imagePathList[i]);
		if (!finfo.exists())
			continue;
		QString extension = finfo.suffix().toLower();
		if (extension != tr("png") && extension != tr("jpg") && extension != tr("jpeg") && extension != tr("tif") && extension != tr("bmp"))
			continue;
		addImage(imagePathList[i]);
	}
	this->setText(text);
	for (int i = 0; i < 3; i++)
	{
		mPosition[i] = pos[i];
	}
	mCellId = cellId;
	QString label;
	label.append(QString("Point Note: Polygon Id (") + QString::number(cellId) + QString(")"));
	this->setLabel(label);
	QString info(label);
	info.append(QString(" Coordinates (") + QString::number(mPosition[0]) + QString(", ") + QString::number(mPosition[1])
		+ QString(", ") + QString::number(mPosition[2]) + QString(")"));

	info.append(QString("\nColor Type:\n"));
	info.append(colorType);
	info.append(QString("\nNote Start:"));
	this->setInfo(info);
	mFile->close();
	qDebug() << "finish PointNote instructor";
	isSucceed = true;
}

void PointNote::removePointNote()
{
	mFile->remove();
	this->hideNote(); 
}

SurfaceNote::SurfaceNote(QString path, vtkSmartPointer<vtkSelectionNode> cellIds, const int noteId, const ColorType type)
	: Note(noteId, type)
{
	mPath = new QString(path);
    mCellIds = vtkSmartPointer<vtkSelectionNode>::New();
	mCellIds->SetFieldType(vtkSelectionNode::CELL);
    mCellIds->SetContentType(vtkSelectionNode::INDICES);
	mFileName = new QString(path);
	//mFileName->append(QDir::separator() + QString("Note") + WORD_SEPARATOR + QString::number(qHash(QString::number(number))) + QString(".txt"));
	qsrand(time(NULL) * cellIds->GetSelectionList()->GetSize());
	int number = qrand();
	mFileName->append(QDir::separator() + QString("SurfaceNote") + WORD_SEPARATOR + QString::number(qHash(QString::number(number))) + QString(".txt"));

	qDebug(mFileName->toLatin1());
	mFile = new QFile(*mFileName);

	mCellIds->DeepCopy(cellIds);

	QString label;
	label.append(QString("Surface Note: Number of Selected Polygon ( ") + QString::number(mCellIds->GetSelectionList()->GetSize()) + QString(" )"));
	this->setLabel(label);
	QString info(label);
	info.append(QString("\nPolygon Ids:\n"));
	for (int i = 0; i < mCellIds->GetSelectionList()->GetSize(); i++)
	{
		info.append(QString::number(mCellIds->GetSelectionList()->GetVariantValue(i).ToInt()));
		if (i != mCellIds->GetSelectionList()->GetSize() - 1)
			info.append(QString(' '));
		else
			info.append(QString("\n"));
	}
	info.append(QString("Color Type:\n"));
	info.append(QString(colortype2str(mColor).c_str()));
	info.append(QString("\nNote Start:"));
	this->setInfo(info);
	qDebug() << "finish Surface Note instructor";
}

SurfaceNote::SurfaceNote(QString path, QString fileName, const int noteId, bool& isSucceed)
	: Note(noteId)
{	
	mPath = new QString(path);
	mCellIds = vtkSmartPointer<vtkSelectionNode>::New();
	mCellIds->Initialize();
	mCellIds->SetFieldType(vtkSelectionNode::CELL);
    mCellIds->SetContentType(vtkSelectionNode::INDICES);
	vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
	
	mFileName = new QString(path);
	mFileName->append(QDir::separator() + fileName);

	qDebug(mFileName->toLatin1());
	mFile = new QFile(*mFileName);
    if (!mFile->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Open Note file " << this->mNoteId << " " << mFileName << " Failed"; 
		isSucceed = false;
		return;
	}
	
    QTextStream in(mFile);
    QString firstLine = in.readLine();
	if (firstLine.split(" ").size() != 9)
	{
		qDebug() << "The Syntax of First Line is incorrect. The First Line is " << firstLine;
		isSucceed = false;
		return;
	}
	bool ok0;
	int cellNum;
	cellNum = firstLine.split(" ")[7].toInt(&ok0);
	
	if (!ok0)
	{
		qDebug() << "The Syntax of First Line is incorrect. The First Line is " << firstLine;
		isSucceed = false;
		return;
	}

    while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Polygon Ids:"))
			break;
	}
	ids->Resize(cellNum);
	for (int i = 0; i < cellNum; i++)
	{
		int cellId;
		in >> cellId;
		ids->InsertNextValue(cellId);
	}
	mCellIds->SetSelectionList(ids);
	while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Color Type:"))
			break;
	}
	QString colorType;
	in >> colorType;

	mColor = str2colortype(colorType.toStdString());
	while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Note Start:"))
			break;
	}

	QString content = in.readAll();
	QString text = content.split("\nLinked Images:\n")[0];
	QString imagePaths = content.split("\nLinked Images:\n")[1];
	QStringList imagePathList = imagePaths.split("\n");
	for (int i = 0; i < imagePathList.size(); i++)
	{
		qDebug()<<"Image Path"<<imagePathList[i];
		QFileInfo finfo(imagePathList[i]);
		if (!finfo.exists())
			continue;
		QString extension = finfo.suffix().toLower();
		if (extension != tr("png") && extension != tr("jpg") && extension != tr("jpeg") && extension != tr("tif") && extension != tr("bmp"))
			continue;
		addImage(imagePathList[i]);
	}
	this->setText(text);

	QString label;
	label.append(QString("Surface Note: Number of Selected Polygon ( ") + QString::number(cellNum) + QString(" )"));
	this->setLabel(label);
	QString info(label);
	info.append(QString("\nPolygon Ids:\n"));

	for (int i = 0; i < cellNum; i++)
	{
		info.append(QString::number(mCellIds->GetSelectionList()->GetVariantValue(i).ToInt()));
		if (i != mCellIds->GetSelectionList()->GetSize() - 1)
			info.append(QString(' '));
		else
			info.append(QString("\n"));
	}
	info.append(QString("Color Type:\n"));
	info.append(colorType);
	info.append(QString("\nNote Start:"));
	this->setInfo(info);
	mFile->close();
	qDebug() << "finish Surface instructor";
	isSucceed = true;
}

void SurfaceNote::removeSurfaceNote()
{
	mFile->remove();
	this->hideNote(); 
}


FrustumNote::FrustumNote(QString path, vtkSmartPointer<vtkPoints> points, vtkSmartPointer<vtkDataArray> normals, const int noteId, const ColorType type)
	: Note(noteId, type)
{
	mPath = new QString(path);
	mFileName = new QString(path);
	mPoints  = vtkSmartPointer<vtkPoints>::New();
	mNormals = normals;
	//mFileName->append(QDir::separator() + QString("Note") + WORD_SEPARATOR + QString::number(qHash(QString::number(number))) + QString(".txt"));
	qsrand(time(NULL));
	int number = qrand();
	mFileName->append(QDir::separator() + QString("FrustumNote") + WORD_SEPARATOR + QString::number(qHash(QString::number(number))) + QString(".txt"));

	qDebug(mFileName->toLatin1());
	mFile = new QFile(*mFileName);

	mPoints->DeepCopy(points);
	mNormals->DeepCopy(normals);

	QString label;
	label.append(QString("Frustum Note: Origin Point of Each Plane:\n"));
	for (int i = 0; i < points->GetNumberOfPoints(); i++)
	{
		double point[3];
		points->GetPoint(i, point);
		label.append(QString("(") + QString::number(point[0]) + ", " + QString::number(point[1]) + ", " + QString::number(point[2]) + ")\n");
	}
	this->setLabel(label);
	
	QString info(label);
	info.append(QString("Normal Point of Each Plane:\n"));
	for (int i = 0; i < 6; i++)
	{
		info.append(QString("(") + QString::number(mNormals->GetVariantValue(i*3).ToDouble()) + ", " 
			+ QString::number(mNormals->GetVariantValue(i*3 + 1).ToDouble()) + ", " + QString::number(mNormals->GetVariantValue(i*3 + 2).ToDouble()) + ")\n");
	}
	info.append(QString("Color Type:\n"));
	info.append(QString(colortype2str(mColor).c_str()));
	info.append(QString("\nNote Start:"));
	this->setInfo(info);
	qDebug() << "finish Frustum Note instructor";
}

FrustumNote::FrustumNote(QString path, QString fileName, const int noteId, bool& isSucceed)
	: Note(noteId)
{	
	mPath = new QString(path);
	mFileName = new QString(path);
	mFileName->append(QDir::separator() + fileName);
	mPoints  = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkDoubleArray> normalArray = vtkSmartPointer<vtkDoubleArray>::New();
	mNormals = vtkDataArray::SafeDownCast(normalArray);

	qDebug(mFileName->toLatin1());
	mFile = new QFile(*mFileName);
    if (!mFile->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Open Note file " << this->mNoteId << " " << mFileName << " Failed"; 
		isSucceed = false;
		return;
	}
	
    QTextStream in(mFile);
    in.readLine();
	for (int i = 0; i < 6; i++)
	{
		QString line = in.readLine();
		if (line.split(" ").size() != 3)
		{
			qDebug() << "The Syntax of is incorrect. The First Line is " << line;
			isSucceed = false;
			return;
		}
		bool ok0, ok1, ok2;
	    double pos[3];
		pos[0] = line.split(" ")[0].split("(")[1].split(",")[0].toDouble(&ok0);
		pos[1] = line.split(" ")[1].split(",")[0].toDouble(&ok1);
		pos[2] = line.split(" ")[2].split(")")[0].toDouble(&ok2);
		if (!ok0 || !ok1 || !ok2)
		{
			qDebug() << "The Syntax of Line is incorrect. The First Line is " << line;
			isSucceed = false;
			return;
		}
		mPoints->InsertNextPoint(pos);
	}

	while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Normal Point of Each Plane:"))
			break;
	}
	mNormals->SetNumberOfComponents(3);
	mNormals->SetNumberOfTuples(6);
	for (int i = 0; i < 6; i++)
	{
		QString line = in.readLine();
		if (line.split(" ").size() != 3)
		{
			qDebug() << "The Syntax of First Line is incorrect. The First Line is " << line;
			isSucceed = false;
			return;
		}
		bool ok0, ok1, ok2;
	    double pos[3];
		pos[0] = line.split(" ")[0].split("(")[1].split(",")[0].toDouble(&ok0);
		pos[1] = line.split(" ")[1].split(",")[0].toDouble(&ok1);
		pos[2] = line.split(" ")[2].split(")")[0].toDouble(&ok2);
		if (!ok0 || !ok1 || !ok2)
		{
			qDebug() << "The Syntax of input note is incorrect. The First Line is " << line;
			isSucceed = false;
			return;
		}
		mNormals->InsertComponent(i, 0, pos[0]);
		mNormals->InsertComponent(i, 1, pos[1]);
		mNormals->InsertComponent(i, 2, pos[2]);
	}

	mNormals->Resize(6);

	while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Color Type:"))
			break;
	}
	QString colorType;
	in >> colorType;

	mColor = str2colortype(colorType.toStdString());
	while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Note Start:"))
			break;
	}

	QString content = in.readAll();
	QString text = content.split("\nLinked Images:\n")[0];
	QString imagePaths = content.split("\nLinked Images:\n")[1];
	QStringList imagePathList = imagePaths.split("\n");
	for (int i = 0; i < imagePathList.size(); i++)
	{
		qDebug()<<"Image Path"<<imagePathList[i];
		QFileInfo finfo(imagePathList[i]);
		if (!finfo.exists())
			continue;
		QString extension = finfo.suffix().toLower();
		if (extension != tr("png") && extension != tr("jpg") && extension != tr("jpeg") && extension != tr("tif") && extension != tr("bmp"))
			continue;
		addImage(imagePathList[i]);
	}
	this->setText(text);

	QString label;
	label.append(QString("Frustum Note: Origin Point of Each Plane:\n"));
	for (int i = 0; i < mPoints->GetNumberOfPoints(); i++)
	{
		double point[3];
		mPoints->GetPoint(i, point);
		label.append(QString("(") + QString::number(point[0]) + ", " + QString::number(point[1]) + ", " + QString::number(point[2]) + ")\n");
	}
	this->setLabel(label);
	QString info(label);
	info.append(QString("Normal Point of Each Plane:\n"));
	for (int i = 0; i < 6; i++)
	{
		info.append(QString("(") + QString::number(mNormals->GetVariantValue(i*3).ToDouble()) + ", " 
			+ QString::number(mNormals->GetVariantValue(i*3 + 1).ToDouble()) + ", " + QString::number(mNormals->GetVariantValue(i*3 + 2).ToDouble()) + ")\n");
	}
	info.append(QString("Color Type:\n"));
	info.append(colorType);
	info.append(QString("\nNote Start:"));
	this->setInfo(info);
	mFile->close();
	qDebug() << "finish Frustum instructor";
	isSucceed = true;
}

void FrustumNote::removeFrustumNote()
{
	mFile->remove();
	this->hideNote(); 
}

PointNote2D::PointNote2D(QString path, const double* point, const int* pointImage, const int noteId, const ColorType type)
	: Note(noteId, type)
{
	mPath = new QString(path);
    mPoint = new double[3];
	mImagePoint = new int[3];
	mFileName = new QString(path);
	//mFileName->append(QDir::separator() + QString("Note") + WORD_SEPARATOR + QString::number(qHash(QString::number(number))) + QString(".txt"));
	qsrand(time(NULL));
	int number = qrand();
	mFileName->append(QDir::separator() + QString("PointNote2D") + WORD_SEPARATOR + QString::number(qHash(QString::number(number))) + QString(".txt"));

	qDebug(mFileName->toLatin1());
	mFile = new QFile(*mFileName);
	for (int i = 0; i < 3; i++)
	{
		mPoint[i] = point[i];
		mImagePoint[i] = pointImage[i];
	}

	QString label;
	label.append(QString("Point Note: World Coordinate (") + QString::number(point[0]) + QString(", ") + QString::number(point[1]) + QString(")")); 
	this->setLabel(label);
	QString info;
	info.append(QString("Point Note: World Coordinate (") + QString::number(point[0]) + QString(", ") + QString::number(point[1]) + QString(") Image Coordinate (")
		+ QString::number(pointImage[0]) + QString(", ") + QString::number(pointImage[1]) + QString(")"));

	info.append(QString("\nColor Type:\n"));
	info.append(QString(colortype2str(mColor).c_str()));
	info.append(QString("\nNote Start:"));
	this->setInfo(info);
	qDebug() << "finish Point Note 2D instructor";
}

PointNote2D::PointNote2D(QString path, QString fileName, const int noteId, bool& isSucceed)
	: Note(noteId)
{	
	mPath = new QString(path);
	mPoint = new double[3];
	mImagePoint = new int[3];
	vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
	
	mFileName = new QString(path);
	mFileName->append(QDir::separator() + fileName);

	qDebug(mFileName->toLatin1());
	mFile = new QFile(*mFileName);
    if (!mFile->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Open Note file " << this->mNoteId << " " << mFileName << " Failed"; 
		isSucceed = false;
		return;
	}
	
    QTextStream in(mFile);
    QString firstLine = in.readLine();
	if (firstLine.split(" ").size() != 10)
	{
		qDebug() << "The Syntax of First Line is incorrect. The First Line is " << firstLine;
		isSucceed = false;
		return;
	}

	bool ok0, ok1;
	double pos[3];
	pos[0] = firstLine.split(" ")[4].split(",")[0].split("(")[1].toDouble(&ok0);
	pos[1] = firstLine.split(" ")[5].split(")")[0].toDouble(&ok1);
	pos[2] = 0;
	
	if (!ok0 || !ok1)
	{
		qDebug() << "The Syntax of First Line is incorrect. The First Line is " << firstLine;
		isSucceed = false;
		return;
	}

	int posImage[3];
	posImage[0] = firstLine.split(" ")[8].split(",")[0].split("(")[1].toInt(&ok0);
	posImage[1] = firstLine.split(" ")[9].split(")")[0].toInt(&ok1);
	posImage[2] = 0;
	
	if (!ok0 || !ok1)
	{
		qDebug() << "The Syntax of First Line is incorrect. The First Line is " << firstLine;
		isSucceed = false;
		return;
	}
	for (int i = 0; i < 3; i++)
	{
		mPoint[i] = pos[i];
		mImagePoint[i] = posImage[i];
	}

	while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Color Type:"))
			break;
	}
	QString colorType;
	in >> colorType;

	mColor = str2colortype(colorType.toStdString());
	while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Note Start:"))
			break;
	}

	QString content = in.readAll();
	QString text = content.split("\nLinked Images:\n")[0];
	QString imagePaths = content.split("\nLinked Images:\n")[1];
	QStringList imagePathList = imagePaths.split("\n");
	for (int i = 0; i < imagePathList.size(); i++)
	{
		qDebug()<<"Image Path"<<imagePathList[i];
		QFileInfo finfo(imagePathList[i]);
		if (!finfo.exists())
			continue;
		QString extension = finfo.suffix().toLower();
		if (extension != tr("png") && extension != tr("jpg") && extension != tr("jpeg") && extension != tr("tif") && extension != tr("bmp"))
			continue;
		addImage(imagePathList[i]);
	}
	this->setText(text);

	QString label;
	label.append(QString("Point Note: Center (") + QString::number(mPoint[0]) + QString(", ") + QString::number(mPoint[1]) + QString(")"));
	this->setLabel(label);
	QString info;
	info.append(QString("Point Note: World Coordinate (") + QString::number(mPoint[0]) + QString(", ") + QString::number(mPoint[1]) + QString(") Image Coordinate (")
		+ QString::number(mImagePoint[0]) + QString(", ") + QString::number(mImagePoint[1]) + QString(")"));

	info.append(QString("\nColor Type:\n"));
	info.append(colorType);
	info.append(QString("\nNote Start:"));
	this->setInfo(info);
	mFile->close();
	qDebug() << "finish Point instructor";
	isSucceed = true;
}

void PointNote2D::removePointNote2D()
{
	qDebug()<<"remove Point note 2d"<<mFile->fileName();
	mFile->remove();
	this->hideNote(); 
}

SurfaceNote2D::SurfaceNote2D(QString path, const double* point, const int* pointImage, const int noteId, const ColorType type)
	: Note(noteId, type)
{
	mPath = new QString(path);
    mPoint = new double[4];
	mImagePoint = new int[4];
	mFileName = new QString(path);
	//mFileName->append(QDir::separator() + QString("Note") + WORD_SEPARATOR + QString::number(qHash(QString::number(number))) + QString(".txt"));
	qsrand(time(NULL));
	int number = qrand();
	mFileName->append(QDir::separator() + QString("SurfaceNote2D") + WORD_SEPARATOR + QString::number(qHash(QString::number(number))) + QString(".txt"));

	qDebug(mFileName->toLatin1());
	mFile = new QFile(*mFileName);
	for (int i = 0; i < 4; i++)
	{
		mPoint[i] = point[i];
		mImagePoint[i] = pointImage[i];
	}

	QString label;
	label.append(QString("Surface Note: Start (") + QString::number(point[0]) + QString(", ") + QString::number(point[1]) + QString(") End (") 
		+ QString::number(point[2]) + QString(", ") + QString::number(point[3]) + QString(")"));
	this->setLabel(label);
	QString info(label);
	info.append(" Image Coordinate Start (" + QString::number(pointImage[0]) + QString(", ") + QString::number(pointImage[1]) + QString(") End (") 
		+ QString::number(pointImage[2]) + QString(", ") + QString::number(pointImage[3]) + QString(")"));

	info.append(QString("\nColor Type:\n"));
	info.append(QString(colortype2str(mColor).c_str()));
	info.append(QString("\nNote Start:"));
	this->setInfo(info);
	qDebug() << "finish Surface Note 2D instructor";
}

SurfaceNote2D::SurfaceNote2D(QString path, QString fileName, const int noteId, bool& isSucceed)
	: Note(noteId)
{	
	mPath = new QString(path);
	mPoint = new double[4];
	mImagePoint = new int[4];
	vtkSmartPointer<vtkIdTypeArray> ids = vtkSmartPointer<vtkIdTypeArray>::New();
	
	mFileName = new QString(path);
	mFileName->append(QDir::separator() + fileName);

	qDebug(mFileName->toLatin1());
	mFile = new QFile(*mFileName);
    if (!mFile->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		qDebug() << "Open Note file " << this->mNoteId << " " << mFileName << " Failed"; 
		isSucceed = false;
		return;
	}
	
    QTextStream in(mFile);
    QString firstLine = in.readLine();
	if (firstLine.split(" ").size() != 16)
	{
		qDebug() << "The Syntax of First Line is incorrect. The First Line is " << firstLine;
		isSucceed = false;
		return;
	}
	bool ok0, ok1, ok2, ok3;
	double pos[4];
	pos[0] = firstLine.split(" ")[3].split(",")[0].split("(")[1].toDouble(&ok0);
	pos[1] = firstLine.split(" ")[4].split(")")[0].toDouble(&ok1);
	pos[2] = firstLine.split(" ")[6].split(",")[0].split("(")[1].toDouble(&ok2);
	pos[3] = firstLine.split(" ")[7].split(")")[0].toDouble(&ok3);
	
	if (!ok0 || !ok1 || !ok2 || !ok3)
	{
		qDebug() << "The Syntax of First Line is incorrect. The First Line is " << firstLine;
		isSucceed = false;
		return;
	}
	int posImage[4];
	posImage[0] = firstLine.split(" ")[11].split(",")[0].split("(")[1].toInt(&ok0);
	posImage[1] = firstLine.split(" ")[12].split(")")[0].toInt(&ok1);
	posImage[2] = firstLine.split(" ")[14].split(",")[0].split("(")[1].toInt(&ok2);
	posImage[3] = firstLine.split(" ")[15].split(")")[0].toInt(&ok3);
	
	if (!ok0 || !ok1 || !ok2 || !ok3)
	{
		qDebug() << "The Syntax of First Line is incorrect. The First Line is " << firstLine;
		isSucceed = false;
		return;
	}
	for (int i = 0; i < 4; i++)
	{
		mPoint[i] = pos[i];
		mImagePoint[i] = posImage[i];
	}

	while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Color Type:"))
			break;
	}
	QString colorType;
	in >> colorType;

	mColor = str2colortype(colorType.toStdString());
	while(1)
	{
		QString signal = in.readLine();
		if (signal == QString("Note Start:"))
			break;
	}

	QString content = in.readAll();
	QString text = content.split("\nLinked Images:\n")[0];
	QString imagePaths = content.split("\nLinked Images:\n")[1];
	QStringList imagePathList = imagePaths.split("\n");
	for (int i = 0; i < imagePathList.size(); i++)
	{
		qDebug()<<"Image Path"<<imagePathList[i];
		QFileInfo finfo(imagePathList[i]);
		if (!finfo.exists())
			continue;
		QString extension = finfo.suffix().toLower();
		if (extension != tr("png") && extension != tr("jpg") && extension != tr("jpeg") && extension != tr("tif") && extension != tr("bmp"))
			continue;
		addImage(imagePathList[i]);
	}
	this->setText(text);

	QString label;
	label.append(QString("Surface Note: Start (") + QString::number(mPoint[0]) + QString(", ") + QString::number(mPoint[1]) + QString(") End (") 
		+ QString::number(mPoint[2]) + QString(", ") + QString::number(mPoint[3]) + QString(")"));
	this->setLabel(label);
	QString info(label);
	info.append(" Image Coordinate Start (" + QString::number(mImagePoint[0]) + QString(", ") + QString::number(mImagePoint[1]) + QString(") End (") 
		+ QString::number(mImagePoint[2]) + QString(", ") + QString::number(mImagePoint[3]) + QString(")"));

	info.append(QString("\nColor Type:\n"));
	info.append(colorType);
	info.append(QString("\nNote Start:"));
	this->setInfo(info);
	mFile->close();
	qDebug() << "finish Surface instructor";
	isSucceed = true;
}

void SurfaceNote2D::removeSurfaceNote2D()
{
	mFile->remove();
	this->hideNote(); 
}




