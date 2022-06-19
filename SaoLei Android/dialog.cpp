#include "dialog.h"
#include "ui_dialog.h"

//构造函数
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setFixedSize(1080,2400);//窗口分辨率
    setWindowTitle(tr("扫雷"));//窗口标题

    Qt::WindowFlags flags=Qt::Dialog;
    flags |=Qt::WindowMinMaxButtonsHint;//添加最小化最大化按钮（最大化不可用）
    flags |=Qt::WindowCloseButtonHint;//启用关闭按钮
    setWindowFlags(flags);

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//表格只读
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);//只选一个
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);//只能选格

    ui->tableWidget->verticalHeader()->setVisible(false); //设置垂直头不可见
    ui->tableWidget->horizontalHeader()->setVisible(false); //设置水平头不可见

    start = new QMediaPlayer;
    BGM = new QMediaPlayer;
    win = new QMediaPlayer;
    lost = new QMediaPlayer;

    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(100);
    ui->horizontalSlider->setValue(50);

    start->setVolume(volumn);
    BGM->setVolume(volumn);
    win->setVolume(volumn);
    lost->setVolume(volumn);
    start->setMedia(QUrl("qrc:/files/music/win2000.mp3"));
    BGM->setMedia(QUrl("qrc:/files/music/Funky Star.mp3"));
    win->setMedia(QUrl("qrc:/files/music/win3.1.mp3"));
    lost->setMedia(QUrl("qrc:/files/music/th03_OVER.mp3"));

    start->play();

    connect(BGM,SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),this,SLOT(Paly_InitStatus(QMediaPlayer::MediaStatus)));

    ui->spinBox_2->setMinimum(4);//设置最小值
    GroundSize=10;
    ui->spinBox_2->setValue(10);
    ui->spinBox_2->setMaximum(20);//设置最大值

    font=15;
    ui->tableWidget->setFont(QFont("song", font));

    ui->spinBox->setMinimum(1);//设置最小值
    ui->spinBox->setValue(10);
    ui->spinBox->setMaximum(91);//设置最大值
}

void Dialog::Paly_InitStatus(QMediaPlayer::MediaStatus status)//从头播放音乐
{
    if(status == QMediaPlayer::EndOfMedia)
    {
        BGM->setPosition(0);
        BGM->play();
    }
}

Dialog::~Dialog()//析构函数
{
    delete ui;
}

int Dialog::TypeRefresh(int row, int column)//计算cells里不是雷的格子的Type应该更新为多少（至少2x2才有意义）
{
    if(row<0||row>=GroundSize||column<0||column>=GroundSize)
        return -1;
    if(row==0&&column==0)
        return (cells[row][column].Type<0?1:0)+(cells[row+1][column].Type<0?1:0)+(cells[row][column+1].Type<0?1:0)+(cells[row+1][column+1].Type<0?1:0);
    else if(row+1==GroundSize&&column+1==GroundSize)
        return (cells[row][column].Type<0?1:0)+(cells[row-1][column].Type<0?1:0)+(cells[row][column-1].Type<0?1:0)+(cells[row-1][column-1].Type<0?1:0);
    else if(row==0&&column+1==GroundSize)
        return (cells[row][column].Type<0?1:0)+(cells[row+1][column].Type<0?1:0)+(cells[row][column-1].Type<0?1:0)+(cells[row+1][column-1].Type<0?1:0);
    else if(row+1==GroundSize&&column==0)
        return (cells[row][column].Type<0?1:0)+(cells[row-1][column].Type<0?1:0)+(cells[row][column+1].Type<0?1:0)+(cells[row-1][column+1].Type<0?1:0);
    else if(row==0)
        return (cells[row][column].Type<0?1:0)+(cells[row+1][column].Type<0?1:0)+(cells[row][column+1].Type<0?1:0)+(cells[row+1][column+1].Type<0?1:0)+(cells[row][column-1].Type<0?1:0)+(cells[row+1][column-1].Type<0?1:0);
    else if(column==0)
        return (cells[row][column].Type<0?1:0)+(cells[row+1][column].Type<0?1:0)+(cells[row][column+1].Type<0?1:0)+(cells[row+1][column+1].Type<0?1:0)+(cells[row-1][column].Type<0?1:0)+(cells[row-1][column+1].Type<0?1:0);
    else if(row+1==GroundSize)
        return (cells[row][column].Type<0?1:0)+(cells[row-1][column].Type<0?1:0)+(cells[row][column-1].Type<0?1:0)+(cells[row-1][column-1].Type<0?1:0)+(cells[row][column+1].Type<0?1:0)+(cells[row-1][column+1].Type<0?1:0);
    else if(column+1==GroundSize)
        return (cells[row][column].Type<0?1:0)+(cells[row-1][column].Type<0?1:0)+(cells[row][column-1].Type<0?1:0)+(cells[row-1][column-1].Type<0?1:0)+(cells[row+1][column].Type<0?1:0)+(cells[row+1][column-1].Type<0?1:0);
    else
        return (cells[row-1][column-1].Type<0?1:0)+(cells[row][column-1].Type<0?1:0)+(cells[row+1][column-1].Type<0?1:0)+(cells[row-1][column].Type<0?1:0)+(cells[row][column].Type<0?1:0)+(cells[row+1][column].Type<0?1:0)+(cells[row-1][column+1].Type<0?1:0)+(cells[row][column+1].Type<0?1:0)+(cells[row+1][column+1].Type<0?1:0);
}

