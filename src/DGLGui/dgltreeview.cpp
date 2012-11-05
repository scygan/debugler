#include "dgltreeview.h"


DGLTreeItem::DGLTreeItem(DGLTreeItem *parent) {
    m_ParentItem = parent;
    //m_ItemData = data;
}

DGLTreeItem::~DGLTreeItem() {
    qDeleteAll(m_ChildItems);
}

void DGLTreeItem::appendChild(DGLTreeItem *item) {
    m_ChildItems.append(item);
}

DGLTreeItem *DGLTreeItem::child(int row) {
    return m_ChildItems.value(row);
}

int DGLTreeItem::childCount() const {
    return m_ChildItems.count();
}

int DGLTreeItem::columnCount() const {
    return m_ItemData.count();
}

QVariant DGLTreeItem::data(int column) const {
    return m_ItemData.value(column);
}

DGLTreeItem *DGLTreeItem::parent()
{
    return m_ParentItem;
}

int DGLTreeItem::row() const {
    if (m_ParentItem)
        return m_ParentItem->m_ChildItems.indexOf(const_cast<DGLTreeItem*>(this));
    return 0;
}


DGLTreeModel::DGLTreeModel(QObject *parent):QAbstractItemModel(parent) {
    QList<QVariant> rootData;
    m_RootItem = new DGLAppTreeItem(tr("unknown.exe"));
    m_RootItem->appendChild(new DGLCtxTreeItem(m_RootItem));
    m_RootItem->appendChild(new DGLCtxTreeItem(m_RootItem));
}

DGLTreeModel::~DGLTreeModel() {
    delete m_RootItem;
}

int DGLTreeModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid())
        return static_cast<DGLTreeItem*>(parent.internalPointer())->columnCount();
    else
        return m_RootItem->columnCount();
}

QVariant DGLTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    DGLTreeItem *item = static_cast<DGLTreeItem*>(index.internalPointer());

    return item->data(index.column());
}

Qt::ItemFlags DGLTreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant DGLTreeModel::headerData(int section, Qt::Orientation orientation,
    int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_RootItem->data(section);

    return QVariant();
}

QModelIndex DGLTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    DGLTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = m_RootItem;
    else
        parentItem = static_cast<DGLTreeItem*>(parent.internalPointer());

    DGLTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex DGLTreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return QModelIndex();

    DGLTreeItem *childItem = static_cast<DGLTreeItem*>(index.internalPointer());
    DGLTreeItem *parentItem = childItem->parent();

    if (parentItem == m_RootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int DGLTreeModel::rowCount(const QModelIndex &parent) const {
    DGLTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = m_RootItem;
    else
        parentItem = static_cast<DGLTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

DGLTreeView::DGLTreeView(QWidget* parrent, DglController* controller):QDockWidget(tr("State Tree"), parrent), m_TreeModel(this) {
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    disable();
    
    m_TreeView.setModel(&m_TreeModel);

    setWidget(&m_TreeView);
    //inbound
    assert(connect(controller, SIGNAL(connected()), this, SLOT(enable())));
    assert(connect(controller, SIGNAL(disconnected()), this, SLOT(disable())));
    assert(connect(controller, SIGNAL(breaked(CalledEntryPoint, uint)), this, SLOT(breaked(CalledEntryPoint, uint))));

    //outbound
    
}


DGLTreeView::~DGLTreeView() {}

void DGLTreeView::enable() {
    m_Enabled = true;
}

void DGLTreeView::disable() {
    m_Enabled = false;
}


void DGLTreeView::breaked(CalledEntryPoint entryp, uint) {

}
