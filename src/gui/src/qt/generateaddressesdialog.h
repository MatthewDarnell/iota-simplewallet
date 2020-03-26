#ifndef GENERATEADDRESSESDIALOG_H
#define GENERATEADDRESSESDIALOG_H

#include <QDialog>

class WalletModel;

namespace Ui {
class GenerateAddressesDialog;
}

class GenerateAddressesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GenerateAddressesDialog(WalletModel *walletModel, QWidget *parent);
    ~GenerateAddressesDialog();

private:
    Ui::GenerateAddressesDialog *ui;
    WalletModel *m_walletModel;
};

#endif // GENERATEADDRESSESDIALOG_H
