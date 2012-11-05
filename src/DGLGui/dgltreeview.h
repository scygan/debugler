#ifndef DGLTREEVIEW_H
#define DGLTREEVIEW_H

#include <QDockWidget>
#include <QListWidget>

#include "DGLCommon//gl-types.h"

#include "dglcontroller.h"

#include <QTreeView>


class DGLTreeItem {
public:
    DGLTreeItem(DGLTreeItem *parent = 0);
    virtual ~DGLTreeItem();

    void appendChild(DGLTreeItem *child);

    DGLTreeItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    DGLTreeItem *parent();

protected:
    void setData(QList<QVariant> data) {
        m_ItemData = data;
    }
    void setData(QVariant data) {
        setData(QList<QVariant>() << data);
    }

private:
    QList<DGLTreeItem*> m_ChildItems;
    QList<QVariant> m_ItemData;
    DGLTreeItem *m_ParentItem;
};


class DGLCtxTreeItem:public DGLTreeItem {
public:
    DGLCtxTreeItem(DGLTreeItem *parent = 0):DGLTreeItem(parent) {}
};

class DGLAppTreeItem:public DGLTreeItem {
public:
    DGLAppTreeItem(QString appName, DGLTreeItem *parent = 0):DGLTreeItem(parent) {
        setData(appName);
    }
};


class DGLTreeModel : public QAbstractItemModel {
    Q_OBJECT
public:
    DGLTreeModel(QObject *parent = 0);
    ~DGLTreeModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
        const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    DGLTreeItem *m_RootItem;
};


class DGLTreeView : public QDockWidget {
    Q_OBJECT

public:
    DGLTreeView(QWidget* parrent, DglController* controller);
    ~DGLTreeView();


public slots:
    void enable();
    void disable();
    void breaked(CalledEntryPoint, uint);


private: 
    QTreeView m_TreeView;
    DGLTreeModel m_TreeModel;
    bool m_Enabled;   
};

#endif // DGLTREEVIEW_H