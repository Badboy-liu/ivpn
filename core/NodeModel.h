//
// Created by loukas on 2026/4/3.
//

#ifndef IVPN_NODEMODEL_H
#define IVPN_NODEMODEL_H




#include <QAbstractListModel>
#include <memory>
#include "Node.h"

class NodeModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ServerRole,
        PortRole,
        LatencyRole,
        UUIDRole,
        PasswordRole,
        TypeRole
    };

    explicit NodeModel(QObject* parent = nullptr) : QAbstractListModel(parent) {}
    // QAbstractListModel 接口
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void addNode(const std::shared_ptr<Node>& node) ;

    // 设置节点列表
    void setNodes(const QList<std::shared_ptr<Node>>& nodeList);
    std::shared_ptr<Node> getNode(int index) const;

private:
    QList<std::shared_ptr<Node>> m_nodes;
};


#endif //IVPN_NODEMODEL_H