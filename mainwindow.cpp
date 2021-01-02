#include "mainwindow.h"
#include "ui_mainwindow.h"



diagram_node_type::~diagram_node_type()
{
  delete title;
  delete token;
  for(int i=0; i<connectionLines.size();i++)
    {
      delete connectionLines[i];
    }
  if(shapeIsRect)delete nonTerminalNode;
  else delete terminalNode;
}

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  listOfTokens = new std::queue<token_type>;
  ui->listOfTokensTable->setColumnCount(2);
  ui->listOfTokensTable->setHorizontalHeaderLabels(QStringList()<<"Token Value"<<"Token Type");
  ui->XMLTextEdit->setReadOnly(true);
  ui->XMLTextEdit->setFontFamily("cascadia code");
  fileSaved = true;
  this->setWindowTitle("Tiny Parser");
  ui->treeDiagramGraphicsView->setScene(&treeScene);
  ui->treeDiagramGraphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

}

MainWindow::~MainWindow()
{
  delete listOfTokens;
  delete ui;
}

std::queue<token_type>* MainWindow::extract_list_of_tokens(std::string fileText)
{
      std::stringstream csvFile;
      csvFile.str(fileText);
      std::queue<token_type> *result=new std::queue<token_type>();
      std::string tempLine;
      token_type tempToken;
      auto commaLocation=std::string::npos;
      while(!csvFile.eof())
      {
          std::getline(csvFile,tempLine);
          commaLocation = tempLine.find(',');
          if(commaLocation != std::string::npos)
            {
              tempToken.tokenValue = new char[commaLocation];
              strcpy(tempToken.tokenValue ,tempLine.substr(0 , commaLocation).c_str());
              tempToken.tokenType = (tokens)(search_token_by_name(tempLine.substr(commaLocation +1)));
              result->push(tempToken);
            }
      }
      tempToken.tokenValue = nullptr;
      return result;
}

void MainWindow::generate_XML_file(syntax_tree_node *root)
{
  QString *XMLString = new QString();
  QXmlStreamWriter *XMLWriter = new QXmlStreamWriter(XMLString);
  XMLWriter->setAutoFormatting(true);
  XMLWriter->writeStartDocument();
  XMLWriter->writeStartElement("program");
  navigate_syntax_tree(root ,XMLWriter);
  XMLWriter->writeEndElement();
  XMLWriter->writeEndDocument();
  ui->XMLTextEdit->setText(*XMLString);
  delete XMLString;
  delete XMLWriter;
}

void MainWindow::navigate_syntax_tree(syntax_tree_node *node ,QXmlStreamWriter *writer)
{
  writer->writeStartElement("node");
  writer->writeAttribute("title",QString(node->get_title().c_str()));
  writer->writeAttribute("tokenType",QString(tokenName[node->get_token().tokenType]));
  writer->writeAttribute("tokenValue",QString((char*)(node->get_token().tokenValue)));
  auto nodeChildren= node->get_children();
  for(unsigned int i=0; i<nodeChildren.size();i++)
    {
      writer->writeStartElement("child");
      navigate_syntax_tree(nodeChildren[i],writer);
      writer->writeEndElement();
    }
  writer->writeEndElement();
  if(node->get_adjacentNode() != NULL)
    {
      navigate_syntax_tree(node->get_adjacentNode(),writer);
    }
  return;
}

