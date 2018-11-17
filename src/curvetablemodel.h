#ifndef CURVETABLEMODEL_H
#define CURVETABLEMODEL_H
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QTableView>

class CurveTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    QTableView * tv;
    CurveTableModel(QObject *);

    int rowCount(const QModelIndex &) const;
    int columnCount(const QModelIndex &) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role);
private:
    static const int NBCOLS = 7;
};



class ComboBoxTypeDelegate : public QItemDelegate
{
    Q_OBJECT
public:

    ComboBoxTypeDelegate(QObject *);

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    static QString type_names [];
private slots:
    void commit(int);

};
#endif // CURVETABLEMODEL_H
