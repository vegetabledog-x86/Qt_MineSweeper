#ifndef DIALOG_H
#define DIALOG_H

#include<QDialog>
#include<ctime>
#include<string>
#include<vector>
#include<set>

#include<QTableWidgetItem>
#include<QTableWidget>
#include<QMouseEvent>
#include<QMessageBox>
#include<QtMultimedia/QMediaPlayer>
#include<QSoundEffect>
#include<QMediaPlayer>
#include<QMouseEvent>

#define MAX(A,B) A>B?A:B
#define MIN(A,B) A<B?A:B

using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

    int GroundSize=10;
    int mines=0;
    int counter=0;
    int refreshed=0;
    int font=15;
    int volumn=50;
    bool clicked=false;
    struct Cell//Type:0表示没有雷也不与雷相邻，-1表示有雷，n表示周围雷的个数
    {
        short int Type=0;
        int lable=-1;
        bool State=false;
    };
    Cell cells[20][20];
    bool flag[20][20]={{false}};
    QMediaPlayer *start,*BGM,*win,*lost;

private slots:
    void on_pushButton_clicked();
    void on_horizontalSlider_valueChanged(int value);

    int TypeRefresh(int row, int column);
    bool NextTo(int row, int column,int lable);
    void Two_Pass();
    void Win();
    void Lost();
    void Color(int row, int column);
    bool NotNextTo(int row, int column);
    void Paly_InitStatus(QMediaPlayer::MediaStatus status);

    void on_tableWidget_cellClicked(int row,int column);
    void on_tableWidget_cellDoubleClicked(int row,int column);

private:
    Ui::Dialog *ui;

signals:
    void left();
    void right();
    void left_and_right();
};
#endif // DIALOG_H
