#include "corrtablemodel.h"

CorrTableModel::CorrTableModel(QObject */*parent*/, CPA *cpa)
{
    this->cpa = cpa;
}


int CorrTableModel::rowCount(const QModelIndex & /*parent*/) const
{
   return 256;
}

int CorrTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return this->cpa->byteidx.length();
}

QVariant CorrTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        int rowidx = index.row();
        int colidx = cpa->byteidx[index.column()];
        float max = std::numeric_limits<float>::min();
        int k = 0;
        for (int i = 0; i < cpa->samples_number; i ++)
        {
            if ( max < std::abs(cpa->correlation[colidx][rowidx][i]) )
            {
                max = std::abs(cpa->correlation[colidx][rowidx][i]);
                k = rowidx;
            }
        }
       return QString("0x")+ QString::number( k, 16 )+"\n"+QString::number(max);
    }
    else if (role == Qt::BackgroundRole)
    {
        QString fulldata = index.data().toString();
        int i = fulldata.indexOf("\n") + 1;
        int n = fulldata.length();
        float data = fulldata.right(n-i).toFloat();
        QColor g(Qt::green);
        g.setAlpha((int)(data*256));
        return g;
    }
    return QVariant();
}
