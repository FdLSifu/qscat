#ifndef CORRLISTMODEL_H
#define CORRLISTMODEL_H


#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QListView>
#include "cpa.h"

class CorrListModel : public QAbstractListModel
{
    Q_OBJECT
    CPA * cpa;
    int idx;
private:
    QSortFilterProxyModel * proxyModel;
public:
    CorrListModel(QObject *parent, CPA *cpa, QListView *lv, int idx);

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

#endif // CORRLISTMODEL_H
