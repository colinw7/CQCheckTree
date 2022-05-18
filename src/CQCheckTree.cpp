#include <CQCheckTree.h>

#include <QVBoxLayout>
#include <QItemDelegate>
#include <QPainter>
#include <QMouseEvent>

#include <cassert>
#include <iostream>

namespace {
  enum { CHECK_SIZE = 12 };
}

class CQCheckTreeDelegate : public QItemDelegate {
 public:
  CQCheckTreeDelegate(CQCheckTree *tree);

  void paint(QPainter *painter, const QStyleOptionViewItem &option,
             const QModelIndex &index) const override;

  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

 private:
  CQCheckTree *tree_;
};

//------

CQCheckTree::
CQCheckTree(QWidget *parent) :
 QWidget(parent)
{
  auto *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  tree_ = new QTreeWidget(this);

  tree_->setItemDelegate(new CQCheckTreeDelegate(this));

  tree_->setSelectionBehavior(QAbstractItemView::SelectItems);

  tree_->setSelectionMode(QAbstractItemView::NoSelection);

  layout->addWidget(tree_);

  tree_->setColumnCount(2);

  QStringList headers;

  headers.push_back("Type");
  headers.push_back("Selected");

  tree_->setHeaderLabels(headers);

  connect(tree_, SIGNAL(clicked(const QModelIndex &)),
          this, SLOT(itemClicked(const QModelIndex &)));
}

CQCheckTree::
~CQCheckTree()
{
}

void
CQCheckTree::
setHeaders(const QStringList &headers)
{
  if (headers.size() != 2) return;

  tree_->setHeaderLabels(headers);
}

void
CQCheckTree::
clear()
{
  tree_->clear();

  sections_.clear();
  checks_  .clear();
}

int
CQCheckTree::
addSection(const QString &section)
{
  auto *sectionItem = new CQCheckTreeSection(this, section);

  tree_->addTopLevelItem(sectionItem);

  sections_.push_back(sectionItem);

  int n = int(sections_.size() - 1);

  sectionItem->setInd(n);

  return n;
}

CQCheckTreeIndex
CQCheckTree::
addCheck(int sectionInd, const QString &text)
{
  // add toplevel item
  if (sectionInd < 0 || sectionInd >= int(sections_.size())) {
    auto *checkItem = new CQCheckTreeCheck(this, nullptr, text);

    tree_->addTopLevelItem(checkItem);

    checks_.push_back(checkItem);

    int n = int(checks_.size() - 1);

    checkItem->setInd(n);

    return CQCheckTreeIndex(-1, checkItem->ind());
  }

  //---

  // add section item
  auto *sectionItem = sections_[size_t(sectionInd)];

  auto *checkItem = new CQCheckTreeCheck(this, sectionItem, text);

  int itemInd = sectionItem->addCheck(checkItem);

  checkItem->setInd(itemInd);

  return CQCheckTreeIndex(sectionInd, itemInd);
}

bool
CQCheckTree::
isItemChecked(const CQCheckTreeIndex &ind) const
{
  int sectionInd = ind.first;
  int checkInd   = ind.second;

  if (sectionInd < 0 || sectionInd >= int(sections_.size())) {
    CQCheckTreeCheck *checkItem = nullptr;

    if (checkInd >= 0 && checkInd < int(checks_.size()))
      checkItem = checks_[size_t(checkInd)];

    return (checkItem ? checkItem->isChecked() : false);
  }

  auto *sectionItem = sections_[size_t(sectionInd)];

  return sectionItem->isItemChecked(checkInd);
}

void
CQCheckTree::
setItemChecked(const CQCheckTreeIndex &ind, bool checked)
{
  int sectionInd = ind.first;
  int checkInd   = ind.second;

  if (sectionInd < 0 || sectionInd >= int(sections_.size())) {
    CQCheckTreeCheck *checkItem = nullptr;

    if (checkInd >= 0 && checkInd < int(checks_.size()))
      checkItem = checks_[size_t(checkInd)];

    if (checkItem)
      checkItem->setChecked(checked);

    return;
  }

  auto *sectionItem = sections_[size_t(sectionInd)];

  sectionItem->setItemChecked(checkInd, checked);
}

