#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextStream>
#include <QFile>
#include <QString>
#include <QTextCharFormat>
#include <QTextCursor>
#include <Qcolor>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QXmlStreamWriter>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QVector>
#include <QImage>
#include <QPainter>

#include <queue>
#include <string>
#include <sstream>
#include <cstring>
#include "scanner.h"
#include "parser.h"


#define NODE_WIDTH 80
#define NODE_HEIGHT 40
#define NODE_FONT_SIZE 12
#define NODE_MARGIN 30

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct coordinate
{
  int x;
  int y;
};

struct diagram_node_type
{
  public:
  float xValue;
  float modifyingValue;
  coordinate parentCoord;
  QGraphicsTextItem *title;
  QGraphicsTextItem *token;
  QVector<QGraphicsLineItem*> connectionLines;
  bool shapeIsRect;
  union
  {
    QGraphicsRectItem *nonTerminalNode;
    QGraphicsEllipseItem *terminalNode;
  };
  ~diagram_node_type();
};

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_startParsingButton_clicked();

  void on_browsePushButton_clicked();

  void on_filePathLineEdit_editingFinished();

  void on_actionOpen_triggered();

  void on_actionExport_image_as_png_triggered();

  void on_actionSave_syntax_tree_as_XML_triggered();

  void on_actionExit_triggered();

  void on_actionAbout_QT_triggered();

  void on_actionAbout_triggered();

  void on_actionHow_to_use_triggered();

private:
  std::queue<token_type>* extract_list_of_tokens(std::string fileText);
  void save_file();
  void viewOpenFile(QString filename);
  void generate_XML_file(syntax_tree_node *root);
  void navigate_syntax_tree(syntax_tree_node *node ,QXmlStreamWriter *writer);
  void generate_tree_diagram(syntax_tree_node *root);
  void navigate_syntax_tree(syntax_tree_node *node ,coordinate parentCoord={0,0}, int recursionLevel=0, float xVal=0);
  void closeEvent(QCloseEvent *event);

  Ui::MainWindow *ui;
  parser mainParser;
  QVector<QVector<diagram_node_type*>> treeNodes;
  QGraphicsScene treeScene;
  std::queue<token_type> *listOfTokens;
  QFile openFile;
  bool fileSaved;
};
#endif // MAINWINDOW_H
