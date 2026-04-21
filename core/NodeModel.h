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
    void addNode(Node* node) ;
    void updateNode(int row);
    // 设置节点列表
    void setNodes(const QList<Node*> nodeList);
    Q_INVOKABLE Node* getNode(int index) const;

public:
    QList<Node*> m_nodes;
};


#endif //IVPN_NODEMODEL_H