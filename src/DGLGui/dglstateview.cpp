/* Copyright (C) 2013 Slawomir Cygan <slawomir.cygan@gmail.com>
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "dglstateview.h"

#include <DGLCommon/gl-statequery-db.h>
#include <DGLCommon/def.h>
#include <DGLNet/protocol/resource.h>

#include <set>
#include <climits>
#include <iomanip>
#include <sstream>

class DGLItem {
   public:
    enum class Type {
        TABLE_ITEM,
        STATE_VALUE_ITEM,
    };
    DGLItem(Type type) : m_Type(type) {}
    Type getType() const { return m_Type; }

   private:
    Type m_Type;
};

class DGLStateValueItem;

class DGLStateTableItem : public DGLItem {
   public:
    DGLStateTableItem(stateQueryDB::TableID tableId)
            : DGLItem(DGLItem::Type::TABLE_ITEM), m_TableID(tableId) {
        const char* name;
        stateQueryDB::DataBase::DescribeTable(tableId, name);
        m_Name = name;
    }

    ~DGLStateTableItem() { qDeleteAll(m_Children); }

    void appendChild(DGLStateValueItem* item) { m_Children.append(item); }

    inline const char* getName() const { return m_Name; }

    QList<DGLStateValueItem*>& children() { return m_Children; }

    inline stateQueryDB::TableID getTableID() const { return m_TableID; }

    const char* m_Name;

   private:
    QList<DGLStateValueItem*> m_Children;
    stateQueryDB::TableID m_TableID;
};

class DGLStateValueItem : public DGLItem {
   public:
    DGLStateValueItem(DGLStateTableItem* parrent, const char* stateName,
                      const dglnet::resource::utils::StateItem& stateValue)
            : DGLItem(DGLItem::Type::STATE_VALUE_ITEM),
              m_Parrent(parrent),
              m_StateID((stateQueryDB::StateID)stateValue.m_StateId),
              m_Name(stateName) {
        update(stateValue);
    }

    void update(const dglnet::resource::utils::StateItem& stateValue) {
        std::ostringstream valueStr;
        size_t count = stateValue.m_Values.size();
        if (count > 1) {
            valueStr << "( ";
        }
        for (size_t i = 0; i < count - 1; ++i) {
            stateValue.m_Values[i].writeToSS(valueStr, GLParamTypeMetadata());
            valueStr << ", ";
        }
        stateValue.m_Values[count - 1].writeToSS(valueStr,
                                                 GLParamTypeMetadata());
        if (count > 1) {
            valueStr << " )";
        }
        m_Value = valueStr.str().c_str();
    }

    DGLStateTableItem* parent() { return m_Parrent; }

    int row() const {
        if (m_Parrent)
            return m_Parrent->children().indexOf(
                    const_cast<DGLStateValueItem*>(this));
        return 0;
    }

    QVariant data(int column) const {
        if (column == 0) {
            return m_Name;
        } else if (column == 1) {
            return m_Value;
        }
        return QVariant();
    }

    inline stateQueryDB::StateID getStateID() const { return m_StateID; }

    QString m_Name;
    QVariant m_Value;

   private:
    DGLStateTableItem* m_Parrent;
    stateQueryDB::StateID m_StateID;
};

class DGLStateViewItemModel : public QAbstractItemModel {
   public:
    DGLStateViewItemModel(QObject* parrent) : QAbstractItemModel(parrent) {}

    struct State {
        stateQueryDB::StateID m_Id;
        const char* m_Name;
    };

    void updateStateTable(QModelIndex tableModelIndex, size_t count,
                          const char** pNames,
                          const dglnet::resource::utils::StateItem* pStates) {

        DGLStateTableItem* table = m_TableItems[tableModelIndex.row()];

        size_t updateStateIndex = 0;
        QList<DGLStateValueItem*>::iterator stateItemIter =
                table->children().begin();

        // Update each model table:
        while (stateItemIter != table->children().end() &&
               updateStateIndex < count) {

            const dglnet::resource::utils::StateItem& updateStateItem =
                    pStates[updateStateIndex];
            const char* updateStateName = pNames[updateStateIndex];
            DGLStateValueItem* stateItem = *stateItemIter;

            if ((int)stateItem->getStateID() < (int)updateStateItem.m_StateId) {
                // The current table has lower ID than table to be updated,
                // remove it
                beginRemoveRows(tableModelIndex, stateItem->row(),
                                stateItem->row());
                delete stateItem;
                stateItemIter = table->children().erase(stateItemIter);
                endRemoveRows();
                continue;
            } else if ((int)stateItem->getStateID() >
                       (int)updateStateItem.m_StateId) {
                // The current table has higher ID than table to be updated
                beginInsertRows(tableModelIndex, stateItem->row(),
                                stateItem->row());
                stateItemIter = table->children().insert(
                        stateItemIter,
                        new DGLStateValueItem(table, updateStateName,
                                              updateStateItem));
                endInsertRows();
            } else {
                // State ID matched, update it
                QModelIndex changeIndex =
                        index(stateItem->row(), 1, tableModelIndex);
                stateItem->update(pStates[updateStateIndex]);
                dataChanged(changeIndex, changeIndex);
                // move to next state
                updateStateIndex++;
                stateItemIter++;
            }
        }
        // Remove no longer existing tables
        while (stateItemIter != table->children().end()) {
            DGLStateValueItem* stateItem = *stateItemIter;

            beginRemoveRows(QModelIndex(), stateItem->row(), stateItem->row());
            delete stateItem;
            stateItemIter = table->children().erase(stateItemIter);
            endRemoveRows();
        }
        // Add more states if needed
        while (updateStateIndex < count) {
            const dglnet::resource::utils::StateItem& updateStateItem =
                    pStates[updateStateIndex];
            const char* updateStateName = pNames[updateStateIndex];
            beginInsertRows(tableModelIndex, rowCount(tableModelIndex),
                            rowCount(tableModelIndex));
            table->children().append(new DGLStateValueItem(
                    table, updateStateName, updateStateItem));
            endInsertRows();
            updateStateIndex++;
        }
    }

    void update(const std::vector<dglnet::resource::utils::StateItem>& items) {
        struct Table {
            stateQueryDB::TableID m_TableID;
            std::vector<const char*> m_Names;
            const dglnet::resource::utils::StateItem* m_pStates;
        };
        std::vector<Table> updateItems;

        // Describe and group items
        {
            for (size_t i = 0; i < items.size(); i++) {
                State state;
                stateQueryDB::TableID tableID;
                stateQueryDB::DataBase::DescribeState(
                        (stateQueryDB::StateID)items[i].m_StateId, state.m_Name,
                        tableID);

                // The states are sorted through tables
                if (!updateItems.size() ||
                    updateItems.back().m_TableID < tableID) {
                    Table table = {
                            tableID,    // the table ID to update
                            std::vector<const char*>(),    // the vector of
                                                           // state descriptive
                                                           // names
                            &items[i]    // pointer to first state:value
                    };
                    updateItems.push_back(table);
                }
                updateItems.back().m_Names.push_back(state.m_Name);
            }
        }

        size_t updateTableIndex = 0;
        QList<DGLStateTableItem*>::iterator tableItemIter =
                m_TableItems.begin();

        // Update each model table:
        while (tableItemIter != m_TableItems.end() &&
               updateTableIndex < updateItems.size()) {

            Table& updateTable = updateItems[updateTableIndex];
            DGLStateTableItem* tableItem = *tableItemIter;
            int row = tableItemIter - m_TableItems.begin();

            if ((int)tableItem->getTableID() < (int)updateTable.m_TableID) {
                // The current table has lower ID than table to be updated,
                // remove it
                beginRemoveRows(QModelIndex(), row, row);
                delete tableItem;
                tableItemIter = m_TableItems.erase(tableItemIter);
                endRemoveRows();
            } else if ((int)tableItem->getTableID() >
                       (int)updateTable.m_TableID) {
                // The current table has higher ID than table to be updated
                beginInsertRows(QModelIndex(), row, row);
                tableItemIter = m_TableItems.insert(
                        tableItemIter,
                        new DGLStateTableItem(updateTable.m_TableID));
                endInsertRows();
            } else {
                // Table ID matched, update the table
                updateStateTable(index(row, 0, QModelIndex()),
                                 updateTable.m_Names.size(),
                                 &updateTable.m_Names[0],
                                 updateTable.m_pStates);
                // move to next table
                updateTableIndex++;
                tableItemIter++;
            }
        }
        // Remove no longer existing tables
        while (tableItemIter != m_TableItems.end()) {
            DGLStateTableItem* tableItem = *tableItemIter;
            int row = tableItemIter - m_TableItems.begin();

            beginRemoveRows(QModelIndex(), row, row);
            delete tableItem;
            tableItemIter = m_TableItems.erase(tableItemIter);
            endRemoveRows();
        }
        // Add more tables if needed
        while (updateTableIndex < updateItems.size()) {
            Table& updateTable = updateItems[updateTableIndex];

            beginInsertRows(QModelIndex(), rowCount(QModelIndex()),
                            rowCount(QModelIndex()));
            tableItemIter = m_TableItems.insert(
                    m_TableItems.end(),
                    new DGLStateTableItem(updateTable.m_TableID));
            endInsertRows();
            int row = tableItemIter - m_TableItems.begin();
            updateStateTable(index(row, 0, QModelIndex()),
                             updateTable.m_Names.size(),
                             &updateTable.m_Names[0], updateTable.m_pStates);
            updateTableIndex++;
        }
    }
    void clean() {
        beginResetModel();
        qDeleteAll(m_TableItems);
        m_TableItems.clear();
        endResetModel();
    }

   private:
    QModelIndex index(int row, int column, const QModelIndex& parent) const {
        if (!hasIndex(row, column, parent)) return QModelIndex();

        if (!parent.isValid()) {
            return createIndex(row, column, m_TableItems.value(row));
        } else {
            DGLStateTableItem* tableItem =
                    static_cast<DGLStateTableItem*>(parent.internalPointer());
            return createIndex(row, column, tableItem->children().value(row));
        }
    }

    QModelIndex parent(const QModelIndex& index) const {
        if (!index.isValid()) return QModelIndex();

        DGLItem* childItem = static_cast<DGLItem*>(index.internalPointer());

        if (childItem->getType() == DGLItem::Type::TABLE_ITEM) {
            return QModelIndex();
        } else if (childItem->getType() == DGLItem::Type::STATE_VALUE_ITEM) {
            DGLStateTableItem* parentItem =
                    static_cast<DGLStateValueItem*>(childItem)->parent();
            return createIndex(m_TableItems.indexOf(parentItem), 0, parentItem);
        } else {
            DGL_ASSERT(!"Unknown item type");
            return QModelIndex();
        }
    }

    int rowCount(const QModelIndex& parent) const {
        if (parent.column() > 0) return 0;

        if (!parent.isValid()) {
            return m_TableItems.count();
        } else {
            DGLItem* item = static_cast<DGLItem*>(parent.internalPointer());
            if (item->getType() == DGLItem::Type::TABLE_ITEM) {
                DGLStateTableItem* tableItem =
                        static_cast<DGLStateTableItem*>(item);
                return tableItem->children().count();
            } else if (item->getType() == DGLItem::Type::STATE_VALUE_ITEM) {
                return 0;
            }
            DGL_ASSERT(!"Mismatching item type");
            return 0;
        }
    }

    int columnCount(const QModelIndex&) const {
        if (!m_TableItems.size()) {
            return 0;
        }
        return 2;
    }

    QVariant data(const QModelIndex& index, int role) const {
        if (!index.isValid()) return QVariant();

        if (role != Qt::DisplayRole) return QVariant();

        DGLItem* item = static_cast<DGLItem*>(index.internalPointer());

        if (item->getType() == DGLItem::Type::TABLE_ITEM) {
            if (index.column() == 0) {
                return static_cast<DGLStateTableItem*>(item)->getName();
            } else {
                return QVariant();
            }
        } else if (item->getType() == DGLItem::Type::STATE_VALUE_ITEM) {
            return static_cast<DGLStateValueItem*>(item)->data(index.column());
        } else {
            DGL_ASSERT("Unknown item type");
            return QVariant();
        }
    }

    Qt::ItemFlags flags(const QModelIndex& index) const {
        if (!!m_TableItems.size() || !index.isValid()) return 0;

        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    QVariant headerData(int /*section*/, Qt::Orientation orientation,
                        int role) const {
        if (!m_TableItems.size()) {
            return QVariant();
        }
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return "Header Data";

        return QVariant();
    }

    QList<DGLStateTableItem*> m_TableItems;
};

