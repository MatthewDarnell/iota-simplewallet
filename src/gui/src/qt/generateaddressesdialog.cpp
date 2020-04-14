#include "generateaddressesdialog.h"
#include "forms/ui_generateaddressesdialog.h"

#include <walletmodel.h>

GenerateAddressesDialog::GenerateAddressesDialog(uint32_t numberOfAddressesGenerated, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GenerateAddressesDialog)
{
    ui->setupUi(this);

    ui->numberOfAddresses->setText(QString("Number of addresses: %1").arg(numberOfAddressesGenerated));

    connect(ui->generateBtn, &QPushButton::clicked, this, [this] {
        generateRequested(ui->spinBox->value());
        close();
    });
}

GenerateAddressesDialog::~GenerateAddressesDialog()
{
    delete ui;
}
