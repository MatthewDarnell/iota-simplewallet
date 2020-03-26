#include "generateaddressesdialog.h"
#include "forms/ui_generateaddressesdialog.h"

#include <walletmodel.h>

GenerateAddressesDialog::GenerateAddressesDialog(WalletModel *walletModel, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GenerateAddressesDialog),
    m_walletModel(walletModel)
{
    ui->setupUi(this);
    Q_ASSERT(m_walletModel);

    connect(ui->generateBtn, &QPushButton::clicked, this, [this] {
        m_walletModel->wallet().generateAddresses(ui->spinBox->value());
    });
}

GenerateAddressesDialog::~GenerateAddressesDialog()
{
    delete ui;
}
