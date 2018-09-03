#include <QHeaderView>
#include "corrlistmodel.h"
#include <cmath>

CorrListModel::CorrListModel(QObject *parent, CPA *cpa, QListView *lv, int idx)
{
    Q_UNUSED(parent);
    this->cpa = cpa;
    this->proxyModel = new QSortFilterProxyModel;
    this->proxyModel->setSourceModel(this);
    this->idx = idx;

    lv->setModel(proxyModel);

    lv->setWordWrap(true);
    lv->setTextElideMode(Qt::ElideMiddle);
}


int CorrListModel::rowCount(const QModelIndex & /*parent*/) const
{
   return 256;
}

QVariant CorrListModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        int rowidx = index.row();

        float max = std::numeric_limits<float>::min();
        float corr = 0;
        int k = 0;
        for (int i = 0; i < cpa->samples_number; i ++)
        {
            if ( max < std::abs(cpa->correlation[idx][rowidx][i]) )
            {
                max = std::abs(cpa->correlation[idx][rowidx][i]);
                k = rowidx;
                corr = cpa->correlation[idx][rowidx][i];
            }
        }
       //return QString("0x")+ QString::number( k, 16 )+"\n"+QString::number(max);
        if (corr >= 0)
            return QString::number(max) + "\n(0x" + QString::number( k, 16 )+") +";
        else
            return QString::number(max) + "\n(0x" + QString::number( k, 16 )+") -";
    }
    else if (role == Qt::BackgroundRole)
    {
        QString fulldata = index.data().toString();
        int i = fulldata.indexOf("\n");
        QString sdata = fulldata.left(i);
        float data = sdata.toFloat();
        QColor g(Qt::green);
        g.setAlpha((int)(data*256));
        return g;
    }
    return QVariant();
}
