#include <CQCheckTreeTest.h>
#include <CQApp/CQApp.h>
#include <QVBoxLayout>
#include <iostream>

int
main(int argc, char **argv)
{
  CQApp app(argc, argv);

  auto font = app.font();

  font.setPointSize(20);

  app.setFont(font);

  CQCheckTreeTest *test = new CQCheckTreeTest;

  test->show();

  return app.exec();
}

CQCheckTreeTest::
CQCheckTreeTest(QWidget *parent) :
 QWidget(parent)
{
  auto *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  tree_ = new CQCheckTree(this);

  tree_->setHeaders(QStringList() << "Item" << "Checked");

  int section1 = tree_->addSection("Section 1");
  int section2 = tree_->addSection("Section 2");

  /* auto check1 = */ tree_->addCheck(section1, "One");
  /* auto check2 = */ tree_->addCheck(section1, "Two");
  /* auto check3 = */ tree_->addCheck(section1, "Three");
  /* auto check4 = */ tree_->addCheck(section1, "Four");

  /* auto check5 = */ tree_->addCheck(section2, "Five");
  /* auto check6 = */ tree_->addCheck(section2, "Six");
  /* auto check7 = */ tree_->addCheck(section2, "Seven");
  /* auto check8 = */ tree_->addCheck(section2, "Eight");

  /* auto check9 = */ tree_->addCheck(-1, "Nine");

  connect(tree_, SIGNAL(itemChecked(const CQCheckTreeIndex &, bool)),
          this, SLOT(itemChecked(const CQCheckTreeIndex &, bool)));
  connect(tree_, SIGNAL(sectionClicked(int)),
          this, SLOT(sectionClicked(int)));
  connect(tree_, SIGNAL(itemClicked(const CQCheckTreeIndex &)),
          this, SLOT(itemClicked(const CQCheckTreeIndex &)));

  layout->addWidget(tree_);

  //layout->addStretch(1);
}

void
CQCheckTreeTest::
itemChecked(const CQCheckTreeIndex &ind, bool /*checked*/)
{
  auto name    = tree_->getItemText(ind);
  bool checked = tree_->isItemChecked(ind);

  if (tree_->hasSection(ind))
    name = tree_->getSectionText(ind) + "/" + name;

  std::cerr << "Item "<< name.toStdString() << " " <<
               (checked ? "Checked" : "Unchecked") << "\n";
}

void
CQCheckTreeTest::
sectionClicked(int ind)
{
  auto name = tree_->getSectionText(ind);

  std::cerr << "Section " << name.toStdString() << " clicked\n";
}

void
CQCheckTreeTest::
itemClicked(const CQCheckTreeIndex &ind)
{
  auto name = tree_->getItemText(ind);

  if (tree_->hasSection(ind))
    name = tree_->getSectionText(ind) + "/" + name;

  std::cerr << "Item " << name.toStdString() << " clicked\n";
}