void Dialog::Two_Pass()//两遍扫描算法找到所有Type==0格子的连通域
{
    int Lable=0;
    vector<int> Parent;
    //First Pass
    for(int i=0;i<GroundSize;++i)
        for(int j=0;j<GroundSize;++j)
            if(cells[i][j].Type==0)
            {
                if(i==0&&j==0)
                {
                    cells[i][j].lable=Lable++;
                    Parent.push_back(-2);
                }
                else if(i==0)
                    if(cells[i][j-1].Type==0)
                        cells[i][j].lable=cells[i][j-1].lable;
                    else
                    {
                        cells[i][j].lable=Lable++;
                        Parent.push_back(-2);
                    }
                else if(j==0)
                    if(cells[i-1][j].Type==0)
                        cells[i][j].lable=cells[i-1][j].lable;
                    else
                    {
                        cells[i][j].lable=Lable++;
                        Parent.push_back(-2);
                    }
                else if(cells[i-1][j].Type==0&&cells[i][j-1].Type==0)//要避免循环爹出现，必须是严格的树状结构，小数为大树树根
                {
                    cells[i][j].lable=MIN(cells[i-1][j].lable,cells[i][j-1].lable);
                    if(cells[i-1][j].lable!=cells[i][j-1].lable)
                    {
                        int childmax=MAX(cells[i-1][j].lable,cells[i][j-1].lable);
                        while(Parent[childmax]!=-2)
                            childmax=Parent[childmax];
                        int childmin=MIN(cells[i-1][j].lable,cells[i][j-1].lable);
                        while(Parent[childmin]!=-2)
                            childmin=Parent[childmin];
                        if(childmin!=childmax)
                            Parent[MAX(childmax,childmin)]=MIN(childmax,childmin);
                    }
                }
                else if(cells[i-1][j].Type==0)
                    cells[i][j].lable=cells[i-1][j].lable;
                else if(cells[i][j-1].Type==0)
                    cells[i][j].lable=cells[i][j-1].lable;
                else
                {
                    cells[i][j].lable=Lable++;
                    Parent.push_back(-2);
                }
            }
    //Second Pass
    int current;
    for(int i=0;i<GroundSize;++i)
        for(int j=0;j<GroundSize;++j)
            if(cells[i][j].Type==0)
            {
                current=cells[i][j].lable;
                while(Parent[current]!=-2)
                    current=Parent[current];
                cells[i][j].lable=current;
            }
}

