#include "dialog.h"
#include "ui_dialog.h"

//构造函数
Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("扫雷"));//窗口标题

    QScreen *screen=QGuiApplication::primaryScreen ();
    QRect sc=screen->availableGeometry() ;
    setFixedSize(sc.width(),sc.height());//窗口分辨率

    Qt::WindowFlags flags=Qt::Dialog;
    flags |=Qt::WindowMinMaxButtonsHint;//添加最小化最大化按钮
    flags |=Qt::WindowCloseButtonHint;//启用关闭按钮
    setWindowFlags(flags);

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//表格只读
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);//只选一个
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectItems);//只能选格

    ui->tableWidget->verticalHeader()->setVisible(false); //设置垂直头不可见
    ui->tableWidget->horizontalHeader()->setVisible(false); //设置水平头不可见

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);// 表格列的大小随表格大小的变化而变化
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);// 表格行的大小随表格大小的变化而变化

    qtime.setHMS(0,0,0,0);//设置初始值
    qtimer = new QTimer(this);//创建一个定时器

    connect(qtimer,SIGNAL(timeout()),this,SLOT(StartTiming()));

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
    Length=10;
    ui->spinBox_2->setValue(10);
    ui->spinBox_2->setMaximum(40);//设置最大值

    ui->spinBox_3->setMinimum(4);//设置最小值
    Height=10;
    ui->spinBox_3->setValue(10);
    ui->spinBox_3->setMaximum(40);//设置最大值

    font=MIN((ui->tableWidget->horizontalHeader()->width()/Length)*(0.15), ui->tableWidget->verticalHeader()->height()/Height*(0.15));
    ui->tableWidget->setFont(QFont("song", font));

    ui->spinBox->setMinimum(1);//设置最小值
    ui->spinBox->setValue(10);
    ui->spinBox->setMaximum(91);//设置最大值
}

Dialog::~Dialog()//析构函数
{
    delete ui;
}

void Dialog::Paly_InitStatus(QMediaPlayer::MediaStatus status)//从头播放音乐
{
    if(status == QMediaPlayer::EndOfMedia)
    {
        BGM->setPosition(0);
        BGM->play();
    }
}

void Dialog::resizeEvent(QResizeEvent *event)//自动调节字体大小
{
    Q_UNUSED(event);
    font=MIN((ui->tableWidget->horizontalHeader()->width()/Length)*(0.15), ui->tableWidget->verticalHeader()->height()/Height*(0.15));
    ui->tableWidget->setFont(QFont("song", font));
}

void Dialog::StartTiming()
{
    static quint32 time_out=0;
    ++time_out;
    qtime=qtime.addSecs(1);
    ui->label_13->setText(qtime.toString("mm : ss"));
}

void Dialog::EndTiming()
{
    qtimer->stop();
    qtime.setHMS(0,0,0,0);
}

int Dialog::TypeRefresh(int row, int column)//计算cells里不是雷的格子的Type应该更新为多少
{
    if(row<0||row>=Height||column<0||column>=Length)
        return -1;
    int total=0;
    for(int i=(MAX(row-1,0));i<=(MIN(row+1,Height-1));++i)
    {
        for(int j=(MAX(column-1,0));j<=(MIN(column+1,Length-1));++j)
        {
            if(i==row&&j==column)
                continue;
            if(cells[i][j].Type<0)
                ++total;
        }
    }
    return total;
}

