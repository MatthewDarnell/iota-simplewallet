#include "generateaddressesdialog.h"
#include "forms/ui_generateaddressesdialog.h"

#include <walletmodel.h>

GenerateAddressesDialog::GenerateAddressesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GenerateAddressesDialog)
{
    ui->setupUi(this);

    connect(ui->generateBtn, &QPushButton::clicked, this, [this] {
        generateRequested(ui->spinBox->value());
        close();
    });
}

GenerateAddressesDialog::~GenerateAddressesDialog()
{
    delete ui;
}
