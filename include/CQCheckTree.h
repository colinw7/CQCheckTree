#ifndef CQCheckTree_H
#define CQCheckTree_H

#include <QTreeWidget>
#include <vector>

class CQCheckTree;
class CQCheckTreeCheck;

using CQCheckTreeIndex = std::pair<int, int>;

//---

class CQCheckTreeSection : public QTreeWidgetItem {
 public:
  enum { ITEM_ID = QTreeWidgetItem::UserType + 101 };

 public:
  CQCheckTreeSection(CQCheckTree *tree, const QString &text) :
   QTreeWidgetItem(ITEM_ID), tree_(tree), text_(text) {
    setData(0, Qt::DisplayRole, QVariant(text_));
  }

  const QString &text() const { return text_; }

  Qt::CheckState checkState() const;

  void setChecked(bool checked);

  int addCheck(CQCheckTreeCheck *check);

  uint numChecks() const { return uint(checks_.size()); }

 private:
  friend class CQCheckTree;
  friend class CQCheckTreeCheck;

  bool isItemChecked(int checkInd) const;
  void setItemChecked(int checkInd, bool checked);

  QString getItemText(int checkInd) const;

  void emitChecked(CQCheckTreeCheck *check, bool checked);

  int checkInd(CQCheckTreeCheck *) const;

 private:
  using Checks = std::vector<CQCheckTreeCheck *>;

  CQCheckTree *tree_ { nullptr };
  QString      text_;
  Checks       checks_;
};

//---

class CQCheckTreeCheck : public QTreeWidgetItem {
 public:
  enum { ITEM_ID = QTreeWidgetItem::UserType + 102 };

 public:
  CQCheckTreeCheck(CQCheckTreeSection *section, const QString &text) :
   QTreeWidgetItem(ITEM_ID), section_(section), text_(text), checked_(false) {
    setData(0, Qt::DisplayRole, QVariant(text_));
  }

  const QString &text() const { return text_; }

  bool isChecked() const { return checked_; }

  void setChecked(bool checked);

  QVariant data(int col, int role) const {
    if (role == Qt::ToolTipRole && col == 0)
      return QString(text_);
    else
      return QTreeWidgetItem::data(col, role);
  }

 private:
  CQCheckTreeSection *section_ { nullptr };
  QString             text_;
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

  CQCheckTreeIndex addCheck(int sectionInd, const QString &text);

  bool isItemChecked(const CQCheckTreeIndex &ind) const;
  void setItemChecked(const CQCheckTreeIndex &ind, bool checked);

  bool hasSection(const CQCheckTreeIndex &ind) const;

  QString getSectionText(const CQCheckTreeIndex &ind) const;
  QString getSectionText(int ind) const;

  QString getItemText(const CQCheckTreeIndex &ind) const;

 private:
  friend class CQCheckTreeSection;
  friend class CQCheckTreeDelegate;

  QTreeWidget *tree() const { return tree_; }

  QTreeWidgetItem *getModelItem(const QModelIndex &index) const;

  void emitChecked(CQCheckTreeSection *section, int itemNum, bool checked);

  int sectionInd(CQCheckTreeSection *) const;

 private slots:
  void itemClicked(const QModelIndex &index);

 signals:
  void itemChecked(const CQCheckTreeIndex &ind, bool checked);

  void sectionClicked(int sectionInd);
  void itemClicked(const CQCheckTreeIndex &ind);

 private:
  using Sections = std::vector<CQCheckTreeSection *>;

  QTreeWidget *tree_ { nullptr };
  Sections     sections_;
};

#endif
