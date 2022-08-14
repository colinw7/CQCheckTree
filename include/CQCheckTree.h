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

  friend bool operator<(const CQCheckTreeIndex &lhs, const CQCheckTreeIndex &rhs) {
    return cmp(lhs, rhs) < 0;
  }

  static int cmp(const CQCheckTreeIndex &lhs, const CQCheckTreeIndex &rhs) {
    if (lhs.sectionInd != rhs.sectionInd)
      return (lhs.sectionInd > rhs.sectionInd ? 1 : -1);

    if (lhs.subSectionInd != rhs.subSectionInd)
      return (lhs.subSectionInd > rhs.subSectionInd ? 1 : -1);

    if (lhs.itemInd != rhs.itemInd)
      return (lhs.itemInd > rhs.itemInd ? 1 : -1);

    return 0;
  }
};

//---

class CQCheckTreeItem : public QTreeWidgetItem {
 public:
  CQCheckTreeItem(CQCheckTree *tree, int id);

  virtual ~CQCheckTreeItem() { }

  CQCheckTree *tree() const { return tree_; }

  virtual QString hierName() const = 0;

  int ind() const { return ind_; }
  void setInd(int i) { ind_ = i; }

  const CQCheckTreeIndex &index() const { return index_; }
  void setIndex(const CQCheckTreeIndex &v) { index_ = v; }

 protected:
  CQCheckTree*     tree_ { nullptr };
  int              ind_  { -1 };
  CQCheckTreeIndex index_;
};

//---

class CQCheckTreeSection : public CQCheckTreeItem {
 public:
  using Items    = std::vector<CQCheckTreeItem *>;
  using Sections = std::vector<CQCheckTreeSection *>;
  using Checks   = std::vector<CQCheckTreeCheck *>;

 public:
  enum { ITEM_ID = CQCheckTreeItem::UserType + 101 };

 public:
  CQCheckTreeSection(CQCheckTree *tree, const QString &text);

  virtual ~CQCheckTreeSection() { }

  CQCheckTreeSection *section() const { return section_; }
  void setSection(CQCheckTreeSection *section) { section_ = section; }

  const QString &text() const { return text_; }

  const Sections &sections() const { return sections_; }
  const Checks &checks() const { return checks_; }

  //---

  QString hierName() const override;

  //---

  Qt::CheckState checkState() const;

  void setChecked(bool checked);

  int addSection(const QString &section);

  CQCheckTreeCheck *addCheck(int sectionInd, const QString &name);

  int addCheck(CQCheckTreeCheck *check);

  uint numChecks() const { return uint(checks_.size()); }

  QString getSectionText(int ind) const;

  void updateInds(const QModelIndex &parent) const;

  Items getAllItems() const;
  Items getCheckedItems() const;

  QVariant data(int col, int role) const override {
    if (role == Qt::ToolTipRole && col == 0)
      return hierName();

    return CQCheckTreeItem::data(col, role);
  }

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
  CQCheckTreeSection *section_ { nullptr };
  QString             text_;
  Sections            sections_;
  Checks              checks_;
};

//---

class CQCheckTreeCheck : public CQCheckTreeItem {
 public:
  enum { ITEM_ID = CQCheckTreeItem::UserType + 102 };

 public:
  CQCheckTreeCheck(CQCheckTree *tree, CQCheckTreeSection *section, const QString &text);

  virtual ~CQCheckTreeCheck() { }

  CQCheckTreeSection *section() const { return section_; }

  const QString &text() const { return text_; }

  //---

  QString hierName() const override;

  //---

  bool isChecked() const { return checked_; }
  void setChecked(bool checked);

  QVariant data(int col, int role) const override {
    if (role == Qt::ToolTipRole && col == 0)
      return hierName();

    return CQCheckTreeItem::data(col, role);
  }

 private:
  CQCheckTreeSection *section_ { nullptr };
  QString             text_;
  bool                checked_ { false };
};

//---

class CQCheckTreeWidget : public QTreeWidget {
  Q_OBJECT

 public:
  CQCheckTreeWidget(CQCheckTree *tree);

  void setHeaderLabels(const QStringList &labels);

 private:
  CQCheckTree *tree_ { nullptr };
};

//---

class CQCheckTree : public QFrame {
  Q_OBJECT

  Q_PROPERTY(int checkSize READ checkSize WRITE setCheckSize)

 public:
  using Items    = std::vector<CQCheckTreeItem *>;
  using Sections = std::vector<CQCheckTreeSection *>;
  using Checks   = std::vector<CQCheckTreeCheck *>;

 public:
  CQCheckTree(QWidget *parent=nullptr);
 ~CQCheckTree();

  QTreeWidget *tree() const { return tree_; }

  int checkSize() const { return checkSize_; }
  void setCheckSize(int i) { checkSize_ = i; }

  const Sections &sections() const { return sections_; }
  const Checks &checks() const { return checks_; }

  const QChar &hierSep() const { return hierSep_; }
  void setHierSep(const QChar &v) { hierSep_ = v; }

  //---

  void setHeaders(const QStringList &headers);

  void clear();

  // add section
  CQCheckTreeIndex addSection(const QString &section);
  CQCheckTreeIndex addSection(const CQCheckTreeIndex &ind, const QString &section);
  CQCheckTreeIndex addSection(int sectionInd, const QString &section);

  // add check
  CQCheckTreeIndex addCheck(const QString &name);
  CQCheckTreeIndex addCheck(const CQCheckTreeIndex &ind, const QString &name);
  CQCheckTreeIndex addCheck(int subSectionInd, const QString &name);
  CQCheckTreeIndex addCheck(int sectionInd, int subSectionInd, const QString &name);

  bool isItemChecked(const CQCheckTreeIndex &ind) const;
  void setItemChecked(const CQCheckTreeIndex &ind, bool checked);

  bool hasSection(const CQCheckTreeIndex &ind) const;

  QString getSectionText(const CQCheckTreeIndex &ind) const;
  QString getSectionText(int sectionInd) const;
  QString getSectionText(int sectionInd, int subSectionInd) const;

  QString getItemText(const CQCheckTreeIndex &ind) const;

  Items getAllItems() const;
  Items getCheckedItems() const;

 private:
  friend class CQCheckTreeSection;
  friend class CQCheckTreeDelegate;
  friend class CQCheckTreeCheck;

  CQCheckTreeItem *getModelItem(const QModelIndex &index) const;

  void emitChecked(CQCheckTreeSection *section, int itemNum, bool checked);

 public Q_SLOTS:
  void expandAll();
  void collapseAll();
  void fitColumns();

 private Q_SLOTS:
  void itemClicked(const QModelIndex &index);

  void customContextMenuSlot(const QPoint &pos);

 Q_SIGNALS:
  void itemChecked(const CQCheckTreeIndex &ind, bool checked);

  void sectionClicked(int sectionInd);
  void subSectionClicked(int sectionInd, int subSectionInd);
  void itemClicked(const CQCheckTreeIndex &ind);

 private:
  QTreeWidget *tree_      { nullptr };
  int          checkSize_ { 12 };
  QChar        hierSep_   { '/' };
  Sections     sections_;
  Checks       checks_;
  QPoint       menuPos_;
};

#endif
