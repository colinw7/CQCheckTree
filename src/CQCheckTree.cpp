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
  CQCheckTree *tree_ { nullptr };
};

//------

CQCheckTree::
CQCheckTree(QWidget *parent) :
 QWidget(parent)
{
  setObjectName("checkTree");

  auto *layout = new QVBoxLayout(this);
  layout->setMargin(0); layout->setSpacing(0);

  tree_ = new QTreeWidget(this);
  tree_->setObjectName("tree");

  tree_->setItemDelegate(new CQCheckTreeDelegate(this));

  tree_->setSelectionBehavior(QAbstractItemView::SelectItems);
  tree_->setEditTriggers(QAbstractItemView::NoEditTriggers);

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

int
CQCheckTree::
addSubSection(int sectionInd, const QString &section)
{
  assert(sectionInd >= 0 && sectionInd < int(sections_.size()));

  auto *sectionItem = sections_[size_t(sectionInd)];

  return sectionItem->addSection(section);
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

    return CQCheckTreeIndex(checkItem->ind()); // root
  }

  //---

  // add section item
  auto *sectionItem = sections_[size_t(sectionInd)];

  auto *checkItem = new CQCheckTreeCheck(this, sectionItem, text);

  int itemInd = sectionItem->addCheck(checkItem);

  checkItem->setInd(itemInd);

  return CQCheckTreeIndex(sectionInd, itemInd); // section
}

CQCheckTreeIndex
CQCheckTree::
addCheck(int sectionInd, int subSectionInd, const QString &text)
{
  // add toplevel item
  assert(sectionInd >= 0 && sectionInd < int(sections_.size()));

  //---

  // add section item
  auto *sectionItem = sections_[size_t(sectionInd)];

  return sectionItem->addCheck(subSectionInd, text);
}

bool
CQCheckTree::
isItemChecked(const CQCheckTreeIndex &ind) const
{
  int sectionInd    = ind.sectionInd;
  int subSectionInd = ind.subSectionInd;
  int checkInd      = ind.itemInd;

  // top level check
  if (sectionInd < 0 || sectionInd >= int(sections_.size())) {
    CQCheckTreeCheck *checkItem = nullptr;

    if (checkInd >= 0 && checkInd < int(checks_.size()))
      checkItem = checks_[size_t(checkInd)];

    return (checkItem ? checkItem->isChecked() : false);
  }

  // section check
  auto *sectionItem = sections_[size_t(sectionInd)];

  if (subSectionInd >= 0)
    return sectionItem->isItemChecked(subSectionInd, checkInd);
  else
    return sectionItem->isItemChecked(checkInd);
}

void
CQCheckTree::
setItemChecked(const CQCheckTreeIndex &ind, bool checked)
{
  int sectionInd    = ind.sectionInd;
  int subSectionInd = ind.subSectionInd;
  int checkInd      = ind.itemInd;

  // top level check
  if (sectionInd < 0 || sectionInd >= int(sections_.size())) {
    CQCheckTreeCheck *checkItem = nullptr;

    if (checkInd >= 0 && checkInd < int(checks_.size()))
      checkItem = checks_[size_t(checkInd)];

    if (checkItem)
      checkItem->setChecked(checked);

    return;
  }

  // section check
  auto *sectionItem = sections_[size_t(sectionInd)];

  if (subSectionInd >= 0)
    sectionItem->setItemChecked(subSectionInd, checkInd, checked);
  else
    sectionItem->setItemChecked(checkInd, checked);
}

bool
CQCheckTree::
hasSection(const CQCheckTreeIndex &ind) const
{
  int sectionInd    = ind.sectionInd;
  int subSectionInd = ind.subSectionInd;

  if (sectionInd < 0 || sectionInd >= int(sections_.size()))
    return false;

  if (subSectionInd < 0)
    return true;

  auto *sectionItem = sections_[size_t(sectionInd)];

  return sectionItem->hasSection(subSectionInd);
}

QString
CQCheckTree::
getSectionText(const CQCheckTreeIndex &ind) const
{
  QString sectionName, subSectionName;

  if (ind.sectionInd >= 0)
    sectionName = getSectionText(ind.sectionInd);

  if (ind.subSectionInd >= 0) {
    assert(ind.sectionInd >= 0);

    auto *sectionItem = sections_[size_t(ind.sectionInd)];

    subSectionName = sectionItem->getSectionText(ind.subSectionInd);
  }

  return QString("%1:%2").arg(sectionName).arg(subSectionName);
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
getSubSectionText(int sectionInd, int subSectionInd) const
{
  if (sectionInd < 0 || sectionInd >= int(sections_.size()))
    return "";

  auto *sectionItem = sections_[size_t(sectionInd)];

  return sectionItem->getSectionText(subSectionInd);
}

QString
CQCheckTree::
getItemText(const CQCheckTreeIndex &ind) const
{
  int sectionInd    = ind.sectionInd;
  int subSectionInd = ind.subSectionInd;
  int checkInd      = ind.itemInd;

  // toplevel item
  if (sectionInd < 0 || sectionInd >= int(sections_.size())) {
    CQCheckTreeCheck *checkItem = nullptr;

    if (checkInd >= 0 && checkInd < int(checks_.size()))
      checkItem = checks_[size_t(checkInd)];

    return (checkItem ? checkItem->text() : "");
  }

  auto *sectionItem = sections_[size_t(sectionInd)];

  if (subSectionInd >= 0)
    return sectionItem->getItemText(subSectionInd, checkInd);
  else
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

    section->updateInds(index);

    auto pi = index.parent();

    while (pi.isValid()) {
      auto pi1 = tree_->model()->index(pi.row(), 1, pi.parent());

      tree_->update(pi1);

      pi = pi.parent();
    }

    if (section->section())
      Q_EMIT subSectionClicked(section->section()->ind(), section->ind());
    else
      Q_EMIT sectionClicked(section->ind());
  }
  else if (item->type() == CQCheckTreeCheck::ITEM_ID) {
    auto *check = static_cast<CQCheckTreeCheck *>(item);

    check->setChecked(! check->isChecked());

    tree_->update(index);

    auto *section = check->section();

    int sectionInd = (section ? section->ind() : -1);

    auto pi = index.parent();

    while (pi.isValid()) {
      auto pi1 = tree_->model()->index(pi.row(), 1, pi.parent());

      tree_->update(pi1);

      pi = pi.parent();
    }

    if (section && section->section())
      Q_EMIT itemClicked(CQCheckTreeIndex(section->section()->ind(), sectionInd, check->ind()));
    else
      Q_EMIT itemClicked(CQCheckTreeIndex(sectionInd, check->ind()));
  }
}

