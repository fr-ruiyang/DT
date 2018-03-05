#include "widget.h"
#include "ui_widget.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream>
#include <vector>
#include <QTextCodec>
#include <QTextStream>
#include <math.h>

using namespace std;

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

QList <QStringList> example;//样例集
QList <int> attributes;//属性集
QStringList attributes_string;
QStringList values[4];//属性值
struct node
{
    bool flag;//是否为叶节点
    QString grade;//如果是叶节点，类别结果（叶节点专有）
    int target_attribute;//用于判断的属性（除叶节点）
    QString value;//父树属性的属性值（除根节点）
    vector <struct node*> childs;//子树
};
struct node* DT;

void Widget::on_pushButton_clicked()//浏览
{
    QString filename;
    filename=QFileDialog::getOpenFileName(this,
                                          tr("选择文件"),
                                          "",
                                          tr("所有文件(*.*)"));

    if(filename.isEmpty())
    {
         return;
    }
    else
    {
        ui->lineEdit->setText(filename);
        QFile file(filename);
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) //加载文件
        {
            QMessageBox::warning(this,
                                     tr("警告"),
                                     tr("打开文件失败!"));
            return;
        }
        QTextStream stream(&file);
        stream.readLine();
        while(!stream.atEnd())
        {
            QString qtemp=stream.readLine();
            QStringList qtemplist=qtemp.split("\t");
            example.append(qtemplist);
        }
    }
}

double Entropy(QList <QStringList> example)//计算总体熵
{
    int n=example.size();
    int count[3]={0};
    for (int i=0;i<n;i++)
    {
        if (example.at(i).at(4)=="A")
            count[0]++;
        else if (example.at(i).at(4)=="B")
            count[1]++;
        else if (example.at(i).at(4)=="C")
            count[2]++;
        else
            continue;
    }
    double entropy = 0;
    for (int i=0;i<3;i++)
        if (count[i]!=0)
            entropy -= 1.0*count[i]/n*log(1.0*count[i]/n)/log(2.0);
//    cout<<count[0]<<"\t"<<count[1]<<"\t"<<count[2]<<"\t"<<entropy<<endl;
//    cout<<"entropy"<<entropy<<endl;
    return entropy;
}

double Entropy(QList <QStringList> example , int attribute ,QString value)//计算特定属性下的熵
{
    int n=0;
    int count[3]={0};
    for (int i=0;i<example.size();i++)
    {
        if (example.at(i).at(attribute)==value)
        {
            n++;
            if (example.at(i).at(4)=="A")
                count[0]++;
            else if (example.at(i).at(4)=="B")
                count[1]++;
            else if (example.at(i).at(4)=="C")
                count[2]++;
            else
                continue;
        }
    }
    double entropy = 0;
    for (int i=0;i<3;i++)
        if (count[i]!=0)
            entropy -= 1.0*count[i]/n*log(1.0*count[i]/n)/log(2.0);
//    cout<<count[0]<<"\t"<<count[1]<<"\t"<<count[2]<<"\t"<<entropy<<endl;
//    cout<<"nentropy"<<entropy<<endl;
//    cout<<"attribute"<<attribute<<"n"<<n<<endl;
    return entropy*n/example.size();
}

double Gain(QList <QStringList> example , int attribute)//计算信息增益
{
    switch (attribute) {
    case 1:
//        cout<<"gain1:"<<Entropy(example)<<" "<<Entropy(example,1,"男")<<" "<<Entropy(example,1,"女")<<endl;
        return Entropy(example)-Entropy(example,1,"男")-Entropy(example,1,"女");
        break;
    case 2:
//        cout<<"gain2:"<<Entropy(example)<<" "<<Entropy(example,2,"<21")<<" "<<Entropy(example,2,"≥21且≤25")<<" "<<Entropy(example,2,">25")<<endl;
        return Entropy(example)-Entropy(example,2,"<21")-Entropy(example,2,"≥21且≤25")-Entropy(example,2,">25");
        break;
    case 3:
//        cout<<"gain3:"<<Entropy(example)<<" "<<Entropy(example,3,"未")<<" "<<Entropy(example,3,"已")<<endl;
        return Entropy(example)-Entropy(example,3,"未")-Entropy(example,3,"已");
        break;
    default:
        return -1;
        break;
    }

}

bool flag(QList <QStringList> example,QString *g)//判断子集是否为同一类，若是传递类别结果
{
        QString grade=example.at(0).at(4);
        for (int i=1;i<example.size();i++)
            if (grade!=example.at(i).at(4))
                return false;
        *g=grade;
        return true;
}

