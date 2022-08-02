#ifndef CQCheckTree_H
#define CQCheckTree_H

#include <QTreeWidget>
#include <vector>

class CQCheckTree;
class CQCheckTreeCheck;

struct CQCheckTreeIndex {
  int sectionInd    { -1 };
  int subSectionInd { -1 };
  int itemInd       { -1 };

  CQCheckTreeIndex() { }

  CQCheckTreeIndex(int itemInd) :
   itemInd(itemInd) {
  }

  CQCheckTreeIndex(int sectionInd, int itemInd) :
   sectionInd(sectionInd), itemInd(itemInd) {
  }

  CQCheckTreeIndex(int sectionInd, int subSectionInd, int itemInd) :
   sectionInd(sectionInd), subSectionInd(subSectionInd), itemInd(itemInd) {
  }
};

//---

class CQCheckTreeSection : public QTreeWidgetItem {
 public:
  enum { ITEM_ID = QTreeWidgetItem::UserType + 101 };

 public:
  CQCheckTreeSection(CQCheckTree *tree, const QString &text);

  CQCheckTree *tree() const { return tree_; }

  CQCheckTreeSection *section() const { return section_; }
  void setSection(CQCheckTreeSection *section) { section_ = section; }

  const QString &text() const { return text_; }

  int ind() const { return ind_; }
  void setInd(int i) { ind_ = i; }

  Qt::CheckState checkState() const;

  void setChecked(bool checked);

  int addSection(const QString &section);

  CQCheckTreeIndex addCheck(int sectionInd, const QString &name);

  int addCheck(CQCheckTreeCheck *check);

  uint numChecks() const { return uint(checks_.size()); }

  QString getSectionText(int ind) const;

  void updateInds(const QModelIndex &parent) const;

 private:
  friend class CQCheckTree;
  friend class CQCheckTreeCheck;

  bool isItemChecked(int checkInd) const;
  bool isItemChecked(int sectionInd, int checkInd) const;

  void setItemChecked(int checkInd, bool checked);
  void setItemChecked(int sectionInd, int checkInd, bool checked);

  bool hasSection(int sectionInd) const;

  QString getItemText(int sectionInd, int checkInd) const;
  QString getItemText(int checkInd) const;

  void emitChecked(CQCheckTreeCheck *check, bool checked);

  int checkInd(CQCheckTreeCheck *) const;

 private:
  using Sections = std::vector<CQCheckTreeSection *>;
  using Checks   = std::vector<CQCheckTreeCheck *>;

  CQCheckTree        *tree_    { nullptr };
  CQCheckTreeSection *section_ { nullptr };
  QString             text_;
  int                 ind_     { -1 };
  Sections            sections_;
  Checks              checks_;
};

//---

class CQCheckTreeCheck : public QTreeWidgetItem {
 public:
  enum { ITEM_ID = QTreeWidgetItem::UserType + 102 };

 public:
  CQCheckTreeCheck(CQCheckTree *tree, CQCheckTreeSection *section, const QString &text);

  CQCheckTree *tree() const { return tree_; }

  CQCheckTreeSection *section() const { return section_; }

  const QString &text() const { return text_; }

  bool isChecked() const { return checked_; }
  void setChecked(bool checked);

  int ind() const { return ind_; }
  void setInd(int i) { ind_ = i; }

  QVariant data(int col, int role) const {
    if (role == Qt::ToolTipRole && col == 0)
      return QString(text_);
    else
      return QTreeWidgetItem::data(col, role);
  }

 private:
  CQCheckTree        *tree_    { nullptr };
  CQCheckTreeSection *section_ { nullptr };
  QString             text_;
  int                 ind_     { -1 };
  bool                checked_ { false };
};

//---

class CQCheckTree : public QWidget {
  Q_OBJECT

 public:
  CQCheckTree(QWidget *parent=nullptr);
 ~CQCheckTree();

  void setHeaders(const QStringList &headers);

  void clear();

  int addSection(const QString &section);

  int addSubSection(int sectionInd, const QString &section);

  CQCheckTreeIndex addCheck(int sectionInd, int subSectionInd, const QString &name);

  CQCheckTreeIndex addCheck(int sectionInd, const QString &text);

  bool isItemChecked(const CQCheckTreeIndex &ind) const;
  void setItemChecked(const CQCheckTreeIndex &ind, bool checked);

  bool hasSection(const CQCheckTreeIndex &ind) const;

  QString getSectionText(const CQCheckTreeIndex &ind) const;
  QString getSectionText(int sectionInd) const;
  QString getSubSectionText(int sectionInd, int subSectionInd) const;

  QString getItemText(const CQCheckTreeIndex &ind) const;

 private:
  friend class CQCheckTreeSection;
  friend class CQCheckTreeDelegate;
  friend class CQCheckTreeCheck;

  QTreeWidget *tree() const { return tree_; }

  QTreeWidgetItem *getModelItem(const QModelIndex &index) const;

  void emitChecked(CQCheckTreeSection *section, int itemNum, bool checked);

  //int sectionInd(CQCheckTreeSection *) const;

 private Q_SLOTS:
  void itemClicked(const QModelIndex &index);

 Q_SIGNALS:
  void itemChecked(const CQCheckTreeIndex &ind, bool checked);

  void sectionClicked(int sectionInd);
  void subSectionClicked(int sectionInd, int subSectionInd);
  void itemClicked(const CQCheckTreeIndex &ind);

 private:
  using Sections = std::vector<CQCheckTreeSection *>;
  using Checks   = std::vector<CQCheckTreeCheck *>;

  QTreeWidget *tree_ { nullptr };
  Sections     sections_;
  Checks       checks_;
};

#endif
