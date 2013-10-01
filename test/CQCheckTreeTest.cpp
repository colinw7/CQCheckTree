#include <CQCheckTreeTest.h>
#include <QApplication>
#include <QVBoxLayout>
#include <iostream>

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  CQCheckTreeTest *test = new CQCheckTreeTest;

  test->show();

  return app.exec();
}

CQCheckTreeTest::
CQCheckTreeTest(QWidget *parent) :
 QWidget(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  tree_ = new CQCheckTree(this);

  int section1 = tree_->addSection("Section 1");
  int section2 = tree_->addSection("Section 2");

  CQCheckTreeIndex check1 = tree_->addCheck(section1, "One");
  CQCheckTreeIndex check2 = tree_->addCheck(section1, "Two");
  CQCheckTreeIndex check3 = tree_->addCheck(section1, "Three");
  CQCheckTreeIndex check4 = tree_->addCheck(section1, "Four");

  CQCheckTreeIndex check5 = tree_->addCheck(section2, "Five");
  CQCheckTreeIndex check6 = tree_->addCheck(section2, "Six");
  CQCheckTreeIndex check7 = tree_->addCheck(section2, "Seven");
  CQCheckTreeIndex check8 = tree_->addCheck(section2, "Eight");

  connect(tree_, SIGNAL(itemChecked(const CQCheckTreeIndex &,bool)),
          this, SLOT(itemChecked(const CQCheckTreeIndex &,bool)));
  connect(tree_, SIGNAL(sectionClicked(int)),
          this, SLOT(sectionClicked(int)));
  connect(tree_, SIGNAL(itemClicked(const CQCheckTreeIndex &)),
          this, SLOT(itemClicked(const CQCheckTreeIndex &)));


  layout->addWidget(tree_);
  layout->addStretch(1);
}

void
CQCheckTreeTest::
itemChecked(const CQCheckTreeIndex &ind, bool /*checked*/)
{
  QString name    = tree_->getItemText(ind);
  bool    checked = tree_->isItemChecked(ind);

  std::cerr << name.toStdString() << " " << (checked ? "Checked" : "Unchecked") << std::endl;
}

void
CQCheckTreeTest::
sectionClicked(int ind)
{
  QString name = tree_->getSectionText(ind);

  std::cerr << "Section " << name.toStdString() << " clicked" << std::endl;
}

void
CQCheckTreeTest::
itemClicked(const CQCheckTreeIndex &ind)
{
  QString name = tree_->getItemText(ind);

  std::cerr << "Item " << name.toStdString() << " clicked" << std::endl;
}