void Dialog::on_pushButton_clicked()//开始游戏，初始化所有参数和图形界面
{
    start->stop();
    win->stop();
    win->setPosition(0);
    lost->stop();
    lost->setPosition(0);
    GroundSize=ui->spinBox_2->value();
    font=(ui->tableWidget->horizontalHeader()->width()/GroundSize)*(0.15);
    //设置最大值
    ui->spinBox->setMaximum(GroundSize*GroundSize-9);
    mines=ui->spinBox->value();
    for(int i=0;i<GroundSize;++i)
        for(int j=0;j<GroundSize;++j)
        {
            cells[i][j].Type=0;
            cells[i][j].lable=-1;
            cells[i][j].State=false;
            flag[i][j]=false;
        }
    refreshed=0;
    counter=0;
    clicked=false;
    ui->label_4->setText(QString::number(mines));
    ui->label_5->setText(QString::number(counter));
    ui->label_6->setText(QString::number(GroundSize*GroundSize));
    ui->tableWidget->setFont(QFont("song", font));//必须在前设置字体，不然表格大小可能不正常
    ui->tableWidget->setColumnCount(GroundSize); //设置列数
    ui->tableWidget->setRowCount(GroundSize); //设置行数

    QFont qfont;
    qfont.setBold(true);
    for(int i=0;i<GroundSize;++i)
        ui->tableWidget->setColumnWidth(i,ui->tableWidget->horizontalHeader()->width()/GroundSize);
    for(int i=0;i<GroundSize;++i)
        ui->tableWidget->setRowHeight(i,ui->tableWidget->verticalHeader()->height()/GroundSize);
    for(int i=0;i<GroundSize;++i)
    {
        //ui->tableWidget->setRowHeight(i,ui->tableWidget->verticalHeader()->height()/GroundSize);
        for(int j=0;j<GroundSize;++j)
        {
            ui->tableWidget->setItem(i, j, new QTableWidgetItem);
            ui->tableWidget->item(i,j)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
            ui->tableWidget->item(i, j)->setFont(qfont);
            ui->tableWidget->item(i, j)->setBackground(Qt::gray);
        }
    }
    BGM->play();
}

bool Dialog::NextTo(int row, int column,int lable)//判断Type>0的格子是否在所点开的连通域旁（8邻接）
{
    if(row<0||row>=GroundSize||column<0||column>=GroundSize||lable<0)
        return false;
    if(row==0&&column==0)
        return (cells[row+1][column].lable==lable)||(cells[row][column+1].lable==lable)||(cells[row+1][column+1].lable==lable);
    else if(row+1==GroundSize&&column+1==GroundSize)
        return (cells[row-1][column].lable==lable)||(cells[row][column-1].lable==lable)||(cells[row-1][column-1].lable==lable);
    else if(row==0&&column+1==GroundSize)
        return (cells[row+1][column].lable==lable)||(cells[row][column-1].lable==lable)||(cells[row+1][column-1].lable==lable);
    else if(row+1==GroundSize&&column==0)
        return (cells[row-1][column].lable==lable)||(cells[row][column+1].lable==lable)||(cells[row-1][column+1].lable==lable);
    else if(row==0)
        return (cells[row+1][column].lable==lable)||(cells[row][column+1].lable==lable)||(cells[row][column-1].lable==lable)||(cells[row+1][column-1].lable==lable)||(cells[row+1][column+1].lable==lable);
    else if(column==0)
        return (cells[row+1][column].lable==lable)||(cells[row-1][column].lable==lable)||(cells[row][column+1].lable==lable)||(cells[row+1][column+1].lable==lable)||(cells[row-1][column+1].lable==lable);
    else if(row+1==GroundSize)
        return (cells[row-1][column].lable==lable)||(cells[row][column+1].lable==lable)||(cells[row][column-1].lable==lable)||(cells[row-1][column-1].lable==lable)||(cells[row-1][column+1].lable==lable);
    else if(column+1==GroundSize)
        return (cells[row+1][column].lable==lable)||(cells[row-1][column].lable==lable)||(cells[row][column-1].lable==lable)||(cells[row-1][column-1].lable==lable)||(cells[row+1][column-1].lable==lable);
    else
        return (cells[row+1][column].lable==lable)||(cells[row-1][column].lable==lable)||(cells[row][column+1].lable==lable)||(cells[row][column-1].lable==lable)||(cells[row+1][column+1].lable==lable)||(cells[row-1][column-1].lable==lable)||(cells[row+1][column-1].lable==lable)||(cells[row-1][column+1].lable==lable);
}

void Dialog::Win()//赢后处理工作
{
    for(int i=0;i<GroundSize;++i)
        for(int j=0;j<GroundSize;++j)
        {
            if(cells[i][j].Type<0&&flag[i][j]==false)
                ui->tableWidget->item(i, j)->setText("💣");
            cells[i][j].State=true;
            ui->tableWidget->item(i, j)->setFlags(Qt::NoItemFlags);
        }
    win->play();
    win->setPosition(0);
    BGM->stop();
    BGM->setPosition(0);
    QMessageBox::information(this, tr("游戏结束"),tr("雷场清除"), QMessageBox::Ok);
}

