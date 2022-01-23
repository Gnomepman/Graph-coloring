#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QGraphicsTextItem"
#include <set>

//can control radius of vertices and distance between them
#define ellipseRadius 30
#define radius 120

//brushes, pens
QBrush whiteBrush(Qt::white);

QPen bluepen(Qt::blue);
QPen graypen(Qt::gray);
QPen bluepenDot(Qt::blue);

//two vectors for colors for edges and vertices
std::vector <QBrush> brushesForVertices;
std::vector <QPen> pensForEdges;
//vector to store all connections
std::vector <Connections> edges;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->spinBox->setMinimum(0);

    scene = new QGraphicsScene(this);
    scene->clearFocus();
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);//anti-aliasing in scene

    graypen.setWidth(3);
    bluepen.setWidth(2);
    bluepenDot.setWidth(2);
    bluepen.setCapStyle(Qt::RoundCap);
    bluepenDot.setStyle(Qt::DashLine);
}

MainWindow::~MainWindow()
{
    delete ui;
}

//function for building vertices
void MainWindow::paintVerticies(int numberOfverticies){
    if (numberOfverticies==0)
        return;
    for (int i=1; i<=numberOfverticies; ++i){
        scene->addEllipse(radius*cos(i),radius*sin(i),ellipseRadius,ellipseRadius,graypen,whiteBrush);
        QGraphicsTextItem *text = scene->addText(QString::number(i));
        text->setPos(radius*cos(i)+7,radius*sin(i)+3);
    }
};

//function for building one vertex with specific color
void MainWindow::paintVerticies(int position, QBrush brush){
    if (position==0){
        return;
    }
    scene->addEllipse(radius*cos(position),radius*sin(position),ellipseRadius,ellipseRadius,graypen,brush);
    QGraphicsTextItem *text = scene->addText(QString::number(position));
    text->setPos(radius*cos(position)+7,radius*sin(position)+3);
};


void MainWindow::on_spinBox_valueChanged(int arg1)
{
    ui->tableSum->setRowCount(arg1);
    ui->tableSum->setColumnCount(arg1);

    for (int i=0;i<ui->tableSum->rowCount();++i){
        for (int j=0;j<ui->tableSum->columnCount();++j) {
            ui->tableSum->setColumnWidth(i,10);
            QTableWidgetItem *Zero = new QTableWidgetItem();
            Zero->setText(QString::fromStdString("0"));
            Zero->setTextAlignment(Qt::AlignHCenter);
            ui->tableSum->setItem(i,j,Zero);
        }
    }
}


void MainWindow::on_pushButton_clicked()
{
    ui->tableSum->clear();
    ui->spinBox->setValue(0);
}

//build graph from matrix
void MainWindow::on_pushButton_2_clicked()
{
    edges.clear();
    scene->clear();

    for (int i=0 ; i < ui->tableSum->rowCount(); ++i){
        QPen temp(QColor(rand()%256,rand()%256,rand()%256));
        QBrush tempB(QColor(rand()%256,rand()%256,rand()%256));
        pensForEdges.push_back(temp);
        brushesForVertices.push_back(tempB);
    }

    //going threw table, remembering all connections
    for(int i=0; i<ui->tableSum->rowCount(); ++i){
        for(int j=0; j<ui->tableSum->rowCount(); ++j){
            if (i>j){//проход только выше главной диагонали, т.к. граф неориентированный
                continue;
            }
            if(ui->tableSum->item(i,j)->text()=="1"){
                Connections pair;
                pair.first=i+1;
                pair.second=j+1;
                edges.push_back(pair);
            }
        }
    }

    //for every connection build edge
    for (Connections item : edges) {
        if(item.first==item.second){
            scene->addEllipse(radius*cos(item.first)-5,radius*sin(item.second)-5,ellipseRadius+5,ellipseRadius+5,bluepenDot);
        }else{
            scene->addLine(radius*cos(item.first)+7,radius*sin(item.first)+3,
                           radius*cos(item.second)+7,radius*sin(item.second)+3,bluepen);
        }
    }
    //then build vertices on top of edges
    paintVerticies(ui->tableSum->columnCount());
}

void MainWindow::on_tableSum_cellClicked(int row, int column)
{
    QTableWidgetItem *mirrored = new QTableWidgetItem();
    mirrored->setText(ui->tableSum->item(row,column)->text());
    mirrored->setTextAlignment(Qt::AlignHCenter);
    ui->tableSum->setItem(column,row,mirrored);
}

static bool contains(int value, std::vector<int> vec){
    for(int i = 0; i < (int) vec.size(); i++){
        if(vec[i] == value){
            return true;
        }
    }
    return false;
}

