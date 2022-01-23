#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QGraphicsTextItem"
#include <set>

//можем контролировать радиус вершины и радиус в т.н. уравнении круга
#define ellipseRadius 30
#define radius 120

//кисти, ручки
QBrush whiteBrush(Qt::white);

QPen bluepen(Qt::blue);
QPen graypen(Qt::gray);
QPen bluepenDot(Qt::blue);

//два вектора с цветами для ребёр и вершин
std::vector <QBrush> brushesForVertices;
std::vector <QPen> pensForEdges;

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
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);//сглаживание в сцене (кайф)

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

//функция для построения вершин
void MainWindow::paintVerticies(int numberOfverticies){
    if (numberOfverticies==0)
        return;
    for (int i=1; i<=numberOfverticies; ++i){
        scene->addEllipse(radius*cos(i),radius*sin(i),ellipseRadius,ellipseRadius,graypen,whiteBrush);
        QGraphicsTextItem *text = scene->addText(QString::number(i));
        text->setPos(radius*cos(i)+7,radius*sin(i)+3);
    }
};

//функция для построения одной вершины с определённым цветом
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

//Построить граф из матрицы
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

    //проходим по таблице, запоминаем связи между вершинами
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

    //для каждой связи в векторе, строим рёбра
    for (Connections item : edges) {
        if(item.first==item.second){
            scene->addEllipse(radius*cos(item.first)-5,radius*sin(item.second)-5,ellipseRadius+5,ellipseRadius+5,bluepenDot);
        }else{
            scene->addLine(radius*cos(item.first)+7,radius*sin(item.first)+3,
                           radius*cos(item.second)+7,radius*sin(item.second)+3,bluepen);
        }
    }
    //после рёбер строим поверх вершины
    paintVerticies(ui->tableSum->columnCount());
}

//при нажатии на клетку добавит зеркальный элемент
void MainWindow::on_tableSum_cellClicked(int row, int column)
{
    QTableWidgetItem *mirrored = new QTableWidgetItem();
    mirrored->setText(ui->tableSum->item(row,column)->text());
    mirrored->setTextAlignment(Qt::AlignHCenter);
    ui->tableSum->setItem(column,row,mirrored);
}

//функция-проверка, можно ли построить цикл
bool MainWindow::canBuildEilerCycle(){
    int numberOfOnes;
    for(int i=0; i<ui->tableSum->rowCount(); ++i){
        numberOfOnes = 0;
        for(int j=0; j<ui->tableSum->rowCount(); ++j){
            if(i==j){
                continue;
            }
            if(ui->tableSum->item(i,j)->text()=="1"){
                ++numberOfOnes;
            }
        }
        if(numberOfOnes%2!=0){
            return false;
        }
    }
    return true;
}

//функция поиска следующей связи в векторе всех связей графа
void checkVector(std::vector <Connections> &tempEdges, int &findThis, Connections &item){
    for (int i=(int)tempEdges.size()-1; i>=0; --i){
        if(tempEdges[i].first==findThis){
            item=tempEdges[i];
            findThis=tempEdges[i].second;
            tempEdges.erase(tempEdges.begin()+i);
            return;
        }
    }
    for (int i=(int)tempEdges.size()-1; i>=0; --i){
        if(tempEdges[i].second==findThis){
            item.first=tempEdges[i].second;
            item.second=tempEdges[i].first;
            findThis=tempEdges[i].first;
            tempEdges.erase(tempEdges.begin()+i);
            return;
        }
    }
}

void MainWindow::on_pushButton_3_clicked()//Эйлеровий цикл
{
    ui->listWidget->clear();

    //Проверка, можно ли вообще построить цикл
    if(!canBuildEilerCycle()){
        QListWidgetItem *canNotBuild = new QListWidgetItem;
        canNotBuild->setText("Цикл неможливо побудувати");
        ui->listWidget->addItem(canNotBuild);
        return;
    }

    //Скопируем все связи в другой вектор
    std::vector <Connections> tempEdges=edges;

    //Удаляет вершины, инцидентные себе, во втором векторе (они нам не нужны)
    for (int i=0;i<(int)tempEdges.size() ;++i ) {
        if(tempEdges[i].first==tempEdges[i].second){
            tempEdges.erase(tempEdges.begin()+i);
        }
    }

    //Запоминаем первую пару
    QListWidgetItem *firstPair = new QListWidgetItem;
    firstPair->setText(QString::number(tempEdges[0].first)+"-"+QString::number(tempEdges[0].second)+ " : y" + QString::number(1));
    ui->listWidget->addItem(firstPair);

    int compare = tempEdges[0].second;
    tempEdges.erase(tempEdges.begin());

    int listCounter = 1;//Счётчик для listWidget
    do{
        listCounter++;
        Connections item;
        checkVector(tempEdges,compare,item);
        QListWidgetItem *listChild = new QListWidgetItem;
        listChild->setText(QString::number(item.first)+"-"+QString::number(item.second)+ " : y" + QString::number(listCounter));
        ui->listWidget->addItem(listChild);
    }while(!tempEdges.empty());

}

static bool contains(int value, std::vector<int> vec){
    for(int i = 0; i < (int) vec.size(); i++){
        if(vec[i] == value){
            return true;
        }
    }
    return false;
}

//разукрасить вершины
void MainWindow::on_pushButton_4_clicked()
{
    //генерируем случайные цвета, n = количеству вершин
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

    //сначала строим рёбра, поверх них будем строить вершины
    for (Connections item : edges) {
        if(item.first==item.second){
            scene->addEllipse(radius*cos(item.first)-5,radius*sin(item.second)-5,ellipseRadius+5,ellipseRadius+5,bluepenDot);
        }else{
            scene->addLine(radius*cos(item.first)+7,radius*sin(item.first)+3,
                           radius*cos(item.second)+7,radius*sin(item.second)+3,bluepen);
        }
    }

    //скопируем все связи во временный вектор
    std::vector <Connections> tempEdges=edges;

    std::set <int> usedVertices;//тут хранятся вершины, которые надо разукрасить
    std::vector <int> paintedVertices;//тут будут храниться разукрашенные вершины
    std::vector <int> bannedVertices;//тут будут временно храниться вершины, которые пока не надо разукрашивать
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

//поиск следующей вершины для покраски
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

//разукрасить рёбра
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

//построить ребро с определённым цветом
void MainWindow::buildEdge(int x, int y, QPen pen){
    scene->addLine(radius*cos(x)+7,radius*sin(x)+3,
                   radius*cos(y)+7,radius*sin(y)+3,pen);
}

//поиск следующего ребра для покраски
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

