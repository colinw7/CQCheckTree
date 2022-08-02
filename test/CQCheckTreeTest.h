#include <QWidget>
#include <CQCheckTree.h>

class CQCheckTreeTest : public QWidget {
  Q_OBJECT

 public:
  CQCheckTreeTest(QWidget *parent=0);

 private slots:
  void itemChecked(const CQCheckTreeIndex &ind, bool checked);

  void sectionClicked(int sectionInd);
  void subSectionClicked(int sectionInd, int subSectionInd);

  void itemClicked(const CQCheckTreeIndex &ind);

  void printState();

 private:
  CQCheckTree *tree_ { nullptr };
};