void Dialog::Two_Pass()//两边扫描算法找到所有Type==0格子的连通域
{
    int Lable=0;
    vector<int> Parent;
    //First Pass
    for(int i=0;i<Height;++i)
    {
        for(int j=0;j<Length;++j)
        {
            if(cells[i][j].Type==0)
            {
                if(i==0&&j==0)
                {
                    cells[i][j].lable=Lable;
                    Parent.push_back(-2);
                    ++Lable;
                }
                else if(i==0)
                {
                    if(cells[i][j-1].Type==0)
                    {
                        cells[i][j].lable=cells[i][j-1].lable;
                    }
                    else
                    {
                        cells[i][j].lable=Lable;
                        Parent.push_back(-2);
                        ++Lable;
                    }
                }
                else if(j==0)
                {
                    if(cells[i-1][j].Type==0)
                    {
                        cells[i][j].lable=cells[i-1][j].lable;
                    }
                    else
                    {
                        cells[i][j].lable=Lable;
                        Parent.push_back(-2);
                        ++Lable;
                    }
                }
                else if(cells[i-1][j].Type==0&&cells[i][j-1].Type==0)//要避免循环爹出现，必须是严格的树状结构，小数为大数树根(并查集)
                {
                    cells[i][j].lable=(MIN(cells[i-1][j].lable,cells[i][j-1].lable));
                    if(cells[i-1][j].lable!=cells[i][j-1].lable)
                    {
                        int childmax=(MAX(cells[i-1][j].lable,cells[i][j-1].lable));
                        while(Parent[childmax]!=-2)
                            childmax=Parent[childmax];
                        int childmin=(MIN(cells[i-1][j].lable,cells[i][j-1].lable));
                        while(Parent[childmin]!=-2)
                            childmin=Parent[childmin];
                        if(childmin!=childmax)
                            Parent[(MAX(childmax,childmin))]=(MIN(childmax,childmin));
                    }
                }
                else if(cells[i-1][j].Type==0)
                {
                    cells[i][j].lable=cells[i-1][j].lable;
                }
                else if(cells[i][j-1].Type==0)
                {
                    cells[i][j].lable=cells[i][j-1].lable;
                }
                else
                {
                    cells[i][j].lable=Lable;
                    Parent.push_back(-2);
                    ++Lable;
                }
            }
        }
    }
    //Second Pass
    int current;
    for(int i=0;i<Height;++i)
    {
        for(int j=0;j<Length;++j)
        {
            if(cells[i][j].Type==0)
            {
                current=cells[i][j].lable;
                while(Parent[current]!=-2)
                    current=Parent[current];
                cells[i][j].lable=current;
            }
        }
    }
}