DGLStateView::DGLStateView(QWidget* parrent, DglController* controller)
        : QDockWidget(tr("OpenGL State"), parrent),
          m_Listener(NULL),
          m_Controller(controller),
          m_Ui(NULL) {
    setObjectName("DGLStateView");

    setConnected(false);

    // inbound
    CONNASSERT(controller, SIGNAL(setConnected(bool)), this,
               SLOT(setConnected(bool)));

    m_Model = new DGLStateViewItemModel(this);
}

void DGLStateView::update(const dglnet::DGLResource& res) {
    const dglnet::resource::DGLResourceState* resource =
            dynamic_cast<const dglnet::resource::DGLResourceState*>(&res);

    m_Model->update(resource->m_Items);
}

void DGLStateView::error(const std::string& /*message*/) { m_Model->clean(); }

void DGLStateView::setConnected(bool connected) {
    if (!connected) {
        if (m_Ui) {
            delete m_Ui->frame;
            delete m_Ui;
            m_Ui = NULL;
        }
    } else {
        m_Ui = new Ui::DGLStateView();
        m_Ui->setupUi(this);
        setWidget(m_Ui->frame);

        m_Model->clean();
        m_Ui->treeView->setModel(m_Model);

        m_Listener = m_Controller->getResourceManager()->createListener(
                dglnet::ContextObjectName(),
                dglnet::message::ObjectType::State);
        m_Listener->setParent(m_Ui->frame);

        CONNASSERT(m_Listener, SIGNAL(update(const dglnet::DGLResource&)), this,
                   SLOT(update(const dglnet::DGLResource&)));
        CONNASSERT(m_Listener, SIGNAL(error(const std::string&)), this,
                   SLOT(error(const std::string&)));

        m_Listener->setEnabled(isVisible());
        CONNASSERT(this, SIGNAL(visibilityChanged(bool)), m_Listener,
                   SLOT(setEnabled(bool)));
    }
}