bool
CQCheckTree::
hasSection(const CQCheckTreeIndex &ind) const
{
  return (ind.first >= 0 && ind.first < int(sections_.size()));
}

QString
CQCheckTree::
getSectionText(const CQCheckTreeIndex &ind) const
{
  return getSectionText(ind.first);
}

QString
CQCheckTree::
getSectionText(int sectionInd) const
{
  if (sectionInd < 0 || sectionInd >= int(sections_.size()))
    return "";

  auto *sectionItem = sections_[size_t(sectionInd)];

  return sectionItem->text();
}

QString
CQCheckTree::
getItemText(const CQCheckTreeIndex &ind) const
{
  int sectionInd = ind.first;
  int checkInd   = ind.second;

  if (sectionInd < 0 || sectionInd >= int(sections_.size())) {
    CQCheckTreeCheck *checkItem = nullptr;

    if (checkInd >= 0 && checkInd < int(checks_.size()))
      checkItem = checks_[size_t(checkInd)];

    return (checkItem ? checkItem->text() : "");
  }

  auto *sectionItem = sections_[size_t(sectionInd)];

  return sectionItem->getItemText(checkInd);
}

QTreeWidgetItem *
CQCheckTree::
getModelItem(const QModelIndex &index) const
{
  QTreeWidgetItem *item;

  if (! index.parent().isValid())
    item = tree_->topLevelItem(index.row());
  else {
    auto *parent = getModelItem(index.parent());
    assert(parent);

    item = parent->child(index.row());
  }

  return item;
}

void
CQCheckTree::
itemClicked(const QModelIndex &index)
{
  auto *item = getModelItem(index);
  if (! item) return;

  if (index.column() != 1)
    return;

  if      (item->type() == CQCheckTreeSection::ITEM_ID) {
    auto *section = static_cast<CQCheckTreeSection *>(item);

    auto checkState = section->checkState();

    section->setChecked(checkState != Qt::Checked);

    tree_->update(index);

    for (uint i = 0; i < section->numChecks(); ++i) {
      //auto ind = index.child(int(i), 1);
      auto ind = index.model()->index(int(i), 1, index);

      tree_->update(ind);
    }

    emit sectionClicked(section->ind());
  }
  else if (item->type() == CQCheckTreeCheck::ITEM_ID) {
    auto *check = static_cast<CQCheckTreeCheck *>(item);

    check->setChecked(! check->isChecked());

    auto *section = check->section();

    int sectionInd = (section ? section->ind() : -1);

    tree_->update(index);

    auto pi = index.parent();

    auto pi1 = tree_->model()->index(pi.row(), 1);

    tree_->update(pi1);

    emit itemClicked(CQCheckTreeIndex(sectionInd, check->ind()));
  }
}

void
CQCheckTree::
emitChecked(CQCheckTreeSection *section, int itemNum, bool checked)
{
  int sectionInd = (section ? section->ind() : -1);

  emit itemChecked(CQCheckTreeIndex(sectionInd, itemNum), checked);
}

#if 0
int
CQCheckTree::
sectionInd(CQCheckTreeSection *section) const
{
  for (uint i = 0; i < sections_.size(); ++i)
    if (sections_[i] == section)
      return int(i);

  return -1;
}
#endif

//------

CQCheckTreeDelegate::
CQCheckTreeDelegate(CQCheckTree *tree) :
 tree_(tree)
{
}