void Dialog::on_pushButton_clicked()//开始游戏，初始化所有参数和图形界面
{
    EndTiming();
    start->stop();
    win->stop();
    win->setPosition(0);
    lost->stop();
    lost->setPosition(0);
    Length=ui->spinBox_2->value();
    Height=ui->spinBox_3->value();
    font=MIN((ui->tableWidget->horizontalHeader()->width()/Length)*(0.15), ui->tableWidget->verticalHeader()->height()/Height*(0.15));
    mines=ui->spinBox->value();
    for(int i=0;i<Height;++i)
    {
        for(int j=0;j<Length;++j)
        {
            cells[i][j].Type=0;
            cells[i][j].lable=-1;
            cells[i][j].State=false;
            flag[i][j]=0;
        }
    }
    refreshed=0;
    counter=0;
    clicked=false;
    finished=false;
    ui->label_13->setText(qtime.toString("mm  :  ss"));
    ui->label_4->setText(QString::number(mines));
    ui->label_5->setText(QString::number(counter));
    ui->label_6->setText(QString::number(Length)+"  X  "+QString::number(Height));
    ui->label_13->setText(qtime.toString("mm  :  ss"));
    ui->tableWidget->setFont(QFont("song", font));//必须在设置场地大小前设置字体，否则排版混乱
    ui->tableWidget->setColumnCount(Length); //设置列数(长)
    ui->tableWidget->setRowCount(Height); //设置行数(高)

    QFont qfont;
    qfont.setBold(true);
    for(int i=0;i<Height;++i)
    {
        for(int j=0;j<Length;++j)
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
    if(row<0||row>=Height||column<0||column>=Length||lable<0)
        return false;
    for(int i=(MAX(row-1,0));i<=(MIN(row+1,Height-1));++i)
    {
        for(int j=(MAX(column-1,0));j<=(MIN(column+1,Length-1));++j)
        {
            if(i==row&&j==column)
                continue;
            if(cells[i][j].lable==lable)
                return true;
        }
    }
    return false;
}

void Dialog::Win()//赢后处理工作
{
    EndTiming();
    for(int i=0;i<Height;++i)
    {
        for(int j=0;j<Length;++j)
        {
            if(cells[i][j].Type<0&&flag[i][j]==0)
                ui->tableWidget->item(i, j)->setText("💣");
            cells[i][j].State=true;
            ui->tableWidget->item(i, j)->setFlags(Qt::NoItemFlags);
        }
    }
    finished=true;
    win->play();
    win->setPosition(0);
    BGM->stop();
    BGM->setPosition(0);
    QMessageBox::information(this, tr("游戏结束"),tr("雷场清除"), QMessageBox::Ok);
}

void Dialog::Lost()//输后处理工作
{
    EndTiming();
    for(int i=0;i<Height;++i)
    {
        for(int j=0;j<Length;++j)
        {
            if(cells[i][j].Type<0)
            {
                ui->tableWidget->item(i, j)->setText("💣");
                if(flag[i][j]==1)
                    ui->tableWidget->item(i, j)->setBackground(Qt::green);
                else if(flag[i][j]==2)
                    ui->tableWidget->item(i, j)->setBackground(Qt::darkYellow);
            }
            else if(flag[i][j]>0)
                ui->tableWidget->item(i, j)->setText("❌");
            cells[i][j].State=true;
            ui->tableWidget->item(i, j)->setFlags(Qt::NoItemFlags);
        }
    }
    finished=true;
    lost->play();
    lost->setPosition(0);
    BGM->stop();
    BGM->setPosition(0);
    QMessageBox::warning(this, tr("游戏结束"),tr("地雷炸了"), QMessageBox::Ok);
}

void Dialog::Color(int row, int column)//改变邻接雷格子数字的颜色，还原原版扫雷颜色
{
    if(cells[row][column].Type==1)
        ui->tableWidget->item(row, column)->setForeground(Qt::blue);
    else if(cells[row][column].Type==2)
        ui->tableWidget->item(row, column)->setForeground(Qt::green);
    else if(cells[row][column].Type==3)
        ui->tableWidget->item(row, column)->setForeground(Qt::red);
    else if(cells[row][column].Type==4)
        ui->tableWidget->item(row, column)->setForeground(Qt::darkBlue);
    else if(cells[row][column].Type==5)
        ui->tableWidget->item(row, column)->setForeground(Qt::darkRed);
    else if(cells[row][column].Type==6)
        ui->tableWidget->item(row, column)->setForeground(Qt::cyan);
    else if(cells[row][column].Type==7)
        ui->tableWidget->item(row, column)->setForeground(Qt::black);
    else if(cells[row][column].Type==8)
        ui->tableWidget->item(row, column)->setForeground(Qt::lightGray);
    else
        ui->tableWidget->item(row, column)->setForeground(Qt::magenta);
}

bool Dialog::NotNextTo(int row, int column)//判断旁边是否有雷（8邻接）
{
    if(row<0||row>=Height||column<0||column>=Length)
        return false;
    for(int i=(MAX(row-1,0));i<=(MIN(row+1,Height-1));++i)
    {
        for(int j=(MAX(column-1,0));j<=(MIN(column+1,Length-1));++j)
        {
            if(i==row&&j==column)
                continue;
            if(cells[i][j].Type<0)
                return false;
        }
    }
    return true;
}

void Dialog::on_tableWidget_cellClicked(int row,int column)//点击表格行为
{
    if(finished==true)
        return ;
    else if(mode==false)
        Left(row,column);
    else if(mode==true)
        Right(row,column);
}

void Dialog::Left(int row,int column)//点击模式
{
    if(clicked==false)//第一次点击后才生成雷，保证第一次不会踩雷
    {
        int temp;
        set<int> s;
        for(int i=0;i<mines;)//如果随机数与已经有的随机数重复就跳过找下一个随机数，直到雷数达到要求
        {
            temp=QRandomGenerator::securelySeeded().bounded(Length*Height);
            if((s.find(temp)==s.end())&&((temp-(temp%Length))/Length!=row||temp%Length!=column))
            {
                s.insert(temp);
                cells[(temp-(temp%Length))/Length][temp%Length].Type=-1;
                ++i;
                if(!NotNextTo(row,column))
                {
                    s.erase(temp);
                    cells[(temp-(temp%Length))/Length][temp%Length].Type=0;
                    --i;
                }
            }
        }
        for(int i=0;i<Height;++i)
            for(int j=0;j<Length;++j)
                if(cells[i][j].Type>=0)
                    cells[i][j].Type=TypeRefresh(i,j);
        Two_Pass();
        clicked=true;

        qtimer->start(1000);
    }

    Refresh(row,column);
}

void Dialog::Refresh(int row, int column)//更新格子
{
    if(cells[row][column].Type<0&&cells[row][column].State==false)//点到雷
    {
        ui->tableWidget->item(row, column)->setBackground(Qt::red);
        Lost();
    }
    else if(cells[row][column].Type>0&&cells[row][column].State==false)//点到Type>0的格子(邻接雷)
    {
        Color(row,column);
        ui->tableWidget->item(row, column)->setText(QString::number(cells[row][column].Type));
        cells[row][column].State=true;
        ++refreshed;
        ui->tableWidget->item(row, column)->setBackground(Qt::white);
        if(refreshed==Length*Height-mines)
            Win();
    }
    else if(cells[row][column].Type==0&&cells[row][column].State==false)//点到Type==0的格子(不邻接雷)
    {
        for(int i=0;i<Height;++i)
        {
            for(int j=0;j<Length;++j)
            {
                if(cells[i][j].Type==0&&cells[i][j].State==false&&cells[i][j].lable==cells[row][column].lable)
                {
                    cells[i][j].State=true;
                    ++refreshed;
                    ui->tableWidget->item(i, j)->setBackground(Qt::white);
                }
            }
        }
        for(int i=0;i<Height;++i)
        {
            for(int j=0;j<Length;++j)
            {
                if(cells[i][j].Type>0&&cells[i][j].State==false&&(NextTo(i,j,cells[row][column].lable)))
                {
                    Color(i,j);
                    ui->tableWidget->item(i, j)->setText(QString::number(cells[i][j].Type));
                    cells[i][j].State=true;
                    ++refreshed;
                    ui->tableWidget->item(i, j)->setBackground(Qt::white);
                }
            }
        }
        if(refreshed==Length*Height-mines)
            Win();
    }
}

void Dialog::Right(int row,int column)//右键格子，标记模式
{
    if(cells[row][column].State==false)//没有更新过且没有标记过
    {
        ++counter;
        ui->label_5->setText(QString::number(counter));
        ui->tableWidget->item(row, column)->setText("🚩");
        cells[row][column].State=true;
        flag[row][column]=1;
    }
    else if(cells[row][column].State==true&&flag[row][column]==1)//被标记过，更替标记
    {
        --counter;
        ui->label_5->setText(QString::number(counter));
        ui->tableWidget->item(row, column)->setText("❓");
        flag[row][column]=2;
    }
    else if(cells[row][column].State==true&&flag[row][column]==2)//被标记过，取消标记
    {
        ui->tableWidget->item(row, column)->setText("");
        cells[row][column].State=false;
        flag[row][column]=0;
    }
    else if(cells[row][column].State==true&&flag[row][column]==0&&cells[row][column].Type>0)//原先更新过且不是标记，并且是数字
    {
        int nums=0;
        for(int i=(MAX(row-1,0));i<=(MIN(row+1,Height-1));++i)
            for(int j=(MAX(column-1,0));j<=(MIN(column+1,Length-1));++j)
                if(cells[i][j].State==true&&flag[i][j]==1)//8邻接格内标记的数目
                    ++nums;
        if(cells[row][column].Type==nums)
        {
            for(int i=(MAX(row-1,0));i<=(MIN(row+1,Height-1));++i)
            {
                for(int j=(MAX(column-1,0));j<=(MIN(column+1,Length-1));++j)
                {
                    if(cells[i][j].State==true)
                        continue;
                    Refresh(i,j);
                }
            }
        }
        else
        {
            for(int i=(MAX(row-1,0));i<=(MIN(row+1,Height-1));++i)
                for(int j=(MAX(column-1,0));j<=(MIN(column+1,Length-1));++j)
                    if(cells[i][j].State==false)
                        ui->tableWidget->item(i, j)->setBackground(Qt::yellow);

            QElapsedTimer timer;
            timer.start();
            while(timer.elapsed() < 200)             //等待时间流逝 200 ms
                QCoreApplication::processEvents();   //处理事件

            for(int i=(MAX(row-1,0));i<=(MIN(row+1,Height-1));++i)
                for(int j=(MAX(column-1,0));j<=(MIN(column+1,Length-1));++j)
                    if(cells[i][j].State==false)
                        ui->tableWidget->item(i, j)->setBackground(Qt::gray);
        }
    }
    if(counter==mines)//标记数和雷数相同
    {
        int tag=0;
        bool notsure=false;
        for(int i=0;i<Height;++i)
        {
            for(int j=0;j<Length;++j)
            {
                if(cells[i][j].Type<0&&flag[i][j]==1)
                    ++tag;
                if(flag[i][j]==2)
                    notsure=true;
            }
        }
        if(tag==mines&&notsure==false)//所有有效标记的位置都是雷的位置且没有不确定标记
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

void Dialog::on_spinBox_2_valueChanged(int arg1)//调节长度
{
    Q_UNUSED(arg1);
    ui->spinBox->setMaximum((ui->spinBox_2->value())*(ui->spinBox_3->value())-9);//设置最大值
}

void Dialog::on_spinBox_3_valueChanged(int arg1)//调节高度
{
    Q_UNUSED(arg1);
    ui->spinBox->setMaximum((ui->spinBox_2->value())*(ui->spinBox_3->value())-9);//设置最大值
}

void Dialog::on_checkBox_stateChanged(int arg1)//改变模式
{
    if(arg1 == Qt::Unchecked)
        mode=false;
    else if(arg1 == Qt::Checked)
        mode=true;
}

