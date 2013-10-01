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

  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

 private:
  CQCheckTree *tree_;
};

//------

CQCheckTree::
CQCheckTree(QWidget *parent) :
 QWidget(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
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

int
CQCheckTree::
addSection(const QString &section)
{
  CQCheckTreeSection *sectionItem = new CQCheckTreeSection(this, section);

  tree_->addTopLevelItem(sectionItem);

  sections_.push_back(sectionItem);

  return sections_.size() - 1;
}

CQCheckTreeIndex
CQCheckTree::
addCheck(int sectionInd, const QString &text)
{
  if (sectionInd < 0 || sectionInd >= int(sections_.size()))
    return CQCheckTreeIndex(-1,-1);

  CQCheckTreeSection *sectionItem = sections_[sectionInd];

  CQCheckTreeCheck *checkItem = new CQCheckTreeCheck(sectionItem, text);

  int itemInd = sectionItem->addCheck(checkItem);

  return CQCheckTreeIndex(sectionInd, itemInd);
}

bool
CQCheckTree::
isItemChecked(const CQCheckTreeIndex &ind) const
{
  int sectionInd = ind.first;
  int checkInd   = ind.second;

  if (sectionInd < 0 || sectionInd >= int(sections_.size()))
    return false;

  CQCheckTreeSection *sectionItem = sections_[sectionInd];

  return sectionItem->isItemChecked(checkInd);
}

QString
CQCheckTree::
getSectionText(int sectionInd) const
{
  if (sectionInd < 0 || sectionInd >= int(sections_.size()))
    return "";

  CQCheckTreeSection *sectionItem = sections_[sectionInd];

  return sectionItem->text();
}

QString
CQCheckTree::
getItemText(const CQCheckTreeIndex &ind) const
{
  int sectionInd = ind.first;
  int checkInd   = ind.second;

  if (sectionInd < 0 || sectionInd >= int(sections_.size()))
    return "";

  CQCheckTreeSection *sectionItem = sections_[sectionInd];

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
    QTreeWidgetItem *parent = getModelItem(index.parent());
    assert(parent);

    item = parent->child(index.row());
  }

  return item;
}

void
CQCheckTree::
itemClicked(const QModelIndex &index)
{
  QTreeWidgetItem *item = getModelItem(index);
  if (! item) return;

  if (index.column() != 1)
    return;

  if      (item->type() == CQCheckTreeSection::ITEM_ID) {
    CQCheckTreeSection *section = static_cast<CQCheckTreeSection *>(item);

    Qt::CheckState checkState = section->checkState();

    section->setChecked(checkState != Qt::Checked);

    tree_->update(index);

    for (uint i = 0; i < section->numChecks(); ++i)
      tree_->update(index.child(i, 1));

    emit sectionClicked(index.row());
  }
  else if (item->type() == CQCheckTreeCheck::ITEM_ID) {
    CQCheckTreeCheck *check = static_cast<CQCheckTreeCheck *>(item);

    check->setChecked(! check->isChecked());

    tree_->update(index);

    QModelIndex pi = index.parent();

    QModelIndex pi1 = tree_->model()->index(pi.row(), 1);

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
      return i;

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
    QTreeWidgetItem *item = tree_->getModelItem(index);
    assert(item);

    Qt::CheckState checkState = Qt::Unchecked;

    if      (item->type() == CQCheckTreeSection::ITEM_ID) {
      CQCheckTreeSection *section = static_cast<CQCheckTreeSection *>(item);

      checkState = section->checkState();
    }
    else if (item->type() == CQCheckTreeCheck::ITEM_ID) {
      CQCheckTreeCheck *check = static_cast<CQCheckTreeCheck *>(item);

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

  return checks_.size() - 1;
}

bool
CQCheckTreeSection::
isItemChecked(int checkInd) const
{
  if (checkInd < 0 || checkInd >= int(checks_.size()))
    return false;

  CQCheckTreeCheck *checkItem = checks_[checkInd];

  return checkItem->isChecked();
}

QString
CQCheckTreeSection::
getItemText(int checkInd) const
{
  if (checkInd < 0 || checkInd >= int(checks_.size()))
    return "";

  CQCheckTreeCheck *checkItem = checks_[checkInd];

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
      return i;

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

  section_->emitChecked(this, checked_);

  emitDataChanged();
}
