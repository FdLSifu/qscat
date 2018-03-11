#include "curvetablemodel.h"
#include "scatool.h"

CurveTableModel::CurveTableModel(QObject *parent)
{
    tv = dynamic_cast<QTableView*>(parent);
    // Combo box for types
    tv->setItemDelegateForColumn(3,new ComboBoxTypeDelegate(tv));
}

int CurveTableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return ScaTool::curves->length();
}

int CurveTableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return CurveTableModel::NBCOLS;
}

QVariant CurveTableModel::data(const QModelIndex &index, int role) const
{
    int idx = index.row();
    Curve *curve = (*ScaTool::curves)[idx];

    if (role == Qt::DisplayRole)
    {
        switch (index.column()) {
        case 0: // Nb
            return QString::number(idx);
            break;
        case 1: // Color
            return QVariant();
            break;
        case 2: // Name
            return curve->cname;
            break;
        case 3: // Type
            return QString::number(curve->type);
            break;
        case 4: // x Offset
            return QString::number(curve->xoffset);
            break;
        case 5: // Displayed
            return QVariant();
            break;
        case 6: // Data
            return curve->textin;
            break;
        default:
            break;
        }
    }
    else if (role == Qt::ToolTipRole)
    {
        if (index.column() == 2)
            return curve->fn;

    }
    else if (role == Qt::BackgroundRole)
    {
        if (index.column() == 1)
        {
            if (curve->displayed)
                return curve->color;
            else
                return QColor(Qt::white);
        }
    }
    else if (role == Qt::CheckStateRole)
    {
        if (index.column() == 5)
        {
            if(curve->displayed)
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
    }
    return QVariant();
}

QVariant CurveTableModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
    if (role == Qt::DisplayRole)
    {
        switch (section) {
        case 0: // Nb
            return QString("Nb");
            break;
        case 1: // Color
            return QString("Color");
            break;
        case 2: // Name
            return QString("Name");
            break;
        case 3: // Type
            return QString("Type");
            break;
        case 4: // x Offset
            return QString("x Offset");
            break;
        case 5: // Displayed
            return QString("Displayed");
            break;
        case 6: // Data
            return QString("Data");
            break;
        default:
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags CurveTableModel::flags(const QModelIndex & index) const
{
    if(index.column() == 5)
    {
        return Qt::NoItemFlags | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }
    else if (index.column() == 3)
    {
        tv->openPersistentEditor(index);
        return Qt::NoItemFlags | Qt::ItemIsEnabled | Qt::ItemIsEditable;
    }
    return Qt::NoItemFlags | Qt::ItemIsSelectable |  Qt::ItemIsEnabled;
}

bool CurveTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if(index.column() == 5 && role == Qt::CheckStateRole)
    {
        ScaTool::curves->at(index.row())->chkbox_toggled(value.toBool());
        return true;
    }
    else if (index.column() == 3 && role == Qt::EditRole)
    {
        return true;
    }
    else
        return QAbstractTableModel::setData(index, value, role);
}



QString ComboBoxTypeDelegate::type_names [8];

ComboBoxTypeDelegate::ComboBoxTypeDelegate(QObject */*parent*/)
{
    type_names[0] = QString("float32");
    type_names[1] = QString("uint32");
    type_names[2] = QString("int32");
    type_names[3] = QString("uint16");
    type_names[4] = QString("int16");
    type_names[5] = QString("uint8");
    type_names[6] = QString("int8");
    type_names[7] = QString("double");
}

QWidget* ComboBoxTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/, const QModelIndex &/*index*/) const
{
    QComboBox* editor = new QComboBox(parent);
    connect(editor,SIGNAL(currentIndexChanged(int)),this,SLOT(commit(int)));
    for(unsigned int i = 0; i < 8; ++i)
      {
        editor->addItem(type_names[i]);
      }
    return editor;
}
void ComboBoxTypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = ScaTool::curves->at(index.row())->type;
    comboBox->setCurrentIndex(value);
}
void ComboBoxTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    Curve * curve = ScaTool::curves->at(index.row());
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    curve->setType(comboBox->currentIndex());
    model->setData(index, curve->type, Qt::EditRole);
}
void ComboBoxTypeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // Not the right place but works for catching external curve type change
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    int value = ScaTool::curves->at(index.row())->type;
    comboBox->setCurrentIndex(value);

    editor->setGeometry(option.rect);
}

void ComboBoxTypeDelegate::commit(int)
{
    QComboBox *comboBox = (QComboBox*)sender();
    commitData(comboBox);
}
