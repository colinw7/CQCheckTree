#include <CQCheckTree.h>
#include <QVBoxLayout>
#include <QItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <cassert>

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
clear()
{
  tree_->clear();

  sections_.clear();
}

int
CQCheckTree::
addSection(const QString &section)
{
  auto *sectionItem = new CQCheckTreeSection(this, section);

  tree_->addTopLevelItem(sectionItem);

  sections_.push_back(sectionItem);

  return int(sections_.size() - 1);
}

CQCheckTreeIndex
CQCheckTree::
addCheck(int sectionInd, const QString &text)
{
  // add toplevel item
  if (sectionInd < 0 || sectionInd >= int(sections_.size())) {
    auto *checkItem = new CQCheckTreeCheck(nullptr, text);

    tree_->addTopLevelItem(checkItem);

    int n = tree_->topLevelItemCount();

    return CQCheckTreeIndex(-1, n - 1);
  }

  //---

  // add section item
  auto *sectionItem = sections_[size_t(sectionInd)];

  auto *checkItem = new CQCheckTreeCheck(sectionItem, text);

  int itemInd = sectionItem->addCheck(checkItem);

  return CQCheckTreeIndex(sectionInd, itemInd);
}

bool
CQCheckTree::
isItemChecked(const CQCheckTreeIndex &ind) const
{
  int sectionInd = ind.first;
  int checkInd   = ind.second;

  if (sectionInd < 0 || sectionInd >= int(sections_.size())) {
    auto *item = dynamic_cast<CQCheckTreeCheck *>(tree_->topLevelItem(checkInd));

    return item->isChecked();
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
    auto *item = dynamic_cast<CQCheckTreeCheck *>(tree_->topLevelItem(checkInd));

    item->setChecked(checked);

    return;
  };

  auto *sectionItem = sections_[size_t(sectionInd)];

  sectionItem->setItemChecked(checkInd, checked);
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
    auto *item = dynamic_cast<CQCheckTreeCheck *>(tree_->topLevelItem(checkInd));

    return item->text();
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

    emit sectionClicked(index.row());
  }
  else if (item->type() == CQCheckTreeCheck::ITEM_ID) {
    auto *check = static_cast<CQCheckTreeCheck *>(item);

    check->setChecked(! check->isChecked());

    tree_->update(index);

    auto pi = index.parent();

    auto pi1 = tree_->model()->index(pi.row(), 1);

    tree_->update(pi1);

    emit itemClicked(CQCheckTreeIndex(pi.row(), index.row()));
  }
}

void
CQCheckTree::
emitChecked(CQCheckTreeSection *section, int itemNum, bool checked)
{
  int sectionInd = this->sectionInd(section);

  if (sectionInd >= 0)
    emit itemChecked(CQCheckTreeIndex(sectionInd, itemNum), checked);
}

int
CQCheckTree::
sectionInd(CQCheckTreeSection *section) const
{
  for (uint i = 0; i < sections_.size(); ++i)
    if (sections_[i] == section)
      return int(i);

  return -1;
}

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

void
CQCheckTreeCheck::
setChecked(bool checked)
{
  if (checked_ == checked)
    return;

  checked_ = checked;

  if (section_)
    section_->emitChecked(this, checked_);

  emitDataChanged();
}