void
CQCheckTreeDelegate::
paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if (index.column() == 1) {
    auto *item = tree_->getModelItem(index);
    assert(item);

    auto checkState = Qt::Unchecked;

    if      (item->type() == CQCheckTreeSection::ITEM_ID) {
      auto *section = static_cast<CQCheckTreeSection *>(item);

      checkState = section->checkState();
    }
    else if (item->type() == CQCheckTreeCheck::ITEM_ID) {
      auto *check = static_cast<CQCheckTreeCheck *>(item);

      checkState = (check->isChecked() ? Qt::Checked : Qt::Unchecked);
    }
    else
      assert(false);

    painter->save();

    int dy = (option.rect.height() - CHECK_SIZE)/2;

    int x = option.rect.left() + 2;
    int y = option.rect.top () + dy;

    QRect rect(x, y, CHECK_SIZE, CHECK_SIZE);

    drawCheck(painter, option, rect, checkState);

    painter->restore();
  }
  else
    QItemDelegate::paint(painter, option, index);
}

QSize
CQCheckTreeDelegate::
sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  if (index.column() == 1)
    return QSize(CHECK_SIZE, CHECK_SIZE);
  else
    return QItemDelegate::sizeHint(option, index);
}

//------

CQCheckTreeSection::
CQCheckTreeSection(CQCheckTree *tree, const QString &text) :
 QTreeWidgetItem(ITEM_ID), tree_(tree), text_(text)
{
  setData(0, Qt::DisplayRole, QVariant(text_));
}

Qt::CheckState
CQCheckTreeSection::
checkState() const
{
  uint numChecked = 0;

  for (uint i = 0; i < checks_.size(); ++i)
    if (checks_[i]->isChecked())
      ++numChecked;

  if      (numChecked == 0)
    return Qt::Unchecked;
  else if (numChecked == checks_.size())
    return Qt::Checked;
  else
    return Qt::PartiallyChecked;
}

void
CQCheckTreeSection::
setChecked(bool checked)
{
  for (uint i = 0; i < checks_.size(); ++i)
    checks_[i]->setChecked(checked);

  emitDataChanged();
}

int
CQCheckTreeSection::
addCheck(CQCheckTreeCheck *check)
{
  addChild(check);

  checks_.push_back(check);

  return int(checks_.size() - 1);
}

bool
CQCheckTreeSection::
isItemChecked(int checkInd) const
{
  if (checkInd < 0 || checkInd >= int(checks_.size()))
    return false;

  auto *checkItem = checks_[size_t(checkInd)];

  return checkItem->isChecked();
}

void
CQCheckTreeSection::
setItemChecked(int checkInd, bool checked)
{
  if (checkInd < 0 || checkInd >= int(checks_.size()))
    return;

  auto *checkItem = checks_[size_t(checkInd)];

  checkItem->setChecked(checked);
}

QString
CQCheckTreeSection::
getItemText(int checkInd) const
{
  if (checkInd < 0 || checkInd >= int(checks_.size()))
    return "";

  auto *checkItem = checks_[size_t(checkInd)];

  return checkItem->text();
}

void
CQCheckTreeSection::
emitChecked(CQCheckTreeCheck *check, bool checked)
{
  int checkInd = this->checkInd(check);

  if (checkInd >= 0)
    tree_->emitChecked(this, checkInd, checked);
}

int
CQCheckTreeSection::
checkInd(CQCheckTreeCheck *check) const
{
  for (uint i = 0; i < checks_.size(); ++i)
    if (checks_[i] == check)
      return int(i);

  return -1;
}

//------

CQCheckTreeCheck::
CQCheckTreeCheck(CQCheckTree *tree, CQCheckTreeSection *section, const QString &text) :
 QTreeWidgetItem(ITEM_ID), tree_(tree), section_(section), text_(text)
{
  setData(0, Qt::DisplayRole, QVariant(text_));
}

void
CQCheckTreeCheck::
setChecked(bool checked)
{
  if (checked_ == checked)
    return;

  checked_ = checked;

  if (section_)
    section_->emitChecked(this, checked_);
  else
    tree_->emitChecked(nullptr, ind(), checked_);

  emitDataChanged();
}