void MainWindow::generate_tree_diagram(syntax_tree_node *root)
{
  ui->treeDiagramGraphicsView->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
  treeNodes.resize(root->height_of_children()+1);
  navigate_syntax_tree(root);
  int maxWidthIndex=0;
  for(int i=1;i<treeNodes.size();i++)
    {
      if(treeNodes[i].size()>=treeNodes[maxWidthIndex].size())maxWidthIndex=i;
    }
//  correct the coordinates of the longest row
//  for(int j=0;j<treeNodes[maxWidthIndex].size();j++)
//    {
//      if(treeNodes[maxWidthIndex][j]->shapeIsRect)
//        {
//          treeNodes[maxWidthIndex][j]->nonTerminalNode->setPos(j*(NODE_WIDTH+NODE_MARGIN), maxWidthIndex*(NODE_HEIGHT+NODE_MARGIN));
//        }
//      else
//        {
//          treeNodes[maxWidthIndex][j]->terminalNode->setPos(j*(NODE_WIDTH+NODE_MARGIN), maxWidthIndex*(NODE_HEIGHT+NODE_MARGIN));
//        }
//      treeNodes[maxWidthIndex][j]->title->setPos(j*(NODE_WIDTH+NODE_MARGIN)+NODE_WIDTH/2-treeNodes[maxWidthIndex][j]->title->boundingRect().width()/2/*textWidth()/2*/, maxWidthIndex*(NODE_HEIGHT+NODE_MARGIN)-5);
//      treeNodes[maxWidthIndex][j]->token->setPos(j*(NODE_WIDTH+NODE_MARGIN)+NODE_WIDTH/2-treeNodes[maxWidthIndex][j]->token->boundingRect().width()/2/*textWidth()/2*/, maxWidthIndex*(NODE_HEIGHT+NODE_MARGIN)+NODE_FONT_SIZE+2);
//    }
  //  Put all nodes in a grid
  for(int i=0;i<treeNodes.size();i++)
    {
      for(int j=0;j<treeNodes[i].size();j++)
        {
          if(treeNodes[i][j]->shapeIsRect)
            {
              treeNodes[i][j]->nonTerminalNode->setPos(j*(NODE_WIDTH+NODE_MARGIN), i*(NODE_HEIGHT+NODE_MARGIN));
            }
          else
            {
              treeNodes[i][j]->terminalNode->setPos(j*(NODE_WIDTH+NODE_MARGIN), i*(NODE_HEIGHT+NODE_MARGIN));
            }
          treeNodes[i][j]->title->setPos(j*(NODE_WIDTH+NODE_MARGIN)+NODE_WIDTH/2-treeNodes[i][j]->title->boundingRect().width()/2/*textWidth()/2*/, i*(NODE_HEIGHT+NODE_MARGIN)-5);
          treeNodes[i][j]->token->setPos(j*(NODE_WIDTH+NODE_MARGIN)+NODE_WIDTH/2-treeNodes[i][j]->token->boundingRect().width()/2/*textWidth()/2*/, i*(NODE_HEIGHT+NODE_MARGIN)+NODE_FONT_SIZE+2);
        }
    }
  //correct the coordinates of the rows above it
//  int childrenCount=0;
//  int lastParent=0;
//  for(int i=maxWidthIndex;i>=0;i--)
//    {
//      lastParent=0;
//      for(int j=0;j<treeNodes[i].size();j++)
//        {
//          if()
//        }
//    }
  //correct the coordinates of the rows below it
//  for(int i=maxWidthIndex+1;i<treeNodes.size();i++)
//    {
//      for(int j=0;j<treeNodes[i].size();j++)
//        {

//        }
//    }
  //generate lines connecting nodes
  coordinate tempCoord;
  for(int i=0;i<treeNodes.size();i++)
    {
      for(int j=0;j<treeNodes[i].size();j++)
        {
          tempCoord = treeNodes[i][j]->parentCoord;
          treeNodes[tempCoord.y][tempCoord.x]->connectionLines.push_back(new QGraphicsLineItem(treeNodes[tempCoord.y][tempCoord.x]->token->x(),
                                                                                               treeNodes[tempCoord.y][tempCoord.x]->token->y(),
                                                                                               treeNodes[i][j]->token->x(),
                                                                                               treeNodes[i][j]->token->y()));
          treeNodes[tempCoord.y][tempCoord.x]->connectionLines.last()->setZValue(0);
          treeScene.addItem(treeNodes[tempCoord.y][tempCoord.x]->connectionLines.last());
        }
    }
  ui->treeDiagramGraphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  return;
}

