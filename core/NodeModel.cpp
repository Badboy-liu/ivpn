//
// Created by loukas on 2026/4/3.
//

#include "NodeModel.h"
#include "NodeModel.h"



int NodeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_nodes.size();
}

QVariant NodeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_nodes.size())
        return {};

    Node* node = m_nodes[index.row()];

    switch (role) {
        case NameRole: return node->name;
        case ServerRole: return node->server;
        case PortRole: return node->port;
        case LatencyRole: return node->latency;
        case UUIDRole: return node->uuid;
        case PasswordRole: return node->password;
        default: return {};
    }
}

QHash<int, QByteArray> NodeModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[ServerRole] = "server";
    roles[PortRole] = "port";
    roles[LatencyRole] = "latency";
    roles[UUIDRole] = "uuid";
    roles[PasswordRole] = "password";
    roles[TypeRole] = "type";

    return roles;
}

void NodeModel::setNodes(const QList<Node*> nodeList)
{
    beginResetModel();
    m_nodes = nodeList;
    endResetModel();
}
void NodeModel::updateNode(int row)
{
    QModelIndex idx = index(row);
    emit dataChanged(idx, idx);
}
void NodeModel::addNode(Node* node)
{
    beginInsertRows(QModelIndex(), m_nodes.size(), m_nodes.size());
    m_nodes.append(node);
    endInsertRows();
}
Node* NodeModel::getNode(int index) const
{
    if (index < 0 || index >= m_nodes.size()) return nullptr;
    return m_nodes[index];
}