void Dialog::Lost()//输后处理工作
{
    for(int i=0;i<GroundSize;++i)
    {
        for(int j=0;j<GroundSize;++j)
        {
            if(cells[i][j].Type<0)
                ui->tableWidget->item(i, j)->setText("💣");
            else if(flag[i][j]==true)
                ui->tableWidget->item(i, j)->setText("❌");
            cells[i][j].State=true;
            ui->tableWidget->item(i, j)->setFlags(Qt::NoItemFlags);
        }
    }
    lost->play();
    lost->setPosition(0);
    BGM->stop();
    BGM->setPosition(0);
    QMessageBox::warning(this, tr("游戏结束"),tr("地雷炸了"), QMessageBox::Ok);
}

void Dialog::Color(int row, int column)//改变邻接雷格子数字的颜色
{
    if(cells[row][column].Type==1)
        ui->tableWidget->item(row, column)->setForeground(Qt::blue);
    else if(cells[row][column].Type==2)
        ui->tableWidget->item(row, column)->setForeground(Qt::green);
    else if(cells[row][column].Type==3)
        ui->tableWidget->item(row, column)->setForeground(Qt::red);
    else if(cells[row][column].Type==4)
        ui->tableWidget->item(row, column)->setForeground(Qt::magenta);
    else if(cells[row][column].Type==5)
        ui->tableWidget->item(row, column)->setForeground(Qt::darkCyan);
    else if(cells[row][column].Type==6)
        ui->tableWidget->item(row, column)->setForeground(Qt::darkYellow);
    else if(cells[row][column].Type==7)
        ui->tableWidget->item(row, column)->setForeground(Qt::darkMagenta);
    else if(cells[row][column].Type==8)
        ui->tableWidget->item(row, column)->setForeground(Qt::lightGray);
    else
        ui->tableWidget->item(row, column)->setForeground(Qt::black);
}

bool Dialog::NotNextTo(int row, int column)//判断旁边是否有雷（8邻接）
{
    if(row<0||row>=GroundSize||column<0||column>=GroundSize)
        return false;
    if(row==0&&column==0)
        return (cells[row+1][column].Type>=0)&&(cells[row][column+1].Type>=0)&&(cells[row+1][column+1].Type>=0);
    else if(row+1==GroundSize&&column+1==GroundSize)
        return (cells[row-1][column].Type>=0)&&(cells[row][column-1].Type>=0)&&(cells[row-1][column-1].Type>=0);
    else if(row==0&&column+1==GroundSize)
        return (cells[row+1][column].Type>=0)&&(cells[row][column-1].Type>=0)&&(cells[row+1][column-1].Type>=0);
    else if(row+1==GroundSize&&column==0)
        return (cells[row-1][column].Type>=0)&&(cells[row][column+1].Type>=0)&&(cells[row-1][column+1].Type>=0);
    else if(row==0)
        return (cells[row+1][column].Type>=0)&&(cells[row][column+1].Type>=0)&&(cells[row][column-1].Type>=0)&&(cells[row+1][column-1].Type>=0)&&(cells[row+1][column+1].Type>=0);
    else if(column==0)
        return (cells[row+1][column].Type>=0)&&(cells[row-1][column].Type>=0)&&(cells[row][column+1].Type>=0)&&(cells[row+1][column+1].Type>=0)&&(cells[row-1][column+1].Type>=0);
    else if(row+1==GroundSize)
        return (cells[row-1][column].Type>=0)&&(cells[row][column+1].Type>=0)&&(cells[row][column-1].Type>=0)&&(cells[row-1][column-1].Type>=0)&&(cells[row-1][column+1].Type>=0);
    else if(column+1==GroundSize)
        return (cells[row+1][column].Type>=0)&&(cells[row-1][column].Type>=0)&&(cells[row][column-1].Type>=0)&&(cells[row-1][column-1].Type>=0)&&(cells[row+1][column-1].Type>=0);
    else
        return (cells[row+1][column].Type>=0)&&(cells[row-1][column].Type>=0)&&(cells[row][column+1].Type>=0)&&(cells[row][column-1].Type>=0)&&(cells[row+1][column+1].Type>=0)&&(cells[row-1][column-1].Type>=0)&&(cells[row+1][column-1].Type>=0)&&(cells[row-1][column+1].Type>=0);
}