void MainWindow::navigate_syntax_tree(syntax_tree_node *node, coordinate parentCoord, int recursionLevel, float xVal)
{
  if(node == NULL)return;
  diagram_node_type *tempNode = new diagram_node_type;
  tempNode->parentCoord=parentCoord;
  tempNode->xValue = xVal;
  tempNode->title = new QGraphicsTextItem(QString(node->get_title().c_str()));
  tempNode->title->setFont(QFont("cascadia code",NODE_FONT_SIZE));
  tempNode->title->setZValue(0.2);
  tempNode->token= new QGraphicsTextItem();
  tempNode->token->setZValue(0.2);
  if(strcmp(node->get_title().c_str(),node->get_token().tokenValue)!=0)
    {
      tempNode->token->setPlainText(QString(node->get_token().tokenValue));
      tempNode->token->setFont(QFont("cascadia code",NODE_FONT_SIZE));
    }
  tempNode->shapeIsRect = !(node->get_title()=="op"||
                            node->get_title()=="const"||
                            node->get_title()=="id");
  if(tempNode->shapeIsRect)
    {
      tempNode->nonTerminalNode= new QGraphicsRectItem(0,0,NODE_WIDTH,NODE_HEIGHT);
      tempNode->nonTerminalNode->setBrush(QBrush(Qt::white));
      tempNode->nonTerminalNode->setZValue(0.1);
    }
  else
    {
      tempNode->terminalNode = new QGraphicsEllipseItem(0,0,NODE_WIDTH,NODE_HEIGHT);
      tempNode->terminalNode->setBrush(QBrush(Qt::white));
      tempNode->terminalNode->setZValue(0.1);
    }
  treeNodes[recursionLevel].push_back(tempNode);
  treeScene.addItem(treeNodes[recursionLevel].last()->title);
  treeScene.addItem(treeNodes[recursionLevel].last()->token);
  if(treeNodes[recursionLevel].last()->shapeIsRect)
    treeScene.addItem(treeNodes[recursionLevel].last()->nonTerminalNode);
  else
    treeScene.addItem(treeNodes[recursionLevel].last()->terminalNode);
  auto nodeChildren= node->get_children();
  for(unsigned int i=0; i<nodeChildren.size();i++)
    {
      navigate_syntax_tree(nodeChildren[i],{treeNodes[recursionLevel].size()-1,recursionLevel} , recursionLevel+1 , i);
    }
  if(node->get_adjacentNode() != NULL)
    {
      navigate_syntax_tree(node->get_adjacentNode(),{treeNodes[recursionLevel].size()-1,recursionLevel} , recursionLevel, xVal+1);
    }
  return;
}

void MainWindow::on_startParsingButton_clicked()
{
  fileSaved = false;
  parser_return_type parsingResult = mainParser.parse_tokens(*listOfTokens);
  if(parsingResult.evaluated)
    {
        ui->statusbar->setStyleSheet("QStatusBar"
                                     "{padding-left:8px;"
                                     "background:green;"
                                     "color:white;"
                                     "font-weight:bold;}");
        ui->statusbar->showMessage((QString(parsingResult.error_message.c_str())));
    }
  else
    {
      ui->statusbar->setStyleSheet("QStatusBar"
                                   "{padding-left:8px;"
                                   "background:red;"
                                   "color:white;"
                                   "font-weight:bold;}");
      ui->statusbar->showMessage((QString(parsingResult.error_message.c_str()))
                                 +" at token "
                                 +QString::number(ui->listOfTokensTable->rowCount() - mainParser.get_number_of_tokens()+1));
    }
  generate_XML_file(mainParser.get_parse_tree_root());
  generate_tree_diagram(mainParser.get_parse_tree_root());
}

void MainWindow::on_browsePushButton_clicked()
{
  ui->filePathLineEdit->setText(
                     QFileDialog::getOpenFileName(
                     this,
                     tr("Open File"),
                     ui->filePathLineEdit->text(),
                     tr("CSV files (*.csv);;Text files (*.txt)")));
  if(QFile(ui->filePathLineEdit->text()).exists())viewOpenFile(ui->filePathLineEdit->text());
}

void MainWindow::on_filePathLineEdit_editingFinished()
{
  if(!ui->filePathLineEdit->text().isEmpty())
  {
    if(QFile(ui->filePathLineEdit->text()).exists())
      {
        viewOpenFile(ui->filePathLineEdit->text());
      }
    else
      {
        QMessageBox::warning(this,"Not Found","This File doesn't Exist");
      }
  }
}