int get_max_gain(QList <QStringList> example, QList <int> attributes)//获取最大信息增益的属性
{
    int id=0;
    double max=0;
    for (int i=0;i<attributes.size();i++)
    {
        double temp=Gain(example,attributes.at(i));
//        cout<<temp<<endl;
        if (temp>max)
        {
            max=temp;
            id=i;
        }
    }
    return id;
}

QString most(QList <QStringList> example)
{
    int count[3]={0};
    for (int i=0;i<example.size();i++)
    {
        if (example.at(i).at(4)=="A")
            count[0]++;
        else if (example.at(i).at(4)=="B")
            count[1]++;
        else if (example.at(i).at(4)=="C")
            count[2]++;
        else
            continue;
    }
    if (count[0]>=count[1]&&count[0]>=count[2])
        return "A";
    else if (count[1]>=count[0]&&count[1]>=count[2])
        return "B";
    else //if (count[2]>=count[1]&&count[2]>=count[0])
        return "C";
}

struct node * buildDT(QList <QStringList> example, QList <int> attributes)//构建决策树
{
    struct node * root=new struct node;
    QString grade;
//    root->value="";
    //如果分类结果为同一类，取该类别结果（叶节点）
    if (flag(example,&grade))
    {
        root->flag=true;
        root->grade=grade;
        return root;
    }
    //如果attributes为空,取example中最多出现的类别结果（叶节点）
    else if (attributes.isEmpty())
    {
        root->flag=true;
        root->grade=most(example);
        return root;
    }
    else
    {
        root->flag=false;
        root->target_attribute=attributes.at(get_max_gain(example,attributes));
        //建立新的剩余属性集
        QList <int> new_attribute;
        for (int i=0;i<attributes.size();i++)
            if (attributes.at(i)!=root->target_attribute)
                new_attribute.append(attributes.at(i));
        //根据target_attribute分割example
        for (int i=0;i<values[root->target_attribute].size();i++)
        {
            QList <QStringList> new_example;
            QString value=values[root->target_attribute].at(i);
            for (int j=0;j<example.size();j++)
                if (example.at(j).at(root->target_attribute)==value)
                    new_example.append(example.at(j));
            if (new_example.isEmpty())
            {
                struct node child_node;
                child_node.flag=true;
                child_node.grade=most(example);
                child_node.value=value;
                root->childs.push_back(&child_node);
            }
            else
            {
                root->childs.push_back(buildDT(new_example,new_attribute));
                root->childs.back()->value=value;
            }
        }
        return root;
    }
}

void displaytree(struct node* DT,Ui::Widget* ui,int depth)
{
    if (!DT->value.isNull())
        ui->textBrowser->insertPlainText(DT->value+"\t");
    if (DT->flag)
        ui->textBrowser->insertPlainText(DT->grade+"\t");
    else
        ui->textBrowser->insertPlainText(attributes_string.at(DT->target_attribute-1)+"\t");
    for (int i=0;i<DT->childs.size();i++)
    {
        displaytree(DT->childs.at(i),ui,depth+2);
        ui->textBrowser->insertPlainText("\n");
        for (int j=0;j<depth;j++)
            ui->textBrowser->insertPlainText("\t");
    }
}

void Widget::on_pushButton_3_clicked()//训练
{
    attributes.clear();
    attributes_string.clear();
    for (int i=0;i<4;i++)
        values[i].clear();
    attributes << 1 << 2 << 3;
    attributes_string<<"性别"<<"年龄段"<<"婚状";
    values[1]<<"男"<<"女";
    values[2]<<"<21"<<"≥21且≤25"<<">25";
    values[3]<<"未"<<"已";
    values[0]<<"A"<<"B"<<"C";
//    for (int i=0;i<attributes_string.size();i++)
//        ui->textBrowser->insertPlainText(attributes_string.at(i)+"\t");
//    for (int i=0;i<4;i++)
//        for (int j=0;j<values[i].size();j++)
//            ui->textBrowser->append(values[i].at(j));
    if (!example.isEmpty())
    {
        DT=buildDT(example,attributes);
        ui->textBrowser->append("决策树如下（从左到右）：\n");
        displaytree(DT,ui,1);
    }
}

void Widget::on_pushButton_2_clicked()//预测，识别
{
    QStringList test;
    test<<ui->comboBox->currentText()<<ui->comboBox_2->currentText()<<ui->comboBox_3->currentText();
    if (DT!=NULL)
    {
        struct node * p=DT;
        while (!p->flag)
        {
            for (int i=0;i<p->childs.size();i++)
                if (p->childs.at(i)->value==test.at(p->target_attribute-1))
                {
                    p=p->childs.at(i);
                    break;
                }
        }
        ui->lineEdit_2->setText(p->grade);
    }
}