void Dialog::Refresh(int row,int column)//点击表格行为
{
    if(clicked==false)//第一次点击后才生成雷，保证第一次不会踩雷
    {
        //srand(time(NULL));
        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        int temp;
        set<int> s;
        for(int i=0;i<mines;)//如果随机数与已经有的随机数重复就跳过找下一个随机数，直到雷数达到要求
        {
            //temp=rand()%(GroundSize*GroundSize);
            temp=qrand()%(GroundSize*GroundSize);
            if((s.find(temp)==s.end())&&((temp-(temp%GroundSize))/GroundSize!=row||temp%GroundSize!=column))
            {
                s.insert(temp);
                cells[(temp-(temp%GroundSize))/GroundSize][temp%GroundSize].Type=-1;
                ++i;
                if(!NotNextTo(row,column))
                {
                    s.erase(temp);
                    cells[(temp-(temp%GroundSize))/GroundSize][temp%GroundSize].Type=0;
                    --i;
                }
            }
        }
        for(int i=0;i<GroundSize;++i)
            for(int j=0;j<GroundSize;++j)
                if(cells[i][j].Type>=0)
                    cells[i][j].Type=TypeRefresh(i,j);
        Two_Pass();
        clicked=true;
    }

    if(cells[row][column].Type<0&&cells[row][column].State==false)//点到雷
    {
        ui->tableWidget->item(row, column)->setBackground(Qt::red);
        Lost();
    }
    else if(cells[row][column].Type>0&&cells[row][column].State==false)//点到Type>0的格子
    {
        Color(row,column);
        ui->tableWidget->item(row, column)->setText(QString::number(cells[row][column].Type));
        cells[row][column].State=true;
        ++refreshed;
        ui->tableWidget->item(row, column)->setBackground(Qt::white);
        if(refreshed==GroundSize*GroundSize-mines)
            Win();
    }
    else if(cells[row][column].Type==0&&cells[row][column].State==false)//点到Type==0的格子
    {
        for(int i=0;i<GroundSize;++i)
            for(int j=0;j<GroundSize;++j)
                if(cells[i][j].Type==0&&cells[i][j].State==false&&cells[i][j].lable==cells[row][column].lable)
                {
                    cells[i][j].State=true;
                    ++refreshed;
                    ui->tableWidget->item(i, j)->setBackground(Qt::white);
                }
        for(int i=0;i<GroundSize;++i)
            for(int j=0;j<GroundSize;++j)
                if(cells[i][j].Type>0&&cells[i][j].State==false&&(NextTo(i,j,cells[row][column].lable)))
                {
                    Color(i,j);
                    ui->tableWidget->item(i, j)->setText(QString::number(cells[i][j].Type));
                    cells[i][j].State=true;
                    ++refreshed;
                    ui->tableWidget->item(i, j)->setBackground(Qt::white);
                }
        if(refreshed==GroundSize*GroundSize-mines)
            Win();
    }
}

void Dialog::Mark(int row,int column)//右键双击格子
{
    if(cells[row][column].State==false)//没有更新过且没有标记过
    {
        ++counter;
        ui->label_5->setText(QString::number(counter));
        ui->tableWidget->item(row, column)->setText("🚩");
        cells[row][column].State=true;
        flag[row][column]=true;
    }
    else if(cells[row][column].State==true&&flag[row][column]==true&&ui->tableWidget->item(row, column)->text()=="🚩")//被标记过，更替标记
    {
        --counter;
        ui->label_5->setText(QString::number(counter));
        ui->tableWidget->item(row, column)->setText("❓");
    }
    else if(cells[row][column].State==true&&flag[row][column]==true&&ui->tableWidget->item(row, column)->text()=="❓")//被标记过，取消标记
    {
        ui->tableWidget->item(row, column)->setText("");
        cells[row][column].State=false;
        flag[row][column]=false;
    }
    if(counter==mines)//标记数和雷数相同
    {
        int tag=0;
        for(int i=0;i<GroundSize;++i)
            for(int j=0;j<GroundSize;++j)
                if(cells[i][j].Type<0&&flag[i][j]==true)
                    ++tag;
        if(tag>=mines)//所有标记的位置都是雷的位置
            Win();
    }
}

void Dialog::on_horizontalSlider_valueChanged(int value)//调节音量
{
    volumn=value;
    start->setVolume(volumn);
    BGM->setVolume(volumn);
    win->setVolume(volumn);
    lost->setVolume(volumn);
}

void Dialog::on_pushButton_2_clicked()
{
    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    if(!items.empty())
        Refresh(ui->tableWidget->currentRow(),ui->tableWidget->currentColumn());
}


void Dialog::on_pushButton_3_clicked()
{
    QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
    if(!items.empty())
        Mark(ui->tableWidget->currentRow(),ui->tableWidget->currentColumn());
}