void
CQCheckTree::
emitChecked(CQCheckTreeSection *section, int itemNum, bool checked)
{
  int sectionInd = (section ? section->ind() : -1);

  if (section && section->section())
    Q_EMIT itemChecked(CQCheckTreeIndex(section->section()->ind(), sectionInd, itemNum), checked);
  else
    Q_EMIT itemChecked(CQCheckTreeIndex(sectionInd, itemNum), checked);
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

    //int checkSize = tree_->style()->pixelMetric(QStyle::PM_IndicatorHeight);
    int checkSize = CHECK_SIZE;

    int dy = (option.rect.height() - checkSize)/2;

    int x = option.rect.left() + 2;
    int y = option.rect.top () + dy;

    QRect rect(x, y, checkSize, checkSize);

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
  if (index.column() == 1) {
    //int checkSize = tree_->style()->pixelMetric(QStyle::PM_IndicatorHeight);
    int checkSize = CHECK_SIZE;

    return QSize(checkSize, checkSize);
  }
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
  uint numSectionsChecked = 0;

  for (uint i = 0; i < sections_.size(); ++i) {
    auto state = sections_[i]->checkState();

    if (state == Qt::PartiallyChecked)
      return state;

    if (state == Qt::Checked)
      ++numSectionsChecked;
  }

  uint numChecked = 0;

  for (uint i = 0; i < checks_.size(); ++i)
    if (checks_[i]->isChecked())
      ++numChecked;

  if      (numSectionsChecked == 0 && numChecked == 0)
    return Qt::Unchecked;
  else if (numSectionsChecked == sections_.size() && numChecked == checks_.size())
    return Qt::Checked;
  else
    return Qt::PartiallyChecked;
}

void
CQCheckTreeSection::
setChecked(bool checked)
{
  for (uint i = 0; i < sections_.size(); ++i)
    sections_[i]->setChecked(checked);

  for (uint i = 0; i < checks_.size(); ++i)
    checks_[i]->setChecked(checked);

  emitDataChanged();
}

int
CQCheckTreeSection::
addSection(const QString &section)
{
  auto *sectionItem = new CQCheckTreeSection(tree_, section);

  addChild(sectionItem);

  sections_.push_back(sectionItem);

  int n = int(sections_.size() - 1);

  sectionItem->setSection(this);

  sectionItem->setInd(n);

  return n;
}

CQCheckTreeIndex
CQCheckTreeSection::
addCheck(int sectionInd, const QString &text)
{
  assert(sectionInd >= 0 && sectionInd < int(sections_.size()));

  //---

  // add section item
  auto *sectionItem = sections_[size_t(sectionInd)];

  auto *checkItem = new CQCheckTreeCheck(tree_, sectionItem, text);

  int itemInd = sectionItem->addCheck(checkItem);

  checkItem->setInd(itemInd);

  return CQCheckTreeIndex(ind(), sectionInd, itemInd); // sub section
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

bool
CQCheckTreeSection::
isItemChecked(int sectionInd, int checkInd) const
{
  assert(sectionInd >= 0 && checkInd < int(sections_.size()));

  auto *subSection = sections_[size_t(sectionInd)];

  return subSection->isItemChecked(checkInd);
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

void
CQCheckTreeSection::
setItemChecked(int sectionInd, int checkInd, bool checked)
{
  assert(sectionInd >= 0 && checkInd < int(sections_.size()));

  auto *subSection = sections_[size_t(sectionInd)];

  subSection->setItemChecked(checkInd, checked);
}

QString
CQCheckTreeSection::
getItemText(int sectionInd, int checkInd) const
{
  assert(sectionInd >= 0 && checkInd < int(sections_.size()));

  auto *subSection = sections_[size_t(sectionInd)];

  return subSection->getItemText(checkInd);
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

bool
CQCheckTreeSection::
hasSection(int sectionInd) const
{
  return (sectionInd >= 0 && sectionInd < int(sections_.size()));
}

QString
CQCheckTreeSection::
getSectionText(int sectionInd) const
{
  if (sectionInd < 0 || sectionInd >= int(sections_.size()))
    return "";

  auto *sectionItem = sections_[size_t(sectionInd)];

  return sectionItem->text();
}

void
CQCheckTreeSection::
updateInds(const QModelIndex &parent) const
{
  for (uint i = 0; i < sections_.size(); ++i) {
    auto pos = indexOfChild(sections_[i]);

    auto ind = parent.model()->index(pos, 1, parent);

    tree_->tree()->update(ind);

    sections_[i]->updateInds(ind);
  }

  for (uint i = 0; i < checks_.size(); ++i) {
    auto pos = indexOfChild(checks_[i]);

    auto ind = parent.model()->index(pos, 1, parent);

    tree_->tree()->update(ind);
  }
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