void MainWindow::viewOpenFile(QString filename)
{
  if(!fileSaved) save_file();
  openFile.close();
  treeScene.clear();
  treeNodes.clear();
  ui->XMLTextEdit->clear();
  ui->statusbar->setStyleSheet("");
  ui->statusbar->clearMessage();
  openFile.setFileName(filename);
  openFile.open(QFile::ReadOnly | QFile::Text);
  if(openFile.isOpen())
  {
    QTextStream openFileStream(&openFile);
    delete listOfTokens;
    listOfTokens = extract_list_of_tokens(openFileStream.readAll().toStdString());
    openFileStream.reset();
    openFile.close();
    openFile.setFileName(filename);
    openFile.open(QFile::ReadOnly | QFile::Text);
    openFileStream.setDevice(&openFile);
    std::queue<token_type> *tempList = extract_list_of_tokens(openFileStream.readAll().toStdString());
    openFileStream.reset();
    ui->listOfTokensTable->clearContents();
    ui->listOfTokensTable->setRowCount(tempList->size());
    for(unsigned int i=0;!tempList->empty();i++)
      {
        ui->listOfTokensTable->setItem(i,0,new QTableWidgetItem(QString(tempList->front().tokenValue)));
        ui->listOfTokensTable->setItem(i,1,new QTableWidgetItem(QString(tokenName[tempList->front().tokenType])));
        tempList->pop();
      }
    delete tempList;
  }
}

void MainWindow::on_actionOpen_triggered()
{
  on_browsePushButton_clicked();
}

void MainWindow::on_actionExport_image_as_png_triggered()
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                ".\\untitled.png",
                                tr("Portable Network Graphics files (*.png)"));
  if(!fileName.isEmpty())
    {
      treeScene.clearSelection();
      treeScene.setSceneRect(treeScene.itemsBoundingRect());
      QImage image(treeScene.sceneRect().size().toSize(), QImage::Format_ARGB32);
      image.fill(Qt::transparent);
      QPainter painter(&image);
      treeScene.render(&painter);
      image.save(fileName);

    }
}

void MainWindow::on_actionSave_syntax_tree_as_XML_triggered()
{
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                                ".\\untitled.xml",
                                tr("XML files (*.xml)"));
    if(!fileName.isEmpty())
      {
        QFile exportedFile(fileName);
        exportedFile.open(QFile::WriteOnly|QFile::Text);
        QTextStream exportFileStream(&exportedFile);
        exportFileStream<<ui->XMLTextEdit->document()->toPlainText();
        exportFileStream.flush();
        exportedFile.close();
        fileSaved=true;
      }
}

void MainWindow::save_file()
{
  QMessageBox::StandardButton reply =
  QMessageBox::question(this , "Syntax Tree was not saved" ,
                           "The Syntax Tree was changed since the last save \n"
                           " Do you want to save it as XML before leaving?",
                           QMessageBox::Yes|QMessageBox::No , QMessageBox::Yes);
  if(reply == QMessageBox::Yes)
    {
        on_actionSave_syntax_tree_as_XML_triggered();
    }
}

void MainWindow::on_actionExit_triggered()
{
  this->close();
}

void MainWindow::on_actionAbout_QT_triggered()
{
  QMessageBox::aboutQt(this , "About QT");
}

void MainWindow::on_actionAbout_triggered()
{
  QMessageBox::about(this, "About",
                       "This program is made for the second assignment of Systems Software course"
                       " for fourth year CSE in Faculty of Engineering ASU.\n"
                       "This entry is submitted by:\n"
                       "Bassem Osama Farouk (1500400)\n"
                       "Khaled Shehab Shams Eldin (15X0031)\n"
                       "Ayman Mohamed Saad Eldin (14T0045)\n"
                       "Assem AbdElMoniem Ibrahim (1205036)\n"
                       "Kirollos Samir Asaad (1601023)");
}

void MainWindow::on_actionHow_to_use_triggered()
{
  QMessageBox::information(this,"How to use",
                             "Press the browse button to choose a file that contains the list of tokens\n"
                             "The file must be either a csv file or a text file that has a token value "
                             "then token type separated by a comma on each line\n"
                             "It's preferred that the input file is taken from the Tiny Scanner program\n"
                             "Press the \"Start Parsing\" button to parse\n"
                             "the state of the parse is displayed in the status bar with the syntax tree "
                             "displayed either as a diagram or an XML file",
                             QMessageBox::Ok , QMessageBox::Ok);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!fileSaved)
      {
        save_file();
      }
    event->accept();
}