//color vertices
void MainWindow::on_pushButton_4_clicked()
{
    //generating random color, n = number of vertices
    brushesForVertices.clear();
    for (int i=0 ; i < ui->tableSum->rowCount(); ++i){
        QBrush temp(QColor(rand()%256,rand()%256,rand()%256));
        brushesForVertices.push_back(temp);
    }

    if(edges.empty()){
        for (int i=0; i < ui->tableSum->rowCount(); ++i){
             paintVerticies(i+1, brushesForVertices[i]);
        }
        return;
    }
    scene->clear();

    //build edges first, will build colored vertices on top
    for (Connections item : edges) {
        if(item.first==item.second){
            scene->addEllipse(radius*cos(item.first)-5,radius*sin(item.second)-5,ellipseRadius+5,ellipseRadius+5,bluepenDot);
        }else{
            scene->addLine(radius*cos(item.first)+7,radius*sin(item.first)+3,
                           radius*cos(item.second)+7,radius*sin(item.second)+3,bluepen);
        }
    }

    //copying all connections in temporary vector
    std::vector <Connections> tempEdges=edges;

    std::set <int> usedVertices;//here will be stored vertices, that needs to be painted
    std::vector <int> paintedVertices;//here will be stored already painted vertices
    std::vector <int> bannedVertices;//here will be stored which temporary will not be bainted
    for(int i = 0; i < (int) tempEdges.size(); i++){
        usedVertices.insert(tempEdges[i].first);
        usedVertices.insert(tempEdges[i].second);
    }

    int currentVert=0;

    do{
        currentVert = *(usedVertices.begin());
        findNextVertex(currentVert,bannedVertices,tempEdges, usedVertices, paintedVertices);
    }while(!usedVertices.empty());
}

//search for next vertex to be painted
void MainWindow::findNextVertex (int &currentVert, std::vector <int> &bannedVertices,
                     std::vector <Connections> &tempEdges, std::set <int> &usedVertices, std::vector <int> &paintedVertices){

    paintVerticies(currentVert, brushesForVertices[0]);

    bannedVertices.push_back(currentVert);
    paintedVertices.push_back(currentVert);

    for (int i=0; i<(int)tempEdges.size(); i++){
        if(tempEdges[i].first==currentVert){
            bannedVertices.push_back(tempEdges[i].second);
        }
    }

    for (int i=0; i<(int)tempEdges.size(); i++){
        if(contains(tempEdges[i].first,bannedVertices)){
            continue;
        }

        currentVert=tempEdges[i].first;
        usedVertices.erase(currentVert);
        findNextVertex(currentVert,bannedVertices,tempEdges, usedVertices, paintedVertices);
        return;
    }

    for (int i=0; i<(int)paintedVertices.size(); i++){
        usedVertices.erase(paintedVertices[i]);
    }
    brushesForVertices.erase(brushesForVertices.begin());
    bannedVertices.clear();

    for (int i=0; i<(int)tempEdges.size(); i++){
        for (int j=0; j<(int)paintedVertices.size(); j++){
            if(tempEdges[i].first==paintedVertices[j]){
                tempEdges.erase(tempEdges.begin()+i);
                i--;
            }
        }
    }
}

//Color edges
void MainWindow::on_pushButton_5_clicked()
{
    if(edges.empty()){
        return;
    }

    pensForEdges.clear();
    scene->clear();
    for (int i=0 ; i < pow(ui->tableSum->rowCount(),2); ++i){
        QPen temp(QColor(rand()%256,rand()%256,rand()%256));
        temp.setWidth(3);
        temp.setCapStyle(Qt::RoundCap);
        pensForEdges.push_back(temp);
    }

    std::vector <Connections> tempEdges=edges;
    std::vector <int> bannedVertices;

    int compFirst=0, compSecond=0;
    while(!tempEdges.empty()){
        compFirst=tempEdges[0].first;
        compSecond=tempEdges[0].second;
        findNextEdge(compFirst,compSecond, bannedVertices,tempEdges);
    }
    paintVerticies(ui->tableSum->columnCount());
}

//build edge with specific color
void MainWindow::buildEdge(int x, int y, QPen pen){
    scene->addLine(radius*cos(x)+7,radius*sin(x)+3,
                   radius*cos(y)+7,radius*sin(y)+3,pen);
}

//search for next edge to be painted
void MainWindow::findNextEdge(int &first,int &second, std::vector <int> &bannedVertices,
                              std::vector <Connections> &tempEdges){
    buildEdge(first,second,pensForEdges[0]);
    bannedVertices.push_back(first);
    bannedVertices.push_back(second);

    //удаляем разукрашенное ребро из списка
    for (int i=0;i<(int) tempEdges.size();i++){
        if(tempEdges[i].first==first && tempEdges[i].second==second){
            tempEdges.erase(tempEdges.begin()+i);
            break;
        }
    }

    for (int i=0;i<(int) tempEdges.size();i++){
        if(contains(tempEdges[i].first,bannedVertices) || contains(tempEdges[i].second,bannedVertices)){
            continue;
        }
        findNextEdge(tempEdges[i].first,tempEdges[i].second, bannedVertices,tempEdges);
        return;
    }
    pensForEdges.erase(pensForEdges.begin());
}

