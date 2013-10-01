#include <QWidget>
#include <CQCheckTree.h>

class CQCheckTreeTest : public QWidget {
  Q_OBJECT

 public:
  CQCheckTreeTest(QWidget *parent=0);

 private slots:
  void itemChecked(const CQCheckTreeIndex &ind, bool checked);

  void sectionClicked(int ind);

  void itemClicked(const CQCheckTreeIndex &ind);

 private:
  CQCheckTree *tree_;
};
