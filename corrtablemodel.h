#ifndef CORRTABLEMODEL_H
#define CORRTABLEMODEL_H

#include <QAbstractTableModel>
#include "cpa.h"

class CorrTableModel : public QAbstractTableModel
{
    Q_OBJECT
    CPA * cpa;
public:
    CorrTableModel(QObject *parent, CPA *cpa);

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
};

#endif // CORRTABLEMODEL_H
