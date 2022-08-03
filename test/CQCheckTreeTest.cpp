#include <CQCheckTreeTest.h>
#ifdef USE_QT_APP
#include <CQApp.h>
#else
#include <QApplication>
#endif
#include <QVBoxLayout>
#include <iostream>

int
main(int argc, char **argv)
{
#ifdef USE_QT_APP
  CQApp app(argc, argv);
#else
  QApplication app(argc, argv);
#endif

  auto font = app.font();

  font.setPointSize(24);

  app.setFont(font);

  auto *test = new CQCheckTreeTest;

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

  auto section1 = tree_->addSection("Section 1");

  /* auto check1 = */ tree_->addCheck(section1, "One");
  /* auto check2 = */ tree_->addCheck(section1, "Two");
  /* auto check3 = */ tree_->addCheck(section1, "Three");
  /* auto check4 = */ tree_->addCheck(section1, "Four");

  /* auto check5 = */ tree_->addCheck("Five");

  auto section2 = tree_->addSection("Section 2");

  /* auto check6 = */ tree_->addCheck(section2, "Six");
  /* auto check7 = */ tree_->addCheck(section2, "Seven");
  /* auto check8 = */ tree_->addCheck(section2, "Eight");
  /* auto check9 = */ tree_->addCheck(section2, "Nine");

  auto subSection1 = tree_->addSection(section2, "Sub Section 1");

  /* auto subCheck1 = */ tree_->addCheck(subSection1, "Eleven");

  /* auto check10 = */ tree_->addCheck("Ten");

  connect(tree_, SIGNAL(itemChecked(const CQCheckTreeIndex &, bool)),
          this, SLOT(itemChecked(const CQCheckTreeIndex &, bool)));
  connect(tree_, SIGNAL(sectionClicked(int)),
          this, SLOT(sectionClicked(int)));
  connect(tree_, SIGNAL(subSectionClicked(int, int)),
          this, SLOT(subSectionClicked(int, int)));
  connect(tree_, SIGNAL(itemClicked(const CQCheckTreeIndex &)),
          this, SLOT(itemClicked(const CQCheckTreeIndex &)));

  layout->addWidget(tree_);

  //layout->addStretch(1);

  tree_->fitColumns();
}

void
CQCheckTreeTest::
itemChecked(const CQCheckTreeIndex &ind, bool /*checked*/)
{
  auto name    = tree_->getItemText(ind);
  bool checked = tree_->isItemChecked(ind);

  if (tree_->hasSection(ind))
    name = tree_->getSectionText(ind) + ":" + name;

  std::cerr << "Item "<< name.toStdString() << " " <<
               (checked ? "Checked" : "Unchecked") << "\n";

  printState();
}

void
CQCheckTreeTest::
sectionClicked(int sectionInd)
{
  auto name = tree_->getSectionText(sectionInd);

  std::cerr << "Section " << name.toStdString() << " clicked\n";
}

void
CQCheckTreeTest::
subSectionClicked(int sectionInd, int subSectionInd)
{
  auto name = tree_->getSectionText(sectionInd, subSectionInd);

  std::cerr << "Sub Section " << name.toStdString() << " clicked\n";
}

void
CQCheckTreeTest::
itemClicked(const CQCheckTreeIndex &ind)
{
  auto name = tree_->getItemText(ind);

  if (tree_->hasSection(ind))
    name = tree_->getSectionText(ind) + ":" + name;

  std::cerr << "Item " << name.toStdString() << " clicked\n";
}

void
CQCheckTreeTest::
printState()
{
  std::cerr << "Checked\n";

  auto items = tree_->getCheckedItems();

  for (const auto *item : items)
    std::cerr << "  " << item->hierName().toStdString() << "\n";
}